#ifndef LEXER_INTERNAL
#define LEXER_INTERNAL

#include <stdarg.h>
#include "lexer.h"
#include "lexer_rules.h"
#include "token_chain.h"

/**
 * @file lexer_internal.h
 * @brief Internal operations used by the lexer in complex cases. 
 */


/**
 * @brief Handles the dollar lexing case.
 * 
 * @param lex_res     A pointer to the lexer result struct being built.
 * @param cur_node    A pointer to the token node being completed.
 * @param cur_char    Current read character.
 * @param cursor      A pointer to the reading cursor.
 * @param pos         A pointer to the current writing position.
 * @param cur_state   A pointer to the lexer current state.
 * @param in_dq       A boolean indicating if the dollar is lexed inside double quotes.
 * 
 * @return LEX_OK if lexing was successful, corresponding error status otherwise.
 */
lex_exit_status_e handle_dollar(lexer_res_t *lex_res, token_node_t *cur_node, char cur_char, 
    size_t *cursor, size_t *pos, lexer_state_e *cur_state, bool in_dq);


/**
 * @brief Handles the braces lexing case.
 * 
 * @param lex_res     A pointer to the lexer result struct being built.
 * @param cur_node    A pointer to the token node being completed.
 * @param cur_char    Current read character.
 * @param raw_input   User's input.
 * @param cursor      A pointer to the reading cursor.
 * @param pos         A pointer to the current writing position.
 * @param cur_state   A pointer to the lexer current state.
 * @param in_dq       A boolean indicating if the braces are lexed inside double quotes.
 * 
 * @return LEX_OK if lexing was successful, corresponding error status otherwise.
 */
lex_exit_status_e handle_braces(lexer_res_t *lex_res, token_node_t *cur_node, char cur_char, char *raw_input,
    size_t *cursor, size_t *pos, lexer_state_e *cur_state, bool in_dq);


/**
 * @brief Flushes the current token when lexing ends.
 * 
 * @param lex_res     A pointer to the built lexer result.
 * @param cur_state   The lexer current state.
 * @param pos         A pointer to the current writing position.
 * 
 * @return LEX_OK if flushing was successful, corresponding error status otherwise.
 */
lex_exit_status_e flush_current_token(lexer_res_t *lex_res, lexer_state_e cur_state, size_t *pos);


/**
 * @brief Assign the given formatted input to the lexer result error field.
 */
void set_lex_res_error(lexer_res_t *lex_res, char *fmt, ...);

/**
 * @brief Assign the given formatted input to the lexer result error information field.
 */
void set_lex_res_error_info(lexer_res_t *lex_res, char *fmt, ...);

#endif