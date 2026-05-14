#include "builtins.h"

#include <stdio.h>
#include <stdbool.h>
#include "env.h"


int builtin_echo(int argc, char **argv, env_t *env){

    if(!argv) return 1;
    (void) env; // silences unused errors

    bool option_n = argc >= 2 && strcmp(argv[1], "-n") == 0;

    int i = 1;
    if(option_n) i = 2;

    while (i < argc){
        printf("%s ", argv[i]);
        i ++;
    }
    
    if(!option_n) printf("\n");

    return 0;
}