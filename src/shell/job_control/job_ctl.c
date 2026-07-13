#include "job_ctl.h"

static pid_t g_shell_pgid = -1;
static job_t *g_foreground_job = NULL;
static job_table_t *g_job_table = NULL;


//  ----- SHELL TOOLS -----------------------------------------------
int save_shell_pid(void){

    pid_t shell_pid = getpid();
    if(shell_pid < 0) return 1;

    g_shell_pgid = shell_pid;
    return 0;
}


bool is_shell_foreground(void){
    return (g_shell_pgid == g_foreground_job->pgid);
}


//  ----- FOREGROUND JOB MANAGEMENT -----------------------------------------------
int init_foreground_job(void){

    job_t *new_fg_job = job_init(g_shell_pgid, g_shell_pgid, STDERR_FILENO,  NULL);
    if(!new_fg_job) return 1;

    if(g_foreground_job) free_job(g_foreground_job);
    g_foreground_job = new_fg_job;
    return 0;
}


job_t *get_foreground_job(void){
    return g_foreground_job;
}


int set_foreground_job(job_t *new_fg_job){
    if(!new_fg_job) return 1;

    print_debug("%s is now foreground (pgid : %d)\n", new_fg_job->job_cmd, new_fg_job->pgid);

    if(g_foreground_job) free_job(g_foreground_job);     
    g_foreground_job = new_fg_job;

    return 0;
}

void free_foreground_job(void){
    free_job(g_foreground_job);
}


//  ----- MAIN JOB TABLE MANAGEMENT -----------------------------------------------
int set_main_job_table(job_table_t *job_tbl){
    if(!job_tbl) return 1;
    g_job_table = job_tbl;
    return 0;
}

job_table_t *get_job_table(void){
    return g_job_table;
}

int main_job_table_add(job_t *job){
    return job_table_add(g_job_table, job);
}

void main_job_table_rm(int job_id){
    job_table_rm(g_job_table, job_id);
}

void free_main_job_table(void){
    free_job_table(g_job_table);
}


//  ----- JOB CONTROL OPERATIONS -----------------------------------------------
int set_shell_foreground(void){

    job_t *new_fg_job = job_init(g_shell_pgid, g_shell_pgid, STDERR_FILENO,  NULL);
    if(!new_fg_job) return 1;

    if(g_foreground_job) free_job(g_foreground_job);
    g_foreground_job = new_fg_job;

    tcsetpgrp(STDIN_FILENO, g_shell_pgid);

    print_debug("Shell is now foreground (shell_pg_id : %d)\n", g_shell_pgid);

    return 0;
}


int suspend_job(job_table_t *job_tbl, job_t *job){

    job->status = SUSPENDED;
    if(job_table_add(job_tbl, job) <= 0) return 1;

    printf("\nolive-sh: suspended %s\n", job->job_cmd);
    
    return 0;
}