#ifndef PARSER
#define PARSER

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>

#include "config.h"
#include "lexer.h"
#include "token_list.h"
#include "ast.h"


typedef struct parse_res_s {    
    bool          success;
    ast_node_t    *ast;
    char          error[MAX_ERROR_LEN];
} parse_res_t;

parse_res_t build_ast(token_chain_t *tk_chain);

#endif