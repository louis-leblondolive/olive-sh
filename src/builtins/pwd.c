#include "builtins.h"

#include <stdio.h>
#include <unistd.h>
#include "env.h"


exec_res_t builtin_pwd(int argc, char **argv, env_t *env){

    (void)argc;
    (void)argv;
    (void)env;

    char *cwd = getcwd(NULL, 0);
    printf("%s\n", cwd);
    free(cwd);

    return exec_res_from_builtin(0);   
}