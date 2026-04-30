#include "builtins.h"

#include <stdio.h>
#include "env.h"


int builtin_export(int argc, char **argv, env_t *env){
    if(!argv || !env) return 1;
    if(argc < 2) return 2;

    char *instr = argv[1];

    char name[MAX_WORD_LENGTH];
    char value[MAX_WORD_LENGTH];
    if(sscanf(instr, "%[^=]=%[^\0]", name, value) != 2) return 2;

    if(env_export(env, name, value) == 0) return 0;
    else return 1;
}