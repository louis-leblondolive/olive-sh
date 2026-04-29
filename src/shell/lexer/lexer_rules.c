#include "lexer_rules.h"


bool is_operator(char c){
    return c == '|' || c == '&' || c == '<' || c == '>' || c == ';';
}


token_e get_operator_token(char *op){

    if(strncmp(op, "|", MAX_WORD_LENGTH) == 0) return TOKEN_PIPE;
    if(strncmp(op, "<", MAX_WORD_LENGTH) == 0) return TOKEN_REDIR_IN;
    if(strncmp(op, ">", MAX_WORD_LENGTH) == 0) return TOKEN_REDIR_OUT;
    if(strncmp(op, "<<", MAX_WORD_LENGTH) == 0) return TOKEN_HEREDOC;
    if(strncmp(op, ">>", MAX_WORD_LENGTH) == 0) return TOKEN_APPEND;
    if(strncmp(op, "&&", MAX_WORD_LENGTH) == 0) return TOKEN_AND;
    if(strncmp(op, "||", MAX_WORD_LENGTH) == 0) return TOKEN_OR;
    if(strncmp(op, ";", MAX_WORD_LENGTH) == 0) return TOKEN_SEQ;
    
    return TOKEN_WORD;
}


bool is_valid_var_char(char c){
    return c == '_' || isalnum(c);
}