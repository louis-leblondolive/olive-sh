#include "builtins.h"

#include <stdio.h>
#include "env.h"


int builtin_env(int argc, char **argv, env_t *env){
    if(!env) return 1;

    (void)argc;
    (void)argv; // silences unused warning

    env_t cur_var = *env; 
    while(cur_var != NULL){
        printf("%s=%s\n", cur_var->name, cur_var->value);
        cur_var = cur_var->next;
    }
    
    return 0;
}