#ifndef EXEC_INTERNAL
#define EXEC_INTERNAL

#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "token_chain.h"
#include "ast.h"
#include "builtins.h"
#include "env.h"
#include "signals.h"
#include "job_ctl.h"
#include "printer.h"
#include "exec_result.h"
#include "exec_rules.h"

/**
 * @file exec_internal.h
 * @brief Internal operations used during AST execution. 
 */


/**
 * @brief Runs a builtin command. 
 * 
 * @param id        The builtin command index in the builtin table (see builtins.h)
 * @param argc      The number of argument passed to the command. 
 * @param argv      The arguments of the command. Must be a NULL terminated string table, 
 *                      with at least `argc` elements.
 * @param env       A pointer to the environment. 
 * @param fd_in     Command input file descriptor. 
 * @param fd_out    Command output file descriptor. 
 * 
 * @return Returns an execution result with the proper fields set depending on command exit.  
 */
exec_res_t run_builtin(int id, int argc, char **argv, env_t *env, int fd_in, int fd_out);


/**
 * @brief Runs an external command. 
 * 
 * @param env       A pointer to the environment.
 * @param argv      The arguments of the command. Must be a NULL terminated string table, 
 *                    containing the command name at index 0. 
 * @param envp      The environment, represented as a NULL terminated "VAR=value" formatted string table. 
 * @param fd_in     Command input file descriptor. 
 * @param fd_out    Command output file descriptor. 
 * @param fd_err    Command error output file descriptor. 
 * 
 * @warning This function will execve without fork or waitpid. These operations are to be 
 *  handled by the user in the calling process. 
 * 
 * @return Returns the function standard exit code, to be exited by the calling forked process 
 *      and caught by waitpid. 
 */
int run_cmd_async(env_t *env, char **argv, char **envp, int fd_in, int fd_out, int fd_err);


/**
 * @brief Forks, connects, runs and waits for both sides of an AST pipe node. 
 * 
 * @param env           A pointer to the environment. 
 * @param ast           A pointer to the pipe node to run. 
 * @param group_pgid    The pgid of the pipeline, which will be assigned to child processes. 
 * @param std_fd_in     Pipeline input file descriptor. 
 * @param std_fd_out    Pipeline output file descriptor. 
 * @param err_out_fd    Pipeline error output file descriptor. 
 * 
 * @return Returns an execution result with proper fields assigned depending on the exit. 
 */
exec_res_t run_pipe_children(env_t *env, ast_node_t *ast, pid_t group_pgid, 
    int std_fd_in, int std_fd_out, int err_out_fd);


/**
 * @brief Runs an AST in the background. 
 * 
 * @param env           A pointer to the environment. 
 * @param ast           A pointer to the AST to run background. 
 * @param std_fd_in     Process input file descriptor. 
 * @param std_fd_out    Process output file descriptor. 
 * 
 * @return The AST execution result. 
 */
exec_res_t run_ast_background(env_t *env, ast_node_t *ast, int std_fd_in, int std_fd_out);


/**
 * @brief Sets up the redirections of a command. 
 * 
 * @param env         A pointer to the environment. 
 * @param cmd_node    A pointer to the command node. 
 * @param fd_in       A pointer to the command input file descriptor, assigned by this function. 
 * @param fd_out      A pointer to the command output file descriptor, assigned by this function. 
 * 
 * @return 0 on success, -1 otherwise. 
 */
int setup_redirs(env_t *env, ast_node_t *cmd_node, int *fd_in, int *fd_out);


/**
 * @brief Formats command arguments and environment. 
 * 
 * @param env         A pointer to the environment. 
 * @param cmd_node    A pointer to the command node. 
 * @param argc        A pointer to the argc field to be assigned by this function. 
 * @param argv        A pointer to the argv table to be allocated and filled by this function. 
 * @param envp        A pointer to the envp table to be allocated and filled by this function. 
 * 
 * @return 0 on success, -1 otherwise. 
 */
int setup_params(env_t *env, ast_node_t *cmd_node, int *argc, char ***argv, char ***envp);


/**
 * @brief Relays a child process error from its error output to the targeted file descriptor. 
 */
void relay_child_error(int err_pipe_fd, int target_fd);

#endif