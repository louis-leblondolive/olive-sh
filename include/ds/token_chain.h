#ifndef TOKEN_CHAIN
#define TOKEN_CHAIN

#include <stdlib.h>
#include <stdio.h>

#include "config.h"


/**
 * @file token_chain.h
 * @brief Token chain data structures and operations for the shell lexer.
 * 
 * Define the segment and token node/chain types, and provide functions to build,
 * manipulate and free token and segments chains produced in the lexer.
 */


//  ----- WORD SEGMENTS -----------------------------------------------
/**
 * @brief Type of a word segment, determining how its value will be interpreted.
 */
typedef enum seg_type {
    SEG_LITERAL,        /** A literal string. */
    SEG_VAR             /** A variable name, to be substituted during expansion. */
} seg_type_e;

/**
 * @brief A segment of a token's value.
 * 
 * Tokens are split into segments to distinguish litteral parts from variables (e.g. "hello$WORLD").
 */
typedef struct segment_s {
    seg_type_e       type;                
    char             value[MAX_WORD_LENGTH];
    struct segment_s *next;
} segment_t; 


//  ----- TOKENS -----------------------------------------------
/**
 * @brief Type of a shell token.
 */
typedef enum token {
    TOKEN_WORD,        /** A contiguous sequence of plain words and variables references.*/
    TOKEN_PIPE,        /** '|' */
    TOKEN_REDIR_IN,    /** '<' */
    TOKEN_REDIR_OUT,   /** '>' */
    TOKEN_HEREDOC,     /** '<<' */
    TOKEN_APPEND,      /** '>>' */
    TOKEN_AND,         /** '&&' */
    TOKEN_OR,          /** '||' */
    TOKEN_SEQ          /** ';' */
} token_e ; 

/**
 * @brief A node in the token chain, representing a single token.
 */
typedef struct token_node {
    token_e token; 

    segment_t *first_seg;
    segment_t *last_seg;

    struct token_node *prev;
    struct token_node *next;
} token_node_t ;

/**
 * @brief A token node doubly-linked list.
 */
typedef struct token_chain {
    token_node_t *first;
    token_node_t *last;
} token_chain_t ;


//  ----- SEGMENT OPERATIONS -----------------------------------------------
/**
 * @brief Adds an empty segment at the end of a node token chain.
 * @return 0 on success, -1 otherwise. 
 */
int add_segment(token_node_t *node);

/**
 * @brief Free a segment chain.
 */
void free_segment_chain(segment_t *chain);

/**
 * @brief Free all segments of a token node.
 */
void free_node_segment_chain(token_node_t *node);

/**
 * @brief Removes empty segments from a node segment chain.
 */
void clean_node_segment_chain(token_node_t *node);


//  ----- TOKEN CHAINS OPERATIONS -----------------------------------------------
/**
 * @brief Converts a token to its shell symbol.
 */
char *token_to_str(token_e token);

/**
 * @brief Initialises a token chain.
 * @return A pointer to an empty token chain (with first and last node set to NULL), 
 * or NULL on failure.
 */
token_chain_t *init_token_chain(void);

/**
 * @brief Free the given token chain and all its nodes.
 */
void free_token_chain(token_chain_t *tk_chain);

/**
 * @brief Adds an empty token node at the end of the chain.
 * @return 0 on success, -1 otherwise.
 */
int add_token_node(token_chain_t *tk_chain);

/**
 * @brief Removes and returns the first token of the chain.
 * @return A pointer to the popped node, or NULL if the chain is empty.
 */
token_node_t *token_chain_pop(token_chain_t *chain);


#endif