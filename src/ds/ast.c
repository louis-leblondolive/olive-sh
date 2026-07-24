#include "ast.h"

// ----- REDIR CHAINS OPERATIONS ---------------------------------
redir_chain_t *init_redir_chain(void){

    redir_chain_t *new = (redir_chain_t*)malloc(sizeof(redir_chain_t));
    if(!new) return NULL;

    new->first = NULL;
    new->last = NULL;

    return new;
}


void free_redir_chain(redir_chain_t *rd_chain){

    if(!rd_chain) return;
    
    redir_t *redir = rd_chain->first;
    while(redir != NULL){
        redir_t *cache = redir->next;
        free_segment_chain(redir->target);
        free(redir);
        redir = cache;
    }
    free(rd_chain);
    return;
}


int add_redir(redir_chain_t *rd_chain){

    if(!rd_chain) return -1;

    redir_t *new = (redir_t*)malloc(sizeof(redir_t));
    if(!new) return -1;

    if(!rd_chain->first){   // adding redir to an empty chain
        rd_chain->first = new;
        rd_chain->last = new;
    }
    else{
        if(!rd_chain->last) return -1;  // should never happen 
        rd_chain->last->next = new;
        rd_chain->last = new;
    }

    new->target = NULL;
    new->next = NULL;
    new->type = TOKEN_WORD;

    return 0;
}


// ----- ARGUMENT CHAINS OPERATIONS ------------------------------
argv_t *init_argv(void){
    argv_t *new = (argv_t*)malloc(sizeof(argv_t));
    if(!new) return NULL;
    
    new->first = NULL;
    new->last = NULL;

    return new;
}


void free_argv(argv_t *argv){

    if(!argv) return;

    arg_t *arg = argv->first;
    while(arg != NULL){
        arg_t *cache = arg->next;
        free_segment_chain(arg->seg_chain);
        free(arg);
        arg = cache;
    }
    free(argv);
    return;
}


int add_arg(argv_t *argv){

    if(!argv) return -1;

    arg_t *new = (arg_t*)malloc(sizeof(arg_t));
    if(!new) return -1;

    if(!argv->first){   // adding arg to an empty chain
        argv->first = new;
        argv->last = new;
    }
    else{
        if(!argv->last) return -1;  // should never happen
        argv->last->next = new;
        argv->last = new;
    }

    new->next = NULL;
    new->seg_chain = NULL;

    return 0;
}


void free_arg_array(char **arg_arr){

    size_t i = 0;
    while(arg_arr[i] != NULL){
        free(arg_arr[i]);
        i ++;
    }
    free(arg_arr);
}


int count_args(argv_t *argv){
    int argc = 0;
    arg_t *cur_arg = argv->first;
    while(cur_arg != NULL){
        argc ++;
        cur_arg = cur_arg->next;
    }

    return argc;
}


char **arg_chain_to_array(env_t *env, argv_t *argv){
    if(!argv) return NULL;

    int argc = count_args(argv);

    // Build result 
    char **res = (char**)malloc(sizeof(char*) * (argc + 1));
    if(!res) return NULL;

    res[argc] = NULL;   // sentinel

    arg_t *cur_arg = argv->first;
    for (int i = 0; i < argc; i++){

        res[i] = expand_segment_chain(env, cur_arg->seg_chain);
        if(!res[i]) return NULL;
        cur_arg = cur_arg->next;
    }
    
    return res;
}


// ----- AST OPERATIONS ------------------------------------------
ast_node_t *init_node(void){
    ast_node_t *new = (ast_node_t*)malloc(sizeof(ast_node_t));
    if(!new) return NULL;

    argv_t *argv =  init_argv();
    redir_chain_t *redirs = init_redir_chain();
    if(!argv || !redirs){
        free_argv(argv);
        free_redir_chain(redirs);
        free(new);
        return NULL;
    }

    new->left = NULL;
    new->right = NULL;
    new->argv = argv;
    new->redirs = redirs;
    new->token = TOKEN_WORD;
    new->str_cmd = NULL;

    return new;
}


void free_ast(ast_node_t *root){
    if(!root) return;

    free_ast(root->left);
    free_ast(root->right);

    free_redir_chain(root->redirs);
    free_argv(root->argv);

    free(root->str_cmd);

    free(root);

    return ;
}


int add_child_left(ast_node_t *parent, ast_node_t *child){
    if(!parent) return -1;

    if(parent->left != NULL){   // replace existing left branch
        free_ast(parent->left);
    }
    parent->left = child;

    return 0;
}


int add_child_right(ast_node_t *parent, ast_node_t *child){
    if(!parent) return -1;

    if(parent->right != NULL){   // replace existing right branch
        free_ast(parent->right);
    }
    parent->right = child;

    return 0;
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