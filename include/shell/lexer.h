#ifndef LEXER
#define LEXER

#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "token_chain.h"
#include "printer.h"


/**
 * @file lexer.h
 * @brief Lexer for a POSIX-compliant shell.
 * 
 * Transforms a raw input string into a token chain, handling variable 
 * detection, quoting rules and shell operators.
 * See also: lexer_internal.h, lexer_rules.h
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
 * @brief Result of a lexing operation. 
 */
typedef struct lexer_res_s {
    bool            success;        /** true if lexing succeeded, false otherwise. */
    token_chain_t   *tk_chain;      /** The built token chain, only valid if success is true. */
    char            *error;         /** Human readable encountered error, set only on failure. */
    char            *error_info;    /** Additional context for the error. */
    size_t          error_pos;      /** The position in the command where the error was met. */
} lexer_res_t; 


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
    IN_DOLLAR_IN_DB_QUOTE,      /** Reading a variable name after '$' inside quotes. */
    IN_BRACES_IN_DB_QUOTE       /** Reading a variable name inside ${...} inside quotes. */

} lexer_state_e;

/**
 * @brief Tokenizes a raw input string into a lexer response struct.
 *
 * @param raw_input   The input string to lex.
 * @param input_len   Length of `raw_input`.
 * @return A lexer response containing the token chain. If an error occured, success is set to false and
 * error fields contains the details.
 */
lexer_res_t lex_input(char *raw_input, size_t input_len);


#endif