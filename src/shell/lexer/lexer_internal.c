#include "lexer_internal.h"


lex_exit_status_e handle_dollar(token_node_t *cur_node, char cur_char, 
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


lex_exit_status_e handle_braces(token_node_t *cur_node, char cur_char, char *raw_input,
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


lex_exit_status_e flush_current_token(token_chain_t *tk_chain, lexer_state_e cur_state, size_t *pos){

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