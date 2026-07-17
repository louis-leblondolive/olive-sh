#ifndef EXEC_RESULT
#define EXEC_RESULT

#include <sys/wait.h>


/**
 * @file exec_result.h
 * @brief Execution result type shared accoss the executor module. 
 */


/**
 * @brief Label for exit possibilities.
 */
typedef enum {
     RES_EXITED,    /** Regular exit */
     RES_SIGNALED,  /** Process terminated by a signal */
     RES_STOPPED    /** Process stopped by a signal */
} res_kind_e;


/**
 * @brief Type of an execution result.
 * @warning  This structure is a labelled union. Always check result kind before trying to access 
 * exit information fields. 
 */
typedef struct exec_res_s {
     res_kind_e kind;    /** Process exit label */
     union {
          int exit_code;      /** In case of a regular exit (RES_EXITED), process exit code */
          int term_sig;       /** In case of a signal termination (RES_SIGNALED), signal id  */
          int stop_sig;       /** In case of a signal pause (RES_STOPPED), signal id  */
     };
} exec_res_t;


/**
 * @brief Returns an execution result corresponding to a regular exit with the given exit_code.  
 */
exec_res_t exec_res_from_builtin(int exit_code);

/**
 * @brief Converts a waitpid status into an execution result, depending on process exit reason. 
 */
exec_res_t exec_res_from_waitpid_status(int status);

#endif