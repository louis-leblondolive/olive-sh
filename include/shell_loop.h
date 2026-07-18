#ifndef SHELL_LOOP 
#define SHELL_LOOP 

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>

#include "env.h"
#include "token_chain.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "executor.h"
#include "signals.h"
#include "job_ctl.h"
#include "printer.h"

/**
 * @file shell_loop.h
 * @brief Main REPL loop of the shell. 
 * 
 * Manages the read-eval-print cycle : reads user input via readline, lexes it into a 
 * token chain then parsed into an AST, executes it and handles interuptions and job control 
 * between cycles. 
 * See also: lexer.h, parser.h, executor.h, signals.h, job_ctl.h
 */

/**
 * @brief Runs the shell main read-eval-print loop until exit. 
 * 
 * @param envp The calling process environment, used to initialize the shell environment. 
 * 
 * @return The shell exit code. 
 */
int run_shell(char **envp);

#endif  