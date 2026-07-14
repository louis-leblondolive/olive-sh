#include "builtins.h"

#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "job_ctl.h"

exec_res_t builtin_fg(int argc, char **argv, env_t *env){

    (void)env;

    if(argc <= 1){
        env_export(env, "ERRLOG", "fg: no arguments provided. Usage: fg %%job_id");
        return exec_res_from_builtin(1);
    }

    int job_id = -1;
    if(sscanf(argv[1], "%%%d", &job_id) != 1){
        env_export(env, "ERRLOG", "fg: incorrect arguments. Usage: fg %%job_id");
        return exec_res_from_builtin(2);
    }

    job_table_t *job_tbl = get_main_job_table();
    if(!job_tbl || !job_tbl->tbl || job_id >= (int)job_tbl->capacity || job_id <= 0
        || !job_tbl->tbl[job_id]){

        env_export(env, "ERRLOG", "fg: invalid job id [%d]", job_id);
        return exec_res_from_builtin(1);
    }

    job_t *job = job_tbl->tbl[job_id];
    
    if(job->status == DONE) return exec_res_from_builtin(0);

    // Extract job from job table
    char *cmd = strdup(job->job_cmd);
    int leader_std_err_fd = dup(job->leader_std_err_fd);
    job_t *leader_job = job_init(job->pgid, job->leader_pid, leader_std_err_fd, cmd);
    leader_job->status = job->status;

    main_job_table_rm(job->job_id);

    if(!leader_job) return exec_res_from_builtin(2);

    // Run and send new leader job forward

    if(leader_job->status == SUSPENDED){
        if(killpg(leader_job->pgid, SIGCONT) != 0) return exec_res_from_builtin(2);
        leader_job->status = RUNNING;
    }
    

    set_foreground_job(leader_job);
    tcsetpgrp(STDIN_FILENO, leader_job->leader_pid);

    int exit_status;
    waitpid(leader_job->leader_pid, &exit_status, WUNTRACED);

        // Clean and exit 
    if(WIFSTOPPED(exit_status)){
        if(suspend_job(leader_job) != 0){
            dprintf(STDERR_FILENO, "Error during job suspension\n");
            close(leader_job->leader_std_err_fd);
            return exec_res_from_builtin(1);
        }

        return exec_res_from_waitpid_status(exit_status);
    }

    if(exit_status != 0){
        // Relay child error
        char errlog[MAX_ERROR_LEN];
        ssize_t n = read(leader_job->leader_std_err_fd, errlog, sizeof(errlog) - 1);
                    
        if(n < 0) { errlog[0] = '\0'; n = 1; }
        else { errlog[n] = '\0'; }

        write(STDERR_FILENO, errlog, (size_t)n);
    }
    close(leader_job->leader_std_err_fd);

    set_shell_foreground();

    return exec_res_from_waitpid_status(exit_status);
}