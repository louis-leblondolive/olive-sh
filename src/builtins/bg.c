#include "builtins.h"

#include <signal.h>
#include "job_ctl.h"


int builtin_bg(int argc, char **argv, env_t *env){
    (void)env;

    if(argc <= 1){
        env_export(env, "ERRLOG", "bg: no arguments provided. Usage: bg %%job_id");
        return 1;
    }

    int job_id = -1;
    if(sscanf(argv[1], "%%%d", &job_id) != 1){
        env_export(env, "ERRLOG", "bg: incorrect arguments. Usage: bg %%job_id");
        return 2;
    }

    job_table_t *job_tbl = get_main_job_table();
    if(!job_tbl || !job_tbl->tbl || job_id >= (int)job_tbl->capacity || job_id <= 0
        || !job_tbl->tbl[job_id]){

        env_export(env, "ERRLOG", "bg: invalid job id [%d]", job_id);
        return 1;
    }

    job_t *job = job_tbl->tbl[job_id];

    if(job->status != SUSPENDED){
        env_export(env, "ERRLOG", "bg: job [%d] already in background", job_id);
        return 1;
    }

    if(killpg(job->pgid, SIGCONT) != 0) return 2;

    job->status = RUNNING;
    
    return 0;
}