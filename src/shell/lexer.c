#include "lexer.h"


/**
 * @brief Determines if a character c is an operator.
 */
static bool is_operator(char c){
    return c == '|' || c == '&' || c == '<' || c == '>' || c == ';';
}


/**
 * @brief Returns the token associated to an operator symbol.
 * @example "|" -> TOKEN_PIPE, "&&" -> TOKEN_AND, ...
 * @return The associated token, or TOKEN_WORD if it doesn't exist.  
 */
static token_e get_operator_token(char *op){

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

/**
 * @brief Determines if a character c can be used in a shell variable name.
 */
static bool is_valid_var_char(char c){
    return c == '_' || isalnum(c);
}

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
static lex_exit_status_e handle_dollar(token_node_t *cur_node, char cur_char, 
    size_t *cursor, size_t *pos, lexer_state_e *cur_state, bool in_dq){

    if(cur_char == '{'){
        if(in_dq) *cur_state = IN_BRACES_IN_DB_QUOTE;
        else *cur_state = IN_BRACES;

        (*cursor) ++;
    }

    else if(in_dq && cur_char == '"'){
        if(*pos == 0){
            cur_node->last_seg->value[*pos] = '$';
            cur_node->last_seg->type = SEG_LITERAL;
            (*pos) ++;
        }

        cur_node->last_seg->value[*pos] = '\0';
        if(add_segment(cur_node) != 0) return LEX_FATAL;
        cur_node->last_seg->type = SEG_LITERAL;

        *cur_state = WORD;
        *pos = 0;
        (*cursor) ++;
    }

    else if(*pos == 0 && (cur_char == '$' || cur_char == '?' || cur_char == '0') ){
        cur_node->last_seg->value[*pos] = cur_char;
        cur_node->last_seg->value[*pos + 1] = '\0';

        if(add_segment(cur_node) != 0) return LEX_FATAL;
            cur_node->last_seg->type = SEG_LITERAL;

            if(in_dq) *cur_state = IN_DB_QUOTE;
            else *cur_state = WORD;

            *pos = 0;
            (*cursor) ++;
    }

    else if(!is_valid_var_char(cur_char)){

        if(*pos == 0){
            cur_node->last_seg->value[*pos] = '$';
            cur_node->last_seg->type = SEG_LITERAL;
            (*pos) ++;
        }
        cur_node->last_seg->value[*pos] = '\0';
        if(add_segment(cur_node) != 0) return LEX_FATAL;
        cur_node->last_seg->type = SEG_LITERAL;
                    
        if(in_dq) *cur_state = IN_DB_QUOTE;
        else *cur_state = WORD;

        *pos = 0;
    }

    else {
        cur_node->last_seg->value[*pos] = cur_char;
        (*pos) ++;
        (*cursor) ++;
    }

    return LEX_OK;
}

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
static lex_exit_status_e handle_braces(token_node_t *cur_node, char cur_char, char *raw_input,
    size_t *cursor, size_t *pos, lexer_state_e *cur_state, bool in_dq){

    if (cur_char == '$' || cur_char == '?' || cur_char == '0'){

            // '}' cannot be found 
            if(*cursor + 1 >= MAX_WORD_LENGTH) return LEX_TOO_LONG;

            if(raw_input[*cursor + 1] == '}'){
                cur_node->last_seg->value[*pos] = cur_char;
                cur_node->last_seg->value[*pos + 1] = '\0';

                if(add_segment(cur_node) != 0) return LEX_FATAL;
                cur_node->last_seg->type = SEG_LITERAL;

                if(in_dq) *cur_state = IN_DB_QUOTE;
                else *cur_state = WORD;

                *pos = 0;
                (*cursor) += 2;
            }
            else return LEX_INVALID_SUBST;
    }

    else if(cur_char == '}'){

        if(in_dq){
            cur_node->last_seg->value[*pos] = '\0';

            if(add_segment(cur_node) != 0) return LEX_FATAL;
            cur_node->last_seg->type = SEG_LITERAL;
            *cur_state = IN_DB_QUOTE;
            *pos = 0;
            (*cursor) ++; 

            return LEX_OK;
        }

        cur_node->last_seg->value[*pos] = '\0';

        // '}' is the last character, no need to look further
        if(*cursor + 1 >= MAX_WORD_LENGTH) return LEX_OK;
        
        if(!is_operator(raw_input[*cursor + 1]) && raw_input[*cursor + 1] != ' '){
            
            if(add_segment(cur_node) != 0) return LEX_FATAL;
            cur_node->last_seg->type = SEG_LITERAL;

            if(in_dq) *cur_state = IN_DB_QUOTE;
            else *cur_state = WORD;

            *pos = 0;
            (*cursor) ++;
        }
        else {
            *cur_state = START;
            (*cursor) ++;
        }            
    }

    else if(!is_valid_var_char(cur_char)){
        return LEX_INVALID_SUBST;
    }

    else{
        cur_node->last_seg->value[*pos] = cur_char;
        (*pos) ++;
        (*cursor) ++;
    }
        
    return LEX_OK;
}

/**
 * @brief Flushes the current token when lexing ends.
 * 
 * @param tk_chain    A pointer to the built token chain.
 * @param cur_state   The lexer current state.
 * @param pos         A pointer to the current writing position.
 * 
 * @return LEX_OK if flushing was successful, corresponding error status otherwise.
 */
static lex_exit_status_e flush_current_token(token_chain_t *tk_chain, lexer_state_e cur_state, size_t *pos){

    token_node_t *cur_node = tk_chain->last;
    token_e op_tk;

    switch(cur_state){
        case WORD:
            cur_node->token = TOKEN_WORD;                  
            cur_node->last_seg->value[*pos] = '\0';
            break;

        case OPERATOR:
            cur_node->last_seg->value[*pos] = '\0';
            op_tk = get_operator_token(cur_node->last_seg->value);
            if(op_tk == TOKEN_WORD) return LEX_UNKNOWN_OP;

            cur_node->token = op_tk;
            free_node_segment_chain(cur_node); // No need for buffer segment anymore
            break;

        case ESCAPE:
            return LEX_EMPTY_ESCAPE;

        case IN_DOLLAR:
            cur_node->token = TOKEN_WORD;   
            if(*pos == 0){
                cur_node->last_seg->value[*pos] = '$';
                cur_node->last_seg->type = SEG_LITERAL;
                (*pos) ++;
            }
            cur_node->last_seg->value[*pos] = '\0';
            break;
        
        case IN_BRACES:
            return LEX_BRC_END_NOT_FOUND;

        case IN_SG_QUOTE:
            return LEX_SQ_END_NOT_FOUND;

        case IN_DB_QUOTE:           /* FALLTHROUGH */
        case IN_DOLLAR_IN_DB_QUOTE:
        case IN_BRACES_IN_DB_QUOTE:
        case ESCAPE_IN_DB_QUOTE:
            return LEX_DQ_END_NOT_FOUND;   
            
        default:
            break;
    }

    return LEX_OK;
}


lex_exit_status_e build_token_list(char *raw_input, size_t input_len, token_chain_t *tk_chain){

    lexer_state_e cur_state = START;
    size_t cursor = 0;
    size_t pos = 0;
    char cur_char;
    token_node_t *cur_node;
    token_e op_tk;
    lex_exit_status_e res;  // intermediate result, used when calling external handler functions (for $ and {})
    
    while(cursor < input_len){

        if(pos >= MAX_WORD_LENGTH) return LEX_TOO_LONG;

        cur_char = raw_input[cursor];
        cur_node = tk_chain->last;

        switch(cur_state){

            case START:

                if(cur_char == ' '){ cursor ++; break; }

                pos = 0;
                if(add_token_node(tk_chain) != 0) return LEX_FATAL;
                if(add_segment(tk_chain->last) != 0) return LEX_FATAL;

                if(is_operator(cur_char)){      // next token is an operator
                    cur_state = OPERATOR;
                }
                else if (cur_char == '$'){      // next token is a variable
                    cur_state = IN_DOLLAR;
                    tk_chain->last->last_seg->type = SEG_VAR;
                    cursor ++;
                } 
                else {                          // next token is a word
                    cur_state = WORD;
                    tk_chain->last->last_seg->type = SEG_LITERAL;
                }
                break;


            case WORD:

                if(is_operator(cur_char) || cur_char == ' '){
                    cur_node->token = TOKEN_WORD;                  
                    cur_node->last_seg->value[pos] = '\0';
                    cur_state = START;
                }

                else if(cur_char == '\''){       
                    cur_node->last_seg->value[pos] = '\0';
                    if(add_segment(cur_node) != 0) return LEX_FATAL;
                    cur_node->last_seg->type = SEG_LITERAL;
                    
                    cur_state = IN_SG_QUOTE;
                    pos = 0;
                    cursor ++;
                }

                else if(cur_char == '"'){
                    if(pos > 0){
                        cur_node->last_seg->value[pos] = '\0';
                        if(add_segment(cur_node) != 0) return LEX_FATAL;
                    }
                    if(cursor + 1 >= MAX_WORD_LENGTH) return LEX_DQ_END_NOT_FOUND;

                    pos = 0;

                    if(raw_input[cursor + 1] == '$'){
                        cur_node->last_seg->type = SEG_VAR;
                        cur_state = IN_DOLLAR_IN_DB_QUOTE;
                        cursor += 2;
                    } else {
                        cur_node->last_seg->type = SEG_LITERAL;
                        cur_state = IN_DB_QUOTE;
                        cursor ++;
                    }
                }

                else if(cur_char == '\\'){
                    cur_state = ESCAPE;
                    cursor ++;
                }

                else if(cur_char == '$'){
                    cur_node->last_seg->value[pos] = '\0';

                    if(add_segment(cur_node) != 0) return LEX_FATAL;
                    cur_node->last_seg->type = SEG_VAR;

                    cur_state = IN_DOLLAR;
                    pos = 0;
                    cursor++;
                }   

                else{
                    cur_node->last_seg->value[pos] = cur_char;
                    pos ++;
                    cursor ++;
                }

                break;


            case OPERATOR:

                if(is_operator(cur_char)){
                    cur_node->last_seg->value[pos] = cur_char;
                    pos ++;
                    cursor ++;
                    break;
                }

                // cur char is not an operator 
                cur_node->last_seg->value[pos] = '\0';

                // Resolving operator  
                op_tk = get_operator_token(cur_node->last_seg->value);
                if(op_tk == TOKEN_WORD) return LEX_UNKNOWN_OP;

                cur_node->token = op_tk;

                free_node_segment_chain(cur_node); // No need for buffer segment anymore

                cur_state = START;
                break;


            case ESCAPE:
                if(cur_char == 'n') cur_node->last_seg->value[pos] = '\n';
                else if(cur_char == 'r') cur_node->last_seg->value[pos] = '\r';
                else if(cur_char == 't') cur_node->last_seg->value[pos] = '\t';
                else cur_node->last_seg->value[pos] = cur_char;

                cursor ++;
                pos ++;
                cur_state = WORD;
                break;


            case IN_DOLLAR:
                res = handle_dollar(cur_node, cur_char, &cursor, &pos, &cur_state, false);
                if(res != LEX_OK) return res;
                break;


            case IN_DOLLAR_IN_DB_QUOTE:
                res = handle_dollar(cur_node, cur_char, &cursor, &pos, &cur_state, true);
                if(res != LEX_OK) return res;
                break;


            case IN_BRACES:
                res = handle_braces(cur_node, cur_char, raw_input, &cursor, &pos, &cur_state, false);
                if(res != LEX_OK) return res;
                break;

            
            case IN_BRACES_IN_DB_QUOTE:
                res = handle_braces(cur_node, cur_char, raw_input, &cursor, &pos, &cur_state, true);
                if(res != LEX_OK) return res;
                break;


            case IN_SG_QUOTE:
                if(cur_char == '\''){
                    cur_node->last_seg->value[pos] = '\0';
                    if(add_segment(cur_node) != 0) return LEX_FATAL;
                    cur_node->last_seg->type = SEG_LITERAL;
                    
                    cur_state = WORD;
                    pos = 0;
                } 
                else{
                    if(pos >= MAX_WORD_LENGTH - 1) return LEX_TOO_LONG; 
                    cur_node->last_seg->value[pos] = cur_char;
                    pos ++;
                }
                cursor ++;
                break;


            case IN_DB_QUOTE:
                
                if(cur_char == '\"'){
                    cur_node->last_seg->value[pos] = '\0';
                    if(add_segment(cur_node) != 0) return LEX_FATAL;
                    cur_node->last_seg->type = SEG_LITERAL;

                    cur_state = WORD;
                    pos = 0;
                    cursor ++;
                }

                else if(cur_char == '\\'){
                    cur_state = ESCAPE_IN_DB_QUOTE;
                    cursor ++;
                } 

                else if(cur_char == '$'){
                    
                    cur_node->last_seg->value[pos] = '\0';
                    if(add_segment(cur_node) != 0) return LEX_FATAL;
                    cur_node->last_seg->type = SEG_VAR;

                    cur_state = IN_DOLLAR_IN_DB_QUOTE;
                    pos = 0;
                    cursor++;
                } 
                else {
                    cur_node->last_seg->value[pos] = cur_char;
                    cursor ++;
                    pos ++;
                }
                break;


            case ESCAPE_IN_DB_QUOTE:
                if(cur_char == '"') cur_node->last_seg->value[pos] = '"';
                else if(cur_char == '\\') cur_node->last_seg->value[pos] = '\\';
                else if(cur_char == '`') cur_node->last_seg->value[pos] = '`';
                else if(cur_char == '$') cur_node->last_seg->value[pos] = '$';
                else{
                    cur_node->last_seg->value[pos] = '\\';
                    cursor --;
                }

                cursor ++;
                pos ++;
                cur_state = IN_DB_QUOTE;
                break;


            default:    // never reached 
                assert(0);
                return LEX_UNKNOWN_ERROR;

        } // end switch
    }

    lex_exit_status_e lex_res = flush_current_token(tk_chain, cur_state, &pos);

    for(token_node_t *node = tk_chain->first; node != NULL; node = node->next){
        clean_node_segment_chain(node);
    }

    return lex_res;
}


const char *lex_exit_status_to_str(lex_exit_status_e ex_st){

    switch (ex_st)
    {
    case LEX_OK:
        return "lexing successful";

    case LEX_UNKNOWN_ERROR:
        return "unknown error";

    case LEX_UNKNOWN_OP:
        return "unexpected operator token";

    case LEX_INVALID_SUBST:
        return "bad substitution";

    case LEX_BRC_END_NOT_FOUND: 
        return "unexpected EOF while looking for }";

    case LEX_SQ_END_NOT_FOUND:
        return "unexpected EOF while looking for \'";

    case LEX_DQ_END_NOT_FOUND:
        return "unexpected EOF while looking for \"";
        
    case LEX_TOO_LONG:
        return "expression too long (>4kB)";

    case LEX_EMPTY_ESCAPE:
        return "unexpected EOF after \\";

    case LEX_FATAL:
        return "fatal error while lexing";
    }

}