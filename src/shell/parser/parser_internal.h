#ifndef PARSER_INTERNAL
#define PARSER_INTERNAL

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "token_chain.h"
#include "ast.h"

void set_parse_res_error(parse_res_t *parse_res, char *fmt, ...);
ast_node_t *build_cmd_node(token_chain_t *tk_chain, parse_res_t *parse_res);
ast_node_t *build_pipe_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);
ast_node_t *build_and_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);
ast_node_t *build_or_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);
ast_node_t *build_seq_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);

#endif