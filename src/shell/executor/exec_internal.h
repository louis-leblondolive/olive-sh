#ifndef EXEC_INTERNAL
#define EXEC_INTERNAL

#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ast.h"
#include "builtins.h"
#include "env.h"
#include "signals.h"
#include "printer.h"

int run_cmd(env_t *env, ast_node_t *cmd_node);

#endif