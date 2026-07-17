#ifndef TOKEN_CHAIN
#define TOKEN_CHAIN

#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "env.h"


/**
 * @file token_chain.h
 * @brief Token chain data structures and operations for the shell lexer.
 * 
 * Defines the segment and token node/chain types, and provides functions to build,
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
 * Tokens are split into segments to distinguish literal parts from variables (e.g. "hello$WORLD").
 */
typedef struct segment_s {
    seg_type_e       type;      /** Kind of segment (SEG_LITERAL or SEG_VAR). */
    char             value[MAX_WORD_LENGTH];    /** Segment raw text (literal value or variable name). */
    struct segment_s *next;     /** Next segment in the chain. */
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
    TOKEN_SEQ,         /** ';' */
    TOKEN_AMP          /** '&' */
} token_e ; 

/**
 * @brief A node in the token chain, representing a single token.
 */
typedef struct token_node {
    token_e token;    /** Node token, either WORD or operator. */

    segment_t *first_seg;       /** First segment of the node token chain. */
    segment_t *last_seg;        /** Last segment of the node token chain. */

    struct token_node *prev;    /** Previous token in the chain. */
    struct token_node *next;    /** Next token in the chain. */
} token_node_t ;

/**
 * @brief A token node doubly-linked list.
 */
typedef struct token_chain {
    token_node_t *first;    /** First node of the chain. */
    token_node_t *last;     /** Last node of the chain. */
} token_chain_t ;


//  ----- SEGMENT OPERATIONS -----------------------------------------------
/**
 * @brief Adds an empty segment at the end of a node token chain.
 * @return 0 on success, -1 otherwise. 
 */
int add_segment(token_node_t *node);

/**
 * @brief Frees a segment chain.
 */
void free_segment_chain(segment_t *chain);

/**
 * @brief Frees all segments of a token node.
 */
void free_node_segment_chain(token_node_t *node);

/**
 * @brief Removes empty segments from a node segment chain.
 */
void clean_node_segment_chain(token_node_t *node);

/**
 * @brief Expands variables into their literal value and concatenates segments into a string.
 * @return NULL on error, the resulting string otherwise.
 */
char *expand_segment_chain(env_t *env, segment_t *chain);

/** 
 * @brief Converts a segment chain to the literal string it represents. 
 */
char *segment_chain_to_str(segment_t *chain);


//  ----- TOKEN CHAINS OPERATIONS -----------------------------------------------
/**
 * @brief Converts a token to its shell symbol.
 */
char *token_to_str(token_e token);

/**
 * @brief Tests if a token is ; or &.
 */
bool is_delim(token_e token);

/**
 * @brief Initialises a token chain.
 * @return A pointer to an empty token chain (with first and last node set to NULL), 
 * or NULL on failure.
 */
token_chain_t *init_token_chain(void);

/**
 * @brief Frees the given token chain and all its nodes.
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