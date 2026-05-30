#include "builtins.h"

#include <stdlib.h>
#include "env.h"
#include "printer.h"

int builtin_errlog(int argc, char **argv, env_t *env){
    if(!env) return 1;

    (void) argc;
    (void) argv;

    char *log = expand_var(env, "ERRLOG");
    if(!log || strlen(log) == 0){
        printf("No details attached to last error.\n");
    }
    else{
        printf("Last error details :\n %s\n", log);
    }
    
    print_hint("Details can be shown automatically. Do to so, run olvsh --errlog\n");
    printf("\n");
    
    return 0;
}