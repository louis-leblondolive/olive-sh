#ifndef EXECUTOR
#define EXECUTOR

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "token_chain.h"
#include "env.h"
#include "ast.h"
#include "job_table.h"
#include "job_ctl.h"
#include "exec_internal.h"
#include "exec_rules.h"

int run_ast(env_t *env, ast_node_t *ast, int std_fd_in, int std_fd_out, int std_fd_err);

#endif