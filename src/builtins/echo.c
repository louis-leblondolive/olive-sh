#include "builtins.h"

#include <stdio.h>
#include "env.h"


int builtin_echo(int argc, char **argv, env_t *env){

    if(!argv) return 1;
    (void) env; // silences unused errors

    for (int i = 1; i < argc; i++){
        printf("%s\n", argv[i]);
    }
    
    return 0;
}