#include "builtins.h"

exec_res_t builtin_true(int argc, char **argv, env_t *env){
    (void) argc;
    (void) argv;
    (void) env;

    return exec_res_from_builtin(0);
}