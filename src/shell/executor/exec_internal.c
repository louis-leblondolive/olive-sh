#include "exec_internal.h"
#include "executor.h"


exec_res_t run_builtin(int id, int argc, char **argv, env_t *env, int fd_in, int fd_out){
    
    print_debug("Running builtin command\n");
    
    // Setup I/O
    int save_in = -1;
    int save_out = -1;
    if(fd_in != STDIN_FILENO){
        save_in = dup(STDIN_FILENO);
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if(fd_out != STDOUT_FILENO){
        save_out = dup(STDOUT_FILENO);
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
    
    // Run 
    exec_res_t res = builtins[id].func(argc, argv, env);

    // Clean 
    if(save_in != -1){
        dup2(save_in, STDIN_FILENO); close(save_in);
    }
    if(save_out != -1){
        dup2(save_out, STDOUT_FILENO); close(save_out);
    } 

    return res;
}


int run_cmd_async(env_t *env, char **argv, char **envp, int fd_in, int fd_out, int fd_err){

    print_debug("Running command asynchronously\n");

    // --- SETUP SIGNAL HANDLERS --------------------------------------------
    if(reset_sa_handlers() != 0){
        perror("sigaction");
        return 1;
    }

    // --- SETUP REDIRS AND I/O ----------------------------------------------
    if(fd_in != STDIN_FILENO){
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }
    if(fd_out != STDOUT_FILENO){
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    } 
    if(fd_err != STDERR_FILENO){
        dup2(fd_err, STDERR_FILENO);
        close(fd_err);
    }

    // --- FIND COMMAND PATH --------------------------------------------------
    char *cmd_name = strdup(argv[0]);
    char *cmd_path = NULL;

    if(strchr(cmd_name, '/') != NULL){      // Run from given path 

        // Resolving path
        cmd_path = realpath(cmd_name, NULL);
        if(!cmd_path){
            dprintf(STDERR_FILENO, "olive-sh: %s: no such file or directory\n", cmd_name);
            return 127;
        }

        // Permission test 
        struct stat buf;
        if(stat(cmd_path, &buf) != 0){
            dprintf(STDERR_FILENO, "olive-sh: %s: no such file or directory\n", cmd_name);
            return 127;
        }
        if(!S_ISREG(buf.st_mode)){
            dprintf(STDERR_FILENO, "olive-sh: %s is a directory\n", cmd_name);
            return 126;
        }
        if(access(cmd_path, X_OK) != 0){
            dprintf(STDERR_FILENO, "olive-sh: %s: permission denied\n", cmd_name);
            return 126;
        }
    }
    else{    // Run a command 
        cmd_path = find_cmd_path(env, cmd_name);
        if(!cmd_path){
            dprintf(STDERR_FILENO, "olive-sh: %s: command not found\n", cmd_name);
            return 127;
        }
    }

    print_debug("Found command %s at %s\n", cmd_name, cmd_path);
            
    execve(cmd_path, argv, envp);

    dprintf(STDERR_FILENO, "olive-sh: %s: %s\n", cmd_name, strerror(errno));
    if(errno == ENOENT) return 127;
    else return 126;
}


exec_res_t run_pipe_children(env_t *env, ast_node_t *ast, 
    pid_t group_pgid, 
    int std_fd_in, int std_fd_out, int err_out_fd){

    // --- Setup pipes ---------------
    int io_pipe[2];
    int left_err_pipe[2];
    int right_err_pipe[2];

    if(open_cloexec_pipe(io_pipe) != 0) 
        return exec_res_from_builtin(1);
    if(open_cloexec_pipe(left_err_pipe) != 0) 
        { close_pipe(io_pipe); return exec_res_from_builtin(1); }
    if(open_cloexec_pipe(right_err_pipe) != 0) 
        { close_pipe(io_pipe); close_pipe(left_err_pipe); return exec_res_from_builtin(1); }

    // --- Handle children ------------

        // Init left child 
    pid_t left_chld = fork();
    if(left_chld == -1) { 
        perror("fork"); 
        close_pipe(io_pipe);
        close_pipe(left_err_pipe); close_pipe(right_err_pipe);
        return exec_res_from_builtin(1); 
    }
                        
        // Run left child 
    setpgid(left_chld, group_pgid);
    if(left_chld == 0) {    
        close(io_pipe[0]);
        reset_sa_handlers();

        exec_res_t cache_res = run_ast(env, ast->left, std_fd_in, io_pipe[1], left_err_pipe[1]);

        switch (cache_res.kind){
            case RES_EXITED: exit(cache_res.exit_code);
            case RES_STOPPED: exit(128 + cache_res.stop_sig);
            case RES_SIGNALED: exit(128 + cache_res.term_sig);
            default: exit(1);
        }
    }

        // Init right child
    pid_t right_chld = fork();
    if(right_chld == -1) { 
        perror("fork"); 
        close_pipe(io_pipe);
        close_pipe(left_err_pipe); close_pipe(right_err_pipe);
        return exec_res_from_builtin(1); 
    }

        // Run right child 
    setpgid(right_chld, group_pgid);
    if(right_chld == 0) {
        close(io_pipe[1]);
        reset_sa_handlers();

        exec_res_t cache_res = run_ast(env, ast->right, io_pipe[0], std_fd_out, right_err_pipe[1]);

        switch (cache_res.kind){
            case RES_EXITED: exit(cache_res.exit_code);
            case RES_STOPPED: exit(128 + cache_res.stop_sig);
            case RES_SIGNALED: exit(128 + cache_res.term_sig);
            default: exit(1);
        }
    }   

        // Clean pipes 
    close_pipe(io_pipe);
    close(left_err_pipe[1]);
    close(right_err_pipe[1]);

    // --- Wait for jobs ----------
    pid_t done_id;
    int res; 
    int l_status = 0; int r_status = 0; 
    bool l_done = false; bool r_done = false; 

    while(!(r_done && l_done)){
                            
        done_id = waitpid(-group_pgid, &res, WUNTRACED);
        if(done_id < 0){ if(errno == EINTR) continue; else break; }

        if(done_id == left_chld){ 
            if(WIFSTOPPED(res)) continue;
            l_status = res; 
            l_done = true; 
        }
        if(done_id == right_chld){ 
            if(WIFSTOPPED(res)) continue;
            r_status = res; 
            r_done = true;
        }
    }

    // --- Clean and exit ---------------
    int exit_status;

    if(l_status != 0 && cfg_infos.pipefail){
        relay_child_error(left_err_pipe[0], err_out_fd); 
        exit_status = l_status;
    }
    else{
        if(r_status != 0) relay_child_error(right_err_pipe[0], err_out_fd); 
        exit_status = r_status;
    }

    close(left_err_pipe[0]);
    close(right_err_pipe[0]);
                
    return exec_res_from_waitpid_status(exit_status);
}


exec_res_t run_ast_background(env_t *env, ast_node_t *ast, 
    int std_fd_in, int std_fd_out){

        
    int err_pipe[2];
    if(open_cloexec_pipe(err_pipe) != 0){
        return exec_res_from_builtin(1);
    }

    pid_t leader_pid = fork();
    if(leader_pid == -1){
        perror("fork");
        close_pipe(err_pipe);
        return exec_res_from_builtin(1);
    }

    bool is_child = (leader_pid == 0);
    
    // Init leader job 
    if(is_child) leader_pid = getpid();
    char *cmd = strdup(ast->str_cmd);

    job_t *leader_job = job_init(leader_pid, leader_pid, err_pipe[0], cmd);
    if(!leader_job){
        close_pipe(err_pipe);
        return exec_res_from_builtin(1);
    }

    setpgid(leader_pid, leader_pid);

    // Run ast background 
    if(is_child){

        if(reset_sa_handlers() != 0){
            perror("signal");
            close_pipe(err_pipe);
            exit(1);
        }

        set_foreground_job(leader_job);
        // Local modification of the main job 
        // Even though this job isn't foreground, sub-processes created when 
        //  running run_ast will believe it and inherit its pgid 
        
        exec_res_t res = run_ast(env, ast, std_fd_in, std_fd_out, err_pipe[1]);
        
        close_pipe(err_pipe); 

        switch(res.kind){
            case RES_EXITED: exit(res.exit_code);
            case RES_STOPPED: exit(res.stop_sig);
            case RES_SIGNALED: exit(res.term_sig);
            default: exit(1);
        }
    }
    
    // Parent
    close(err_pipe[1]);

    if(main_job_table_add(leader_job) <= 0){
        killpg(leader_pid, SIGKILL);
        free_job(leader_job);
        return exec_res_from_builtin(1);
    }

    printf("[%d] - %d\n", leader_job->job_id, leader_job->leader_pid);

    return exec_res_from_builtin(0);
}


int setup_redirs(env_t *env, ast_node_t *cmd_node, int *fd_in, int *fd_out){

    for(redir_t *red = cmd_node->redirs->first; red != NULL; red = red->next){

        char *red_target = expand_segment_chain(env, red->target);
        if(!red_target){
            char *unbound_info = expand_var(env, "ERRLOG");
            env_export(env, "ERRLOG", "olive-sh: failed to resolve redirection target (%s)\n", unbound_info);
            free(unbound_info);
            free(red_target);
            return -1;
        }

        if(red->type == TOKEN_REDIR_IN){
            if(*fd_in != STDIN_FILENO) close(*fd_in);
            *fd_in = open(red_target, O_RDONLY, 0644);
            if(*fd_in < 0){
                env_export(env, "ERRLOG", "olive-sh: < %s: failed to open redirection target", red_target);
                free(red_target);
                return -1;
            }
        }
        else if(red->type == TOKEN_REDIR_OUT){
            if(*fd_out != STDOUT_FILENO) close(*fd_out);
            *fd_out = open(red_target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(*fd_out < 0){
                env_export(env, "ERRLOG", "olive-sh: > %s: failed to open redirection target", red_target);
                free(red_target);
                return -1;
            }
        }
        else if(red->type == TOKEN_APPEND){
            if(*fd_out != STDOUT_FILENO) close(*fd_out);
            *fd_out = open(red_target, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if(*fd_out < 0){
                env_export(env, "ERRLOG", "olive-sh: >> %s: failed to open redirection target", red_target);
                free(red_target);
                return -1;
            }
        }

        free(red_target);
    }

    return 0;
}


void relay_child_error(int err_pipe_fd, int target_fd){

    char errlog[MAX_ERROR_LEN];
    ssize_t n = read(err_pipe_fd, errlog, sizeof(errlog) - 1);
                
    if(n < 0) { errlog[0] = '\0'; n = 1; }
    else { errlog[n] = '\0'; }

    write(target_fd, errlog, (size_t)n);
}