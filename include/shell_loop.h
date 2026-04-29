#ifndef SHELL_LOOP 
#define SHELL_LOOP 

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "token_chain.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "printer.h"

int run_shell();

#endif  