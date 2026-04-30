#ifndef EXECUTOR
#define EXECUTOR

#include "token_chain.h"
#include "env.h"
#include "ast.h"

int run_ast(env_t *env, ast_node_t *ast);

#endif