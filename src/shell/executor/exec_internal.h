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

int run_builtin(int id, int argc, char **argv, env_t *env, 
    int fd_in, int fd_out);

// must be ran from child process
int run_cmd_async(env_t *env, char **argv, char **envp, 
    int fd_in, int fd_out, int fd_err);

exec_res_t run_pipe_children(env_t *env, ast_node_t *ast, 
    pid_t group_pgid, 
    int std_fd_in, int std_fd_out, int err_out_fd);

int setup_redirs(env_t *env, ast_node_t *cmd_node, int *fd_in, int *fd_out);
int setup_params(env_t *env, ast_node_t *cmd_node, int *argc, char ***argv, char ***envp);
void relay_child_error(int err_pipe_fd, int target_fd);

#endif