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
#include "exec_result.h"
#include "exec_internal.h"
#include "exec_rules.h"


/**
 * @file executor.h
 * @brief Executor for a POSIX-compliant shell. 
 * 
 * Recursive implementation of an AST executor. Supports standard operators, pipelines, 
 * options and job control.
 * See also: exec_result.h, exec_internal.h, exec_rules.h
 */

/**
 * @brief Runs an AST recursively. 
 * 
 * @param env            A pointer to the process environment. 
 * @param ast            A pointer to the AST to run.
 * @param std_fd_in      Process input file descriptor. 
 * @param std_fd_out     Process output file descriptor. 
 * @param std_fd_err     Process error output file descriptor. 
 * 
 * @return The execution result of the AST. 
 */
exec_res_t run_ast(env_t *env, ast_node_t *ast, int std_fd_in, int std_fd_out, int std_fd_err);

#endif