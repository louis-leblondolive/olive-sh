#ifndef ENV_INTERNAL
#define ENV_INTERNAL

#include <stdlib.h>
#include <string.h>

/**
 * @file env_internal.h
 * @brief Internal operations used in environment management.
 * 
 * See also: env.h
 */

/**
 * @brief Allocates a fresh environment variable node with the given name and value.
 * @return A pointer to the variable, or NULL on error.
 * @warning The variable name and value fields will be duplicated from the given strings. 
 */
env_var_t *init_env_var(char *name, char *value);

/**
 * @brief Frees the given variable. 
 */
void free_env_var(env_var_t *var);

/**
 * @brief Counts the number of variables contained in the given environment. 
 */
size_t env_var_count(env_t *env);

/** 
 * @brief Converts an environment variable into a formatted "var_name=var_value" string. 
 * @return The formatted string, or NULL on error. 
 */
char *format_env_var(env_var_t *var);

#endif