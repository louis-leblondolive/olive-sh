#ifndef JOB_CTL 
#define JOB_CTL 

#include <stdbool.h>
#include <unistd.h>

#include "job_table.h"
#include "printer.h"

extern pid_t g_shell_pgid; 
extern job_t *g_foreground_job;

bool is_shell_foreground(void);
int set_shell_foreground(void);
int update_foreground_job(job_t *new_fg_job);
int suspend_job(job_table_t *job_tbl, job_t *job);

#endif