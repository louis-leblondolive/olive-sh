#include "parser_internal.h"

void set_parse_res_error(parse_res_t *parse_res, char *fmt, ...){
    if(!parse_res || !fmt) return;

    if(parse_res->error) free(parse_res->error);

    va_list args;
    va_start(args, fmt);
    vasprintf(&(parse_res->error), fmt, args);
    va_end(args);
}


void set_parse_res_error_info(parse_res_t *parse_res, char *fmt, ...){
    if(!parse_res || !fmt) return;

    if(parse_res->error_info) free(parse_res->error_info);

    va_list args;
    va_start(args, fmt);
    vasprintf(&(parse_res->error_info), fmt, args);
    va_end(args);
}


ast_node_t *build_cmd_node(token_chain_t *tk_chain, parse_res_t *parse_res){

    if(!parse_res) return NULL;
    if(!tk_chain){
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "No token chain was provided to the parser.");
        return NULL;
    }

    if(!tk_chain->first) return NULL;   // empty chain 

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
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
        return NULL;
    }

    
    // Add command name
    if(add_arg(cmd_node->argv) != 0){
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "System failed to allocate argument node");

        free_ast(cmd_node);
        parse_res->success = false;
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
        && tk_chain->first->token != TOKEN_SEQ && tk_chain->first->token != TOKEN_AMP
        && tk_chain->first->token != TOKEN_OR && tk_chain->first->token != TOKEN_AND 
        && tk_chain->first->token != TOKEN_PIPE){
        
        cur_node = tk_chain->first;

        switch(cur_node->token){

            case TOKEN_WORD:

                if(!in_redir){
                    if(add_arg(cmd_node->argv) != 0){
                    free_ast(cmd_node);
                    parse_res->success = false;
                    set_parse_res_error(parse_res, "fatal error");
                    set_parse_res_error_info(parse_res, "System failed to allocate argument node.");
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
                    set_parse_res_error(parse_res, "fatal error");
                    set_parse_res_error_info(parse_res, "System failed to allocate redirection node.");
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


ast_node_t *build_pipe_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "No token chain was provided to the parser.");
        return NULL;
    }
    
    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_PIPE){

        print_debug("Building pipe node\n");

        if(cur_ast == NULL){ // case " || cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "parse error near '|'");
            set_parse_res_error_info(parse_res, "'|' source is missing.");
            return NULL;
        } 

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_cmd_node(tk_chain, parse_res);
        if(!parse_res->success) return NULL; // parse_res error already set while parsing command node 

        if(new_right == NULL){
            parse_res->success = false;
            set_parse_res_error(parse_res, "parse error near '|'");
            set_parse_res_error_info(parse_res, "'|' target is missing.");
            free_ast(res);
            free_ast(new_right);
            return NULL;
        }

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
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


ast_node_t *build_and_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "No token chain was provided to the parser.");
        return NULL;
    }
    
    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_AND){

        print_debug("Building and node\n");

        if(cur_ast == NULL){ // case " && cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "parse error near '&&'");
            set_parse_res_error_info(parse_res, "'&&' first command is missing.");
            return NULL;
        } 

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_pipe_node(NULL, tk_chain, parse_res);
        if(!new_right) return NULL; // parse_res error already set while parsing pipe node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
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


ast_node_t *build_or_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "No token chain was provided to the parser.");
        return NULL;
    }
    
    if(tk_chain->first != NULL && tk_chain->first->token == TOKEN_OR){

        print_debug("Building or node\n");

        if(cur_ast == NULL){ // case " || cmd ... "
            parse_res->success = false;
            set_parse_res_error(parse_res, "parse error near '||'");
            set_parse_res_error_info(parse_res, "'||' first command is missing.");
            return NULL;
        }

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_and_node(NULL, tk_chain, parse_res);
        if(!new_right) return NULL; // parse_res error already set while parsing and node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
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


ast_node_t *build_delim_node(ast_node_t *cur_ast, token_chain_t *tk_chain, parse_res_t *parse_res){
    if(!parse_res) return NULL;
    if(!tk_chain){    // should never be reached
        parse_res->success = false;
        set_parse_res_error(parse_res, "fatal error");
        set_parse_res_error_info(parse_res, "No token chain was provided to the parser.");
        return NULL;
    }

    if(tk_chain->first != NULL && is_delim(tk_chain->first->token)){

        print_debug("Building delim node\n");

        token_e delim_tk = tk_chain->first->token;

        token_node_t *cur_node = token_chain_pop(tk_chain); 
        free_node_segment_chain(cur_node);                    // move forward in chain
        free(cur_node);
        
        ast_node_t *res = init_node();

        if(!res || add_child_left(res, cur_ast) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
            free_ast(res);
            return NULL;
        }

        ast_node_t *new_right = build_or_node(NULL, tk_chain, parse_res);
        if(!parse_res->success) return NULL; // parse_res error already set while parsing or node 

        if(add_child_right(res, new_right) != 0){
            parse_res->success = false;
            set_parse_res_error(parse_res, "fatal error");
            set_parse_res_error_info(parse_res, "System failed to allocate AST node.");
            free_ast(res);
            free_ast(new_right);
            return NULL;
        }

        res->token = delim_tk;

        return build_delim_node(res, tk_chain, parse_res);
    }

    else {
        if(cur_ast) return cur_ast;

        ast_node_t *res = build_or_node(cur_ast, tk_chain, parse_res);
        if(!res) return NULL;
        return build_delim_node(res, tk_chain, parse_res);
    }

    return NULL; /* unreachable */
}


int build_ast_text_cmd(ast_node_t *root){
    if(!root) return 0;
    free(root->str_cmd);

    if(root->token == TOKEN_WORD){

        // Convert args and redirs to strings 
        size_t argc = (size_t)count_args(root->argv);
        size_t redir_cnt = 0;
        for (redir_t *red = root->redirs->first; red != NULL; red = red->next) redir_cnt ++;
        
        char *raw_args[argc > 0 ? argc : 1];
        size_t i = 0;

        for (arg_t *arg = root->argv->first; arg != NULL; arg = arg->next){
            raw_args[i] = segment_chain_to_str(arg->seg_chain);
            i++;
        }

        char *raw_redirs[redir_cnt > 0 ? redir_cnt : 1];
        const char *redir_syms[redir_cnt > 0 ? redir_cnt : 1];
        i = 0;
        for (redir_t *red = root->redirs->first; red != NULL; red = red->next){
            raw_redirs[i] = segment_chain_to_str(red->target);
            redir_syms[i] = token_to_str(red->type);
            i++;
        }
        
        // Init res string 
        size_t len = 0;
        for (size_t j = 0; j < argc; j++){
            len += strlen(raw_args[j]) + 1;
        } 
        for (size_t j = 0; j < redir_cnt; j++){
            len += strlen(raw_redirs[j]) + 1;
            len += strlen(redir_syms[j]) + 1;
        }
        len += 1; // counting final '\0'

        char *res = (char*)malloc(sizeof(char) * len);
        if(!res){
            for (size_t j = 0; j < argc; j++) free(raw_args[j]);
            for (size_t j = 0; j < redir_cnt; j++) free(raw_redirs[j]);
            return 1;
        }

        // Copy raw args and redirs 
        size_t pos = 0;
        for (size_t j = 0; j < argc; j++) {

            size_t loc_len = strlen(raw_args[j]);
            memcpy(res + pos, raw_args[j], loc_len);
            pos += loc_len;

            if(j < argc - 1 || redir_cnt > 0){
                res[pos] = ' ';
                pos++;
            }

            free(raw_args[j]); 
        }

        for (size_t j = 0; j < redir_cnt; j++) {

            size_t sym_len = strlen(redir_syms[j]);
            memcpy(res + pos, redir_syms[j], sym_len);
            pos += sym_len;

            res[pos] = ' '; 
            pos++;

            size_t loc_len = strlen(raw_redirs[j]);
            memcpy(res + pos, raw_redirs[j], loc_len);
            pos += loc_len;

            if(j < redir_cnt - 1){
                res[pos] = ' ';
                pos++;
            }
            free(raw_redirs[j]); 
        }

        res[pos] = '\0';
        root->str_cmd = res;
        return 0;
    }

    else {

        if(build_ast_text_cmd(root->left) != 0 || build_ast_text_cmd(root->right) != 0){
            return 1;
        }

        char *sym = token_to_str(root->token);

        size_t sym_len = strlen(sym);
        size_t left_len = root->left ? strlen(root->left->str_cmd) : 0;
        size_t right_len = root->right ? strlen(root->right->str_cmd) : 0;
        
        size_t len = sym_len + 1;

        if(root->left) len += left_len + 1;
        if(root->right) len += right_len + 1;

        char *res = (char*)malloc(sizeof(char) * len);
        if(!res) return 1;

        size_t pos = 0;
        if(root->left){
            memcpy(res, root->left->str_cmd, left_len);
            pos += left_len;
            res[pos] = ' '; pos++;
        }

        memcpy(res + pos, sym, sym_len);
        pos += sym_len;

        if(root->right){
            res[pos] = ' '; pos++;
            memcpy(res + pos, root->right->str_cmd, right_len);
            pos += right_len;
        }

        res[pos] = '\0';
        root->str_cmd = res;
        return 0;
    }

    return 1;   // unreachable
}