#include "job_ctl.h"

pid_t g_shell_pgid = -1;
job_t *g_foreground_job = NULL;


bool is_shell_foreground(void){
    return (g_shell_pgid == g_foreground_job->pg_id);
}


int set_shell_foreground(void){

    job_t *new_fg_job = job_init(g_shell_pgid, NULL);
    if(!new_fg_job) return 1;

    if(g_foreground_job) free_job(g_foreground_job);
    g_foreground_job = new_fg_job;

    tcsetpgrp(STDIN_FILENO, g_shell_pgid);

    print_debug("Shell is now foreground (shell_pg_id : %d)\n", g_shell_pgid);

    return 0;
}


int update_foreground_job(job_t *new_fg_job){
    if(!new_fg_job) return 1;

    print_debug("[insert command] is now foreground (pgid : %d)\n", new_fg_job->pg_id);

    if(g_foreground_job) free_job(g_foreground_job);     
    g_foreground_job = new_fg_job;

    return 0;
}