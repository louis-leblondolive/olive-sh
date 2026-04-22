#ifndef LINKED_LIST
#define LINKED_LIST

#include <stdlib.h>
#include <stdio.h>

#include "config.h"


// Word segments 
typedef enum seg_type {
    SEG_LITTERAL,
    SEG_VAR
} seg_type_e;

typedef struct segment_s {
    seg_type_e type;
    char value[MAX_WORD_LENGTH];
    struct segment_s *next;
} segment_t; 


// Tokens
typedef enum token {
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_REDIR_IN,
    TOKEN_REDIR_OUT,
    TOKEN_HEREDOC,
    TOKEN_APPEND,
    TOKEN_AND, 
    TOKEN_OR, 
    TOKEN_SEQ
} token_e ; 


typedef struct token_node {
    
    token_e token; 

    segment_t *first_seg;
    segment_t *last_seg;

    struct token_node *prev;
    struct token_node *next;

} token_node_t ;


typedef struct token_chain {

    token_node_t *first;
    token_node_t *last;

} token_chain_t ;


int add_segment(token_node_t *node);
void free_node_segment_chain(token_node_t *node);
void clean_node_segment_chain(token_node_t *node);

/**
 * @brief Initialises a token chain.
 * @return An empty token chain (with first and last node set to NULL).
 */
token_chain_t *init_token_chain(void);

/**
 * @brief Free the given token chain. 
 */
void free_token_chain(token_chain_t *tk_chain);

/**
 * @brief Adds a token node at the end of the chain.
 * @return Returns 0 on success, -1 otherwise.
 */
int add_token_node(token_chain_t *tk_chain);

/**
 * @brief Consumes and returns the first token of the list.
 */
token_node_t *token_chain_pop(token_chain_t *chain);




#endif