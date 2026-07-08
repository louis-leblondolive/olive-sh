#ifndef AST
#define AST

#include "token_chain.h"
#include "env.h"

/**
 * @file ast.h
 * @brief Abstract syntax tree structure and operations for the parser.
 * 
 * Define the abstract syntax tree structure and provide operations to build and manipulate
 * it. In particular, the redirection and argument types are defined and provided operations to 
 * build and run command nodes. 
 */

// ----- TYPES --------------------------------------------------------------------------------

// - Redirections ----- 
/** 
 * @brief Type of a redirection node.
 */
typedef struct redir_s {
    token_e        type;      /** Redirection type (i.e. '<', '>', '>>'). */
    segment_t      *target;   /** Redirection target, segmented into its litteral and variable parts. */
    struct redir_s *next;     /** Next redirection node in the chained list. */
} redir_t ;

/**
 * @brief Type of a redirection chained list.
 */
typedef struct redir_chain_s{
    redir_t *first;
    redir_t *last;
} redir_chain_t;


// - Arguments -----
/**
 * @brief Type of an argument node. 
 */
typedef struct arg_s {
    segment_t    *seg_chain;    /** The argument value, segmented into its litteral and variable parts. */
    struct arg_s *next;         /** Next argument node in the chained list. */
} arg_t;

/**
 * @brief Type of an argument chained list. 
 */
typedef struct argv_s{
    arg_t *first;
    arg_t *last;
} argv_t;


// - AST -----
/**
 * @brief Type of an AST node, that can be used either as a command or an operator node. 
 */
typedef struct ast_node_s {

    struct ast_node_s *left;    /** If non-null, the left child of the node. */
    struct ast_node_s *right;   /** If non-null, the right child of the node. */

    token_e token;              /** The node token, defining its behaviour.
                                  * If set to TOKEN_WORD, the node is to be interpreted as a command node. 
                                  * Otherwise, the node is an operator and the following parameters are to be ignored. 
                                  */
    argv_t *argv;               /** The argument chained-list of the node. */
    redir_chain_t *redirs;      /** The redirection chained-list of the node. */
    
    char *str_cmd;              /** The text command represented by the ast  */ 

} ast_node_t;


// ----- REDIR CHAINS OPERATIONS ------------------------------------------------------------
/**
 * @brief Creates an empty redirection chain. 
 * @return Returns an empty redirection chain, or NULL on error. 
 */
redir_chain_t *init_redir_chain(void);

/**
 * @brief Frees a redirection chain.
 * @note The nodes segment chains (containing redirection target) are also freed by this function. 
 */
void free_redir_chain(redir_chain_t *rd_chain);

/**
 * @brief Adds an empty redirection node to a redirection chain. 
 * @return Returns 0 on success, -1 otherwise. 
 */
int add_redir(redir_chain_t *rd_chain);


// ----- ARGUMENT CHAINS OPERATIONS ---------------------------------------------------------

/**
 * @brief Creates an empty argument chain.
 * @return Returns an empty argument chain, or NULL on error. 
 */
argv_t *init_argv(void);

/**
 * @brief Frees an argument chain. 
 * @note The nodes segment chains (containing arguments value) are also freed by this function. 
 */
void free_argv(argv_t *argv);

/**
 * @brief Adds an empty argument node to an argument chain.
 * @return Returns 0 on sucess, -1 otherwise. 
 */
int add_arg(argv_t *argv);

/**
 * @brief Frees a string array formated argument list.
 * @note Strings representing arguments value are also freed by this function.  
 */
void free_arg_array(char **arg_arr);

/**
 * @brief Returns the number of arguments contained in an argument chained list. 
 */
int count_args(argv_t *argv);

/**
 * @brief Converts an argument chained list to a string array, each string representing an 
 * argument value. 
 * @param env  A pointer to the environment 
 * @param argv A pointer to the argument chained list. 
 * @return NULL on error, the array otherwise. 
 * @note The array is terminated by a NULL sentinel, such as argv[argc] == NULL.
 */
char **arg_chain_to_array(env_t *env, argv_t *argv);

// ----- AST OPERATIONS ---------------------------------------------------------------------

ast_node_t *init_node(void);
void free_ast(ast_node_t *root);
int add_child_left(ast_node_t *parent, ast_node_t *child);
int add_child_right(ast_node_t *parent, ast_node_t *child);

#endif