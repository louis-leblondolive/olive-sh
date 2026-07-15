# ifndef EXEC_RULES
# define EXEC_RULES

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "ast.h"
#include "exec_result.h"
#include "builtins.h"
#include "printer.h"

/** 
 * @file exec_rules.h 
 * 
 * @brief Provides utilitaries functions and rule verifiers used in the executor. 
 */


/**
 * @brief Determines whether a command is builtin or not. 
 * @return If the command is builtin, returns its index in the builtin list. Otherwise, returns -1. 
 */
int is_builtin(char *cmd_name);

/**
 * @brief Attempt to find command path in the PATH directories. 
 * @return NULL on error, command path otherwise 
 */
char *find_cmd_path(env_t *env, char *cmd);


/**
 * @brief Frees argv and envp. 
 */
void clean_exec_vars(char **argv, char **envp);

/**
 * @brief Closes fd_in and fd_out if they are not standard I/O, i.e. > 2 and different from 
 * the default value provided. 
 */
void clean_io_fds(int fd_in, int fd_out, int default_fd_in, int default_fd_out);

/**
 * @brief Open a pipe with CLOEXEC flag. 
 */
int open_cloexec_pipe(int fds[2]);

/**
 * @brief Closes both ends of a pipe.
 */
void close_pipe(int fds[2]);




#endif