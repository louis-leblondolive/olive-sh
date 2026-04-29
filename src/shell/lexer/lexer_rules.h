#ifndef LEXER_RULES
#define LEXER_RULES

#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "token_chain.h"

/**
 * @brief Determines if a character c is an operator.
 */
bool is_operator(char c);

/**
 * @brief Returns the token associated to an operator symbol.
 * @example "|" -> TOKEN_PIPE, "&&" -> TOKEN_AND, ...
 * @return The associated token, or TOKEN_WORD if it doesn't exist.  
 */
token_e get_operator_token(char *op);

/**
 * @brief Determines if a character c can be used in a shell variable name.
 */
bool is_valid_var_char(char c);

#endif