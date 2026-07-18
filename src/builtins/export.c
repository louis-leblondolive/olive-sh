#include "builtins.h"

#include <stdio.h>
#include "env.h"


exec_res_t builtin_export(int argc, char **argv, env_t *env){
    if(!argv || !env) return exec_res_from_builtin(1);
    if(argc < 2) return exec_res_from_builtin(2);

    char *instr = argv[1];

    char name[MAX_WORD_LENGTH];
    char value[MAX_WORD_LENGTH];
    if(sscanf(instr, "%[^=]=%s", name, value) != 2) return exec_res_from_builtin(2);

    if(env_export(env, name, "%s", value) == 0) return exec_res_from_builtin(0);
    else return exec_res_from_builtin(1);
}