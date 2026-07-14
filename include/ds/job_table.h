#ifndef JOB_TABLE
#define JOB_TABLE

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @file job_table.h
 * @brief Job and job table operations.
 * 
 * Define the job and job table type, and provide functions to build and 
 * manipulate job tables in the executor. 
 */

// ----- TYPES --------------------------------------------------------

/**
 * @brief Status of a job execution, determining whether it is running or not. 
 */
typedef enum job_status {
    RUNNING,
    SUSPENDED,
    DONE
} job_status_e;

/**
 * @brief Type of a job. 
 */
typedef struct job_s {
    int job_id;             /** job id provided by the shell to the user */
    pid_t pgid;             /** job group pid */
    pid_t leader_pid;       /** job leading process pid */
    int leader_std_err_fd;  /** job leading process standard error output fd */
    job_status_e status;    /** job execution status */
    char *job_cmd;          /** job underlying command */
} job_t; 

/**
 * @brief Type of a job table.
 * @note The job table is implemented as a dynamic table. The capacity field usage is only relevant 
 *  for internal table operations (ex. : adding an element).
 */
typedef struct job_table_s {
    size_t capacity;   /** Table current maximal size */
    job_t **tbl;       /** Job table */
} job_table_t;


//  ----- JOB MANAGEMENT -----------------------------------------------
/**
 * @brief Creates a new job
 * @warning The created job will have it's job id set to -1. Job id assignation occurs when the job is 
 *  added to a job table.
 * @note If the given command is NULL, an empty string will be assigned to the corresponding field. 
 * @return A pointer to the new job, or NULL on error. 
 */
job_t *job_init(pid_t pgid, pid_t leader_pid, int std_err_fd, char *job_cmd);     

/**
 * @brief Frees the job referenced by the given pointer. 
 */
void free_job(job_t *job);

/**
 * @brief Update the value of a job command. 
 * @note This function will duplicate the given command. If the command is NULL, 
 * the job command will be set to an empty string. 
 */
void update_job_cmd(job_t *job, char *cmd);


//  ----- JOB MANAGEMENT -----------------------------------------------

/**
 * @brief Creates a fresh job table. 
 * @return Returns a pointer to the table, or NULL on error. 
 */
job_table_t *job_table_init(void);

/**
 * @brief Frees the job table referenced by the given pointer. 
 * @note All the jobs contained in the table will also be freed. 
 */
void free_job_table(job_table_t *job_tbl);

/**
 * @brief Adds a job to a job table by filling the first gap found, or expaning the table size. 
 * @return Returns the job id of the added job, i.e. its position in the table. On error, returns -1. 
 * @note The "job_id" field of the added job is filled automatically by this function. 
 */
int job_table_add(job_table_t *job_tbl, job_t *job);

/**
 * @brief Removes a job from the job table. 
 */
void job_table_rm(job_table_t *job_tbl, int job_id);


#endif