#ifndef PARSER_INTERNAL
#define PARSER_INTERNAL

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"
#include "token_chain.h"
#include "ast.h"

/**
 * @file parser_internal.h
 * @brief Internal operations used during the parser recursive descent. 
 */


/**
 * @brief Builds an AST command leaf from a token chain.
 * 
 * @param tk_chain    Pointer to the parsed token chain.
 * @param parse_res   Pointer to the parsing result being built. 
 * 
 * @warning The token chain will be affected as parsing will consume tokens. 
 * 
 * @return The AST command leaf, with argument and redirections fields set. 
 * On error, NULL is returned and parsing result error fields are set to the corresponding error.  
 */
ast_node_t *build_cmd_node(token_chain_t *tk_chain, parse_res_t *parse_res);


/**
 * @brief Builds an AST pipe node from a token chain. 
 * 
 * @param cur_ast     Pointer to the currently built AST. 
 * @param tk_chain    Pointer to the parsed token chain. 
 * @param parse_res   Pointer to the parsing result being built. 
 * 
 * @warning The token chain will be affected as parsing will consume tokens. 
 * 
 * @return The resulting AST node. If the token chain contains no pipe operator, the result may be 
 * a single command node rather than an actual pipe node. 
 * On error, NULL is returned and parsing result error fields are set to the corresponding error. 
 */
ast_node_t *build_pipe_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);


/**
 * @brief Builds an AST && node from a token chain. 
 * 
 * @param cur_ast     Pointer to the currently built AST. 
 * @param tk_chain    Pointer to the parsed token chain. 
 * @param parse_res   Pointer to the parsing result being built. 
 * 
 * @warning The token chain will be affected as parsing will consume tokens. 
 * 
 * @return The resulting AST node. If the token chain doesn't contain an && operator, the result may 
 * be a lower-priority operator node (pipe) or simply a command node. 
 * On error, NULL is returned and parsing result error fields are set to the corresponding error. 
 */
ast_node_t *build_and_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);


/**
 * @brief Builds an AST || node from a token chain. 
 * 
 * @param cur_ast     Pointer to the currently built AST. 
 * @param tk_chain    Pointer to the parsed token chain. 
 * @param parse_res   Pointer to the parsing result being built. 
 * 
 * @warning The token chain will be affected as parsing will consume tokens. 
 * 
 * @return The resulting AST node. If the token chain doesn't contain an || operator, the result may 
 * be a lower-priority operator node (&& or pipe) or simply a command node. 
 * On error, NULL is returned and parsing result error fields are set to the corresponding error. 
 */
ast_node_t *build_or_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);


/**
 * @brief Builds an AST delimiter (; and &) node from a token chain. 
 * 
 * @param cur_ast     Pointer to the currently built AST. 
 * @param tk_chain    Pointer to the parsed token chain. 
 * @param parse_res   Pointer to the parsing result being built. 
 * 
 * @warning The token chain will be affected as parsing will consume tokens. 
 * 
 * @return The resulting AST node. If the token chain doesn't contain '&' or ';' operator, the result may 
 * be a lower-priority operator node (||, && or pipe) or simply a command node. 
 * On error, NULL is returned and parsing result error fields are set to the corresponding error. 
 */
ast_node_t *build_delim_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res);


/**
 * @brief Assign the given formatted string to the parser result error field.  
 */
void set_parse_res_error(parse_res_t *parse_res, char *fmt, ...);


/**
 * @brief Assign the given formatted string to the parser result error information field.  
 */
void set_parse_res_error_info(parse_res_t *parse_res, char *fmt, ...);

#endif