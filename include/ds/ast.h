#ifndef AST
#define AST

#include "token_chain.h"

typedef struct redir_s {
    token_e        type;
    segment_t      *target;
    struct redir_s *next;
} redir_t ;

typedef struct redir_chain_s{
    redir_t *first;
    redir_t *last;
} redir_chain_t;


typedef struct arg_s {
    segment_t    *seg_chain;
    struct arg_s *next;
} arg_t;


typedef struct argv_s{
    arg_t *first;
    arg_t *last;
} argv_t;



typedef struct ast_node_s {

    struct ast_node_s *left;
    struct ast_node_s *right;

    token_e token;
    argv_t *argv;
    redir_chain_t *redirs;
    
} ast_node_t;


// ----- REDIR CHAINS OPERATIONS ---------------------------------

redir_chain_t *init_redir_chain(void);
void free_redir_chain(redir_chain_t *rd_chain);
int add_redir(redir_chain_t *rd_chain);


// ----- ARGUMENT CHAINS OPERATIONS ------------------------------

argv_t *init_argv(void);
void free_argv(argv_t *argv);
int add_arg(argv_t *argv);


// ----- AST OPERATIONS ------------------------------------------

ast_node_t *init_node(void);
void free_ast(ast_node_t *root);
int add_child_left(ast_node_t *parent, ast_node_t *child);
int add_child_right(ast_node_t *parent, ast_node_t *child);

#endif