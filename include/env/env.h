#ifndef ENV
#define ENV

#include <stdlib.h>
#include <string.h>

#include "config.h"

typedef struct env_var_s {
    char *name;
    char *value;
    struct env_var_s *next;
} env_var_t ;


typedef env_var_t *env_t;

void free_env(env_t env);
int env_export(env_t *env, char *var_name, char *var_value);
int env_unset(env_t *env, char *var);
char *expand_var(env_t *env, char *var);
void free_env_array(char **env_arr);
char **env_chain_to_array(env_t *env);

#endif