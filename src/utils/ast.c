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

    if(!rd_chain || !rd_chain->first) return;
    
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

    return new;
}


void free_ast(ast_node_t *root){
    if(!root) return;

    free_ast(root->left);
    free_ast(root->right);

    free_redir_chain(root->redirs);
    free_argv(root->argv);

    free(root);

    return ;
}


int add_child_left(ast_node_t *parent, ast_node_t *child){
    if(!parent || !child) return -1;

    if(parent->left != NULL){   // replace existing left branch
        free_ast(parent->left);
    }
    parent->left = child;

    return 0;
}


int add_child_right(ast_node_t *parent, ast_node_t *child){
    if(!parent || !child) return -1;

    if(parent->right != NULL){   // replace existing right branch
        free_ast(parent->right);
    }
    parent->right = child;

    return 0;
}