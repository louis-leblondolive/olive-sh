#include "lexer.h"
#include "lexer_rules.h"
#include "lexer_internal.h"


lex_exit_status_e build_token_list(char *raw_input, size_t input_len, lexer_res_t *lex_res){

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
        cur_node = lex_res->tk_chain->last;

        switch(cur_state){

            case START:

                if(cur_char == ' '){ cursor ++; break; }

                pos = 0;
                if(add_token_node(lex_res->tk_chain) != 0){
                    set_lex_res_error_info(lex_res, "System failed to allocate token node.");
                    return LEX_FATAL;
                }
                if(add_segment(lex_res->tk_chain->last) != 0){
                    set_lex_res_error_info(lex_res, "System failed to allocate segment node.");
                    return LEX_FATAL;
                }

                if(is_operator(cur_char)){      // next token is an operator
                    cur_state = OPERATOR;
                }
                else if (cur_char == '$'){      // next token is a variable
                    cur_state = IN_DOLLAR;
                    lex_res->tk_chain->last->last_seg->type = SEG_VAR;
                    cursor ++;
                } 
                else {                          // next token is a word
                    cur_state = WORD;
                    lex_res->tk_chain->last->last_seg->type = SEG_LITERAL;
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
                    if(add_segment(cur_node) != 0){
                        set_lex_res_error_info(lex_res, "System failed to allocate segment node.");
                        return LEX_FATAL;
                    } 
                    cur_node->last_seg->type = SEG_LITERAL;
                    
                    cur_state = IN_SG_QUOTE;
                    pos = 0;
                    cursor ++;
                }

                else if(cur_char == '"'){
                    if(pos > 0){
                        cur_node->last_seg->value[pos] = '\0';

                        if(add_segment(cur_node) != 0){
                            set_lex_res_error_info(lex_res, "System failed to allocate segment node.");
                            return LEX_FATAL;
                        }
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

                    if(add_segment(cur_node) != 0){
                        set_lex_res_error_info(lex_res, "System failed to allocate segment node.");
                        return LEX_FATAL;
                    } 
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

                if(op_tk == TOKEN_WORD){
                    lex_res->error_pos = cursor;
                    return LEX_UNKNOWN_OP;
                }
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
                res = handle_dollar(lex_res, cur_node, cur_char, &cursor, &pos, &cur_state, false);
                if(res != LEX_OK) return res;
                break;


            case IN_DOLLAR_IN_DB_QUOTE:
                res = handle_dollar(lex_res, cur_node, cur_char, &cursor, &pos, &cur_state, true);
                if(res != LEX_OK) return res;
                break;


            case IN_BRACES:
                res = handle_braces(lex_res, cur_node, cur_char, raw_input, &cursor, &pos, &cur_state, false);
                if(res != LEX_OK) return res;
                break;

            
            case IN_BRACES_IN_DB_QUOTE:
                res = handle_braces(lex_res, cur_node, cur_char, raw_input, &cursor, &pos, &cur_state, true);
                if(res != LEX_OK) return res;
                break;


            case IN_SG_QUOTE:
                if(cur_char == '\''){
                    cur_node->last_seg->value[pos] = '\0';
                    if(add_segment(cur_node) != 0){
                        set_lex_res_error_info(lex_res, "System failed to allocate segment node");
                        return LEX_FATAL;
                    }
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
                    if(add_segment(cur_node) != 0){
                        set_lex_res_error_info(lex_res, "System failed to allocate segment.");
                        return LEX_FATAL;
                    } 
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
                    if(add_segment(cur_node) != 0){
                        set_lex_res_error_info(lex_res, "System failed to allocate segment node.");
                        return LEX_FATAL;
                    }
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

    lex_exit_status_e lex_flush = flush_current_token(lex_res, cur_state, &pos);

    for(token_node_t *node = lex_res->tk_chain->first; node != NULL; node = node->next){
        clean_node_segment_chain(node);
    }

    return lex_flush;
}


lexer_res_t lex_input(char *raw_input, size_t input_len){

    lexer_res_t lex_res;
    lex_res.success = true;
    lex_res.tk_chain = init_token_chain();
    lex_res.error = NULL;
    lex_res.error_info = NULL;
    lex_res.error_pos = 0;

    lex_exit_status_e exit_st = build_token_list(raw_input, input_len, &lex_res);
    if(exit_st != LEX_OK) lex_res.success = false;

    // Assign error descriptions 
    switch (exit_st){
        
        case LEX_UNKNOWN_ERROR:
            set_lex_res_error(&lex_res, "unknown error");
            break;

        case LEX_UNKNOWN_OP:
            set_lex_res_error(&lex_res, "unknown operator");
            break;

        case LEX_INVALID_SUBST:
            set_lex_res_error(&lex_res, "bad substitution");
            break;

        case LEX_BRC_END_NOT_FOUND: 
            set_lex_res_error(&lex_res, "unexpected EOF while looking for }.");
            set_lex_res_error_info(&lex_res, "} not found. Did you forget a closing brace ?");
            break;

        case LEX_SQ_END_NOT_FOUND:
            set_lex_res_error(&lex_res, "unexpected EOF while looking for \'");
            set_lex_res_error_info(&lex_res, "\' not found. Did you forget a closing quote ?");
            break;

        case LEX_DQ_END_NOT_FOUND:
            set_lex_res_error(&lex_res, "unexpected EOF while looking for \"");
            set_lex_res_error_info(&lex_res, "\" not found. Did you forget a closing quote ?");
            break;
            
        case LEX_TOO_LONG:
            set_lex_res_error(&lex_res, "expression too long");
            set_lex_res_error_info(&lex_res, "Expression size exceeded buffer limit (4kB).");
            break;

        case LEX_EMPTY_ESCAPE:
            set_lex_res_error(&lex_res, "unexpected EOF after \\");
            break;

        case LEX_FATAL:
            set_lex_res_error(&lex_res, "fatal error");
            break;
        
        case LEX_OK:
            break;
    }

    return lex_res;
}