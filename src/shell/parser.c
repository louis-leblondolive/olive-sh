#include "parser.h"


static void set_parse_res_error(parse_res_t *parse_res, char *fmt, ...){
    if(!parse_res || !fmt) return;
    va_list args;
    va_start(args, fmt);
    vsnprintf(parse_res->error, MAX_ERROR_LEN, fmt, args);
    va_end(args);
}


static ast_node_t *build_cmd_node(token_chain_t *tk_chain, parse_res_t *parse_res){

    if(!parse_res) return NULL;
    if(!tk_chain){
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected EOF while parsing command");
        return NULL;
    }

    print_debug("Building command node\n");

    token_node_t *cur_node = tk_chain->first;
    
    if(cur_node->token != TOKEN_WORD){
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected %s while parsing command", token_to_str(cur_node->token));
        return NULL;
    }

    ast_node_t *cmd_node = init_node();     // init command node
    if(!cmd_node){
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error while parsing");
        return NULL;
    }

    
    // Add command name
    if(add_arg(cmd_node->argv) != 0){
        free_ast(cmd_node);
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error while parsing");
        return NULL;
    }

    cmd_node->argv->last->seg_chain = cur_node->first_seg;
    cur_node->first_seg = NULL;
    cur_node->last_seg = NULL;

    cur_node = token_chain_pop(tk_chain);   // move forward in chain
    free_node_segment_chain(cur_node);                    
    free(cur_node);

    bool in_redir = false;


    // Add arguments and redirections
    while (tk_chain->first != NULL
        && tk_chain->first->token != TOKEN_SEQ && tk_chain->first->token != TOKEN_OR 
        && tk_chain->first->token != TOKEN_AND && tk_chain->first->token != TOKEN_PIPE){
        
        cur_node = tk_chain->first;

        switch(cur_node->token){

            case TOKEN_WORD:

                if(!in_redir){
                    if(add_arg(cmd_node->argv) != 0){
                    free_ast(cmd_node);
                    parse_res->success = false;
                    set_parse_res_error(parse_res, "fatal error while parsing");
                    return NULL;
                    }

                    cmd_node->argv->last->seg_chain = cur_node->first_seg;
                }

                else{
                    cmd_node->redirs->last->target = cur_node->first_seg;
                    in_redir = false;
                }

                cur_node->first_seg = NULL;
                cur_node->last_seg = NULL;

                break;
            
            default:    // Redirection
                
                if(add_redir(cmd_node->redirs) != 0){
                    free_ast(cmd_node);
                    parse_res->success = false;
                    set_parse_res_error(parse_res, "fatal error while parsing");
                    return NULL;
                }

                cmd_node->redirs->last->type = cur_node->token;
                in_redir = true;

                break;

        } // end switch

        cur_node = token_chain_pop(tk_chain);   // move forward in chain
        free_node_segment_chain(cur_node);                    
        free(cur_node);
    }

    if(in_redir){
        free_ast(cmd_node);
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected EOF near redirection");
        return NULL;
    }

    return cmd_node;
}


static ast_node_t *build_pipe_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected EOF near '|'");
        return NULL;
    }
    
    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_PIPE){

        print_debug("Building pipe node\n");

        if(cur_ast == NULL){ // case " || cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "unexpected EOF near '|'");
            return NULL;
        } 

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_cmd_node(tk_chain, parse_res);
        if(!new_right) return NULL; // parse_res error already set while parsing command node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            free_ast(new_right);
            return NULL;
        }

        res->token = TOKEN_PIPE;

        return build_pipe_node(res, tk_chain, parse_res);
    }

    else {
        if(cur_ast) return cur_ast;

        ast_node_t *res = build_cmd_node(tk_chain, parse_res);
        if(!res) return NULL;
        return build_pipe_node(res, tk_chain, parse_res);
    }

    return NULL; /* unreachable */
}


static ast_node_t *build_and_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected EOF near '&&'");
        return NULL;
    }
    
    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_AND){

        print_debug("Building and node\n");

        if(cur_ast == NULL){ // case " && cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "unexpected EOF near '&&'");
            return NULL;
        } 

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_pipe_node(NULL, tk_chain, parse_res);
        if(!new_right) return NULL; // parse_res error already set while parsing pipe node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            free_ast(new_right);
            return NULL;
        }

        res->token = TOKEN_AND;

        return build_and_node(res, tk_chain, parse_res);
    }

    else {
        if(cur_ast) return cur_ast;
        
        ast_node_t *res = build_pipe_node(cur_ast, tk_chain, parse_res);
        if(!res) return NULL;
        return build_and_node(res, tk_chain, parse_res);
    }

    return NULL; /* unreachable */
}


static ast_node_t *build_or_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected EOF near '||'");
        return NULL;
    }
    
    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_OR){

        print_debug("Building or node\n");

        if(cur_ast == NULL){ // case " || cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "unexpected EOF near '||'");
            return NULL;
        } 

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_and_node(NULL, tk_chain, parse_res);
        if(!new_right) return NULL; // parse_res error already set while parsing and node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            free_ast(new_right);
            return NULL;
        }

        res->token = TOKEN_OR;

        return build_or_node(res, tk_chain, parse_res);
    }

    else {
        if(cur_ast) return cur_ast;
        
        ast_node_t *res = build_and_node(cur_ast, tk_chain, parse_res);
        if(!res) return NULL;
        return build_or_node(res, tk_chain, parse_res);
    }

    return NULL; /* unreachable */
}


static ast_node_t *build_seq_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "unexpected EOF near ';'");
        return NULL;
    }

    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_SEQ){

        print_debug("Building seq node\n");

        if(cur_ast == NULL){ // case " ; cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "unexpected EOF near ';'");
            return NULL;
        } 

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_or_node(NULL, tk_chain, parse_res);
        if(!new_right) return NULL; // parse_res error already set while parsing or node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error while parsing");
            free_ast(res);
            free_ast(new_right);
            return NULL;
        }

        res->token = TOKEN_SEQ;

        return build_seq_node(res, tk_chain, parse_res);
    }

    else {
        if(cur_ast) return cur_ast;

        ast_node_t *res = build_or_node(cur_ast, tk_chain, parse_res);
        if(!res) return NULL;
        return build_seq_node(res, tk_chain, parse_res);
    }

    return NULL; /* unreachable */
}



parse_res_t build_ast(token_chain_t *tk_chain){

    parse_res_t parse_res;
    parse_res.ast = NULL;
    set_parse_res_error(&parse_res, "");
    parse_res.success = true;

    ast_node_t *cur_ast = build_seq_node(NULL, tk_chain, &parse_res);
    if(!cur_ast || !parse_res.success){    // error while parsing
        free_token_chain(tk_chain);
        free_ast(cur_ast);
        return parse_res;      
    }

    parse_res.ast = cur_ast;

    return parse_res;
}