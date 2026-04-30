#include "builtins.h"

#include "env.h"

int builtin_unset(int argc, char **argv, env_t *env){
    if(!argv || !env) return 1;
    if(argc < 2) return 2;

    if(env_unset(env, argv[1]) == 0) return 0;
    else return 1;
}