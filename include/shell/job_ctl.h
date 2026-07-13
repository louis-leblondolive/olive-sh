#ifndef JOB_CTL 
#define JOB_CTL 

#include <stdbool.h>
#include <unistd.h>

#include "job_table.h"
#include "printer.h"


// Shell operations 
int save_shell_pid(void);
bool is_shell_foreground(void);

// Foreground job operations 
int init_foreground_job(void);
job_t *get_foreground_job(void);
int set_foreground_job(job_t *fg_job);
void free_foreground_job(void);

// Job table operations 
int set_main_job_table(job_table_t *job_tbl);
job_table_t *get_main_job_table(void);
int main_job_table_add(job_t *job);
void main_job_table_rm(int job_id);
void free_main_job_table(void);

// Job control operations 
int set_shell_foreground(void);
int suspend_job(job_table_t *job_tbl, job_t *job);

#endif