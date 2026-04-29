#ifndef LEXER_INTERNAL
#define LEXER_INTERNAL

#include "lexer.h"
#include "lexer_rules.h"
#include "token_chain.h"

/**
 * @brief Handles the dollar lexing case.
 * 
 * @param cur_node    A pointer to the token node being completed.
 * @param cur_char    Current read character.
 * @param cursor      A pointer to the reading cursor.
 * @param pos         A pointer to the current writing position.
 * @param cur_state   A pointer to the lexer current state.
 * @param in_dq       A boolean indicating if the dollar is lexed inside double quotes.
 * 
 * @return LEX_OK if lexing was successful, corresponding error status otherwise.
 */
lex_exit_status_e handle_dollar(token_node_t *cur_node, char cur_char, 
    size_t *cursor, size_t *pos, lexer_state_e *cur_state, bool in_dq);


/**
 * @brief Handles the braces lexing case.
 * 
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
lex_exit_status_e handle_braces(token_node_t *cur_node, char cur_char, char *raw_input,
    size_t *cursor, size_t *pos, lexer_state_e *cur_state, bool in_dq);


/**
 * @brief Flushes the current token when lexing ends.
 * 
 * @param tk_chain    A pointer to the built token chain.
 * @param cur_state   The lexer current state.
 * @param pos         A pointer to the current writing position.
 * 
 * @return LEX_OK if flushing was successful, corresponding error status otherwise.
 */
lex_exit_status_e flush_current_token(token_chain_t *tk_chain, lexer_state_e cur_state, size_t *pos);


#endif