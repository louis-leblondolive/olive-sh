#include "builtins.h"

#include "env.h"

exec_res_t builtin_unset(int argc, char **argv, env_t *env){
    if(!argv || !env) return exec_res_from_builtin(1);
    if(argc < 2) return exec_res_from_builtin(2);

    if(env_unset(env, argv[1]) == 0) return exec_res_from_builtin(0);
    else return exec_res_from_builtin(1);
}