#ifndef ENV_INTERNAL
#define ENV_INTERNAL

#include <stdlib.h>
#include <string.h>

env_t init_env_var(char *name, char *value);
void free_env_var(env_t var);
size_t env_var_count(env_t *env);
char *format_env_var(env_t var);

#endif