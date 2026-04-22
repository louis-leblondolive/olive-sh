#ifndef LEXER
#define LEXER

#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "token_list.h"
#include "printer.h"

typedef enum lex_exit_status {

    LEX_OK,
    LEX_UNKNOWN_ERROR,
    LEX_UNKNOWN_OP,
    LEX_INVALID_SUBST,
    LEX_BRC_END_NOT_FOUND,
    LEX_SQ_END_NOT_FOUND,
    LEX_DQ_END_NOT_FOUND,
    LEX_TOO_LONG,
    LEX_EMPTY_ESCAPE,
    LEX_FATAL

} lex_exit_status_e;

typedef enum lexer_state {

    START,
    WORD,
    OPERATOR,
    ESCAPE,
    IN_DOLLAR,
    IN_BRACES,
    IN_SG_QUOTE,
    IN_DB_QUOTE,
    ESCAPE_IN_DB_QUOTE,
    IN_DOLLAR_IN_DB_QUOTE,
    IN_BRACES_IN_DB_QUOTE


} lexer_state_e;


lex_exit_status_e build_token_list(char *raw_input, size_t input_len, token_chain_t *tk_chain);
const char *lex_exit_status_to_str(lex_exit_status_e ex_st);

#endif