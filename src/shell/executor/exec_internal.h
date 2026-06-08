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
#include "printer.h"

int run_cmd(env_t *env, ast_node_t *cmd_node);
pid_t run_cmd_async(env_t *env, ast_node_t *cmd_node, int fd_in, int fd_out, int fd_err);
int run_pipe(env_t *env, ast_node_t *pipe_node);

#endif