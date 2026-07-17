#ifndef JOB_CTL
#define JOB_CTL

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>

#include "job_table.h"
#include "printer.h"

/**
 * @file job_ctl.h
 * @brief Job control operations.
 * 
 * @note This module uses internal variables which can be accessed using the given API to avoid
 * unsafe global variable modifications.
 * 
 * Defines the job control API and operations to handle foreground and background processes.
 * See also: job_table.h
 */

// ----- SHELL OPERATIONS -----------------------------------------------------------
/**
 * @brief Saves the shell pid into an internal variable.
 * @note This function must be run from the main process of the shell to save the correct pid. 
 */
int save_shell_pid(void);

/**
 * @brief Determines if the shell is the current foreground process.
 */
bool is_shell_foreground(void);

// ----- FOREGROUND JOB OPERATIONS ---------------------------------------------------
/**
 * @brief Initializes the foreground job internal variable. 
 * @return 0 on success, 1 otherwise. 
 */
int init_foreground_job(void);

/**
 * @brief Provides a pointer to the internal variable containing the current foreground job. 
 */
job_t *get_foreground_job(void);

/**
 * @brief Sets the foreground job to the given pointer. 
 * @warning If a job is already foreground, it will be freed. 
 */
int set_foreground_job(job_t *fg_job);

/** 
 * @brief Frees the foreground job. 
 */
void free_foreground_job(void);

// ----- JOB TABLE OPERATIONS --------------------------------------------------------
/**
 * @brief Sets the main job table to the given pointer.
 * @return 0 on success, 1 otherwise.
 */
int set_main_job_table(job_table_t *job_tbl);

/**
 * @brief Provides a pointer to the internal variable containing the current main job table.
 */
job_table_t *get_main_job_table(void);

/**
 * @brief Adds a job to the main job table. 
 * @return On success, the index of the job in the table (>= 1). -1 otherwise. 
 * @warning The job pointed to by the given pointer will not be copied. 
 */
int main_job_table_add(job_t *job);

/**
 * @brief Removes a job at a given index from the main job table. 
 * @warning The removed job will be freed. 
 * @warning The table capacity may be decreased by the operation. See job_table.h for more details. 
 */
void main_job_table_rm(int job_id);

/**
 * @brief Frees the main job table. 
 */
void free_main_job_table(void);

// ----- JOB CONTROL OPERATIONS ------------------------------------------------------
/**
 * @brief Sets the shell as the main foreground job.
 * @return 0 on success, 1 otherwise. 
 */
int set_shell_foreground(void);

/**
 * @brief Suspends a job. 
 * @return 0 on success, 1 otherwise. 
 * @warning This function will not send signals to stop the job. 
 */
int suspend_job(job_t *job);

/**
 * @brief Roams the main job table to collect and remove done jobs. 
 * @return A boolean indicating if at least one job was removed. 
 */
bool check_done_jobs(void);

/**
 * @brief Readline event hook roaming the main job table to collect and remove 
 * done jobs. 
 * @return 0 on success, 1 otherwise. 
 */
int check_done_jobs_readline_hook(void);



#endif