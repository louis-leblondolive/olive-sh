#include "builtins.h"

#include <stdlib.h>
#include <errno.h>
#include "env.h"

int builtin_exit(int argc, char **argv, env_t *env){
    
    if(argc >= 2){
        char *end_ptr;
        errno = 0;
        long exit_code = strtol(argv[1], &end_ptr, 10);

        if(errno != 0 || end_ptr == argv[1] || *end_ptr != '\0') exit(1);

        exit(exit_code);
    }
    else {
        if(!env) exit(1);

        char *exit_code_str = expand_var(env, "?");
        if(!exit_code_str) exit(1);

        char *end_ptr;
        errno = 0;
        long exit_code = strtol(exit_code_str, &end_ptr, 10);

        if(errno != 0 || end_ptr == exit_code_str || *end_ptr != '\0') exit(1);

        exit(exit_code);
    }   

    return 0;
}