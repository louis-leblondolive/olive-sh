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
#include "exec_rules.h"

int run_builtin(int id, int argc, char **argv, env_t *env, int fd_in, int fd_out);
pid_t run_cmd_async(env_t *env, char **argv, char **envp, int fd_in, int fd_out, int fd_err);
//int run_pipe(env_t *env, ast_node_t *pipe_node);

#endif