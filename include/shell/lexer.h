#ifndef LEXER
#define LEXER

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "token_list.h"
#include "printer.h"


/**
 * @file lexer.h
 * @brief Lexer for a POSIX-compliant shell.
 * 
 * Transforms a raw input string into a token chain, handling variable 
 * detection, quoting rules and shell operators.
 */


/**
 * @brief Exit status codes returned by the lexer.
 */
typedef enum lex_exit_status {

    LEX_OK,                     /** Lexing completed successfully. */
    LEX_UNKNOWN_ERROR,          /** Unexpected internal error.  */
    LEX_UNKNOWN_OP,             /** Unrecognized operator token. */
    LEX_INVALID_SUBST,          /** Invalid variable substition (e.g. ${?x}) */
    LEX_BRC_END_NOT_FOUND,      /** EOF reached while looking for closing '}'. */
    LEX_SQ_END_NOT_FOUND,       /** EOF reached while looking for closing '\''. */
    LEX_DQ_END_NOT_FOUND,       /** EOF reached while looking for closing '"'. */
    LEX_TOO_LONG,               /** Command length exceeded MAX_WORD_LENGTH. */
    LEX_EMPTY_ESCAPE,           /** EOF reached after '\\'. */
    LEX_FATAL                   /** Memory allocation or system error. */

} lex_exit_status_e;


/**
 * @brief Internal states of the lexer's finite state machine.
 */
typedef enum lexer_state {

    START,                      /** Between token, waiting for next word. */
    WORD,                       /** Reading a plain word token. */
    OPERATOR,                   /** Reading an operator token. */
    ESCAPE,                     /** Reading an escaped character outside quotes. */
    IN_DOLLAR,                  /** Reading a variable name after '$' outside quotes. */
    IN_BRACES,                  /** Reading a variable name inside ${...} outside quotes. */
    IN_SG_QUOTE,                /** Inside single quotes. */
    IN_DB_QUOTE,                /** Inside double quotes. */
    ESCAPE_IN_DB_QUOTE,         /** Reading an escaped character inside quotes. */
    IN_DOLLAR_IN_DB_QUOTE,      /** Reading a variable name after '$' outside quotes. */
    IN_BRACES_IN_DB_QUOTE       /** Reading a variable name inside ${...} outside quotes. */


} lexer_state_e;

/**
 * @brief Tokenizes a raw input string into a linked token chain.
 *
 * @param raw_input   The input string to lex.
 * @param input_len   Length of `raw_input`.
 * @param tk_chain    Output token chain to populate.
 * @return LEX_OK if lexing was successful, corresponding error status otherwise.
 * 
 * @warning Token chain lifecycle is user's responsibility, it will not be freed in this function and 
 * must be initialized before call. 
 */
lex_exit_status_e build_token_list(char *raw_input, size_t input_len, token_chain_t *tk_chain);

/**
 * @brief Returns a human-readable string for a given lexer exit status.
 *
 * @param ex_st   The exit status to describe.
 * @return A static string describing the status.
 */
const char *lex_exit_status_to_str(lex_exit_status_e ex_st);

#endif