#ifndef PARSER
#define PARSER


#include "config.h"
#include "lexer.h"
#include "token_chain.h"
#include "ast.h"

/**
 * @file parser.h
 * @brief Parser for a POSIX-compliant shell. 
 * 
 * Transforms a token chain into an AST using recursive descent, following 
 * POSIX shell grammar priority rules : command < pipe < && < || < ; / &. 
 */


/**
 * @brief Result of a parsing operation. 
 */
typedef struct parse_res_s {    
    bool          success;      /** true if parsing succeeded, false otherwise. */
    ast_node_t    *ast;         /** The built AST, only valid if success is true. */
    char          *error;       /** Human readable encountered error, set only on failure.  */
    char          *error_info;  /** Additional error context. */
} parse_res_t;


/**
 * @brief Builds an AST from a lexed input. 
 * 
 * @param tk_chain   The tokenized input obtained from the lexer. 
 * 
 * @return A parser response containing the AST. On error, its success field is set to false and 
 * its error and error_info fields contain details about the encountered error. 
 */
parse_res_t build_ast(token_chain_t *tk_chain);

#endif