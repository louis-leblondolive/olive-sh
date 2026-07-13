#include "builtins.h"

#include "job_ctl.h"
#include "printer.h"

int builtin_jobs(int argc, char **argv, env_t *env){

    (void)argc; 
    (void)argv; 
    (void)env;

    job_table_t *job_tbl = get_main_job_table();

    for (size_t i = 1; i < job_tbl->capacity; i++){

        if(job_tbl->tbl[i]){
            
            job_t *job = job_tbl->tbl[i];

            printf("[%d] - ", job->job_id);
            
            switch(job->status){
                case SUSPENDED: 
                    printf(BOLD_ORANGE); printf("SUSPENDED");
                    break;

                case RUNNING: 
                    printf(BOLD_GREEN); printf("RUNNING  ");
                    break;

                default: break;
            }
            printf(RESET);

            printf("   %s\n", job->job_cmd);
        }   
    }

    return 0;
}