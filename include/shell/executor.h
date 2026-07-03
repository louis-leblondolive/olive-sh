#ifndef EXECUTOR
#define EXECUTOR

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "token_chain.h"
#include "env.h"
#include "ast.h"
#include "exec_internal.h"
#include "exec_rules.h"

int run_ast(env_t *env, ast_node_t *ast, int fd_in, int fd_out);

#endif