#ifndef ENV
#define ENV

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "config.h"

/**
 * @file env.h
 * @brief Environment variable chained list definition and management.
 * 
 * See also: env_internal.h
 */


/**
 * @brief Type of an environment variable chained list node.
 */
typedef struct env_var_s {
    char *name;                 /** Variable name. */
    char *value;                /** Variable value. */
    struct env_var_s *next;     /** Next node. */
} env_var_t;

/**
 * @brief Handle to an environment variable chained list, represented by its head node.  
 */
typedef env_var_t* env_t;

/**
 * @brief Frees an environment chained list. 
 */
void free_env(env_t env);

/**
 * @brief Assigns the given formatted string to variable var_name. 
 * @return 0 on success, 1 otherwise.
 * @note If the variable does not exist, it is created with the given value. 
 */
int env_export(env_t *env, char *var_name, char *var_value_fmt, ...);

/**
 * @brief Removes a variable from the environment.
 * @return 0 on success, 1 otherwise. 
 * @note A non-existent variable will leave the environment unchanged and return 0. 
 */
int env_unset(env_t *env, char *var);

/**
 * @brief Returns a variable `var` value. 
 * @return The variable value, or NULL on error. 
 * @note A non-existent variable won't be regarded as an error and  will return an empty 
 * string unless no_unset option is activated. 
 * @warning This function will copy the variable value into a fresh string and return 
 * a pointer to it. Freeing this new string is user's responsibility. 
 */
char *expand_var(env_t *env, char *var);

/**
 * @brief Frees a NULL terminated string array representing the environment.
 */
void free_env_array(char **env_arr);

/**
 * @brief Converts the environment chained list into a string array. 
 * @return On success, a formatted NULL-terminated string array representing the environment.
 * NULL otherwise. 
 * @note Array string format is "var_name=var_value".
 */
char **env_chain_to_array(env_t *env);

/**
 * @brief Converts a formatted NULL-terminated string array into an environment chained list. 
 * 
 * @param env_arr   The NULL-terminated formatted string array.
 * @param env       A pointer to the environment where variables will be added.
 * 
 * @warning The array must be NULL-terminated and each string must be formatted as follows: "var_name=var_value".
 * @note This function will not free the given array. 
 * 
 * @return 0 on success, 1 otherwise.
 */
int env_array_to_chain(char **env_arr, env_t *env);

#endif