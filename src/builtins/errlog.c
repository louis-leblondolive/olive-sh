#include "builtins.h"

#include <stdlib.h>
#include "env.h"
#include "printer.h"


int builtin_errlog(int argc, char **argv, env_t *env){
    if(!env) return 1;

    (void) argc;
    (void) argv;

    char *cmd = expand_var(env, "ERRCMD");
    char *time = expand_var(env, "ERRTIME");
    char *log = expand_var(env, "ERRLOG");

    if(!log || !time || !cmd){
        env_export(env, "ERRLOG", "errlog: an error occured loading environment error variables");
        return 1;
    }
    else{
        print_debug("errlog: loaded environment variables\n");

        if(strlen(cmd) == 0 && strlen(time) == 0) printf("No error so far\n");
        
        else{
            printf("Last error details :\n");
            printf(BOLD_ORANGE); printf("Command: "); printf(RESET); printf("%s\n", cmd);
            printf(BOLD_ORANGE); printf("Date: "); printf(RESET); printf("%s\n", time);
            printf(BOLD_ORANGE); printf("Info: "); printf(RESET); 
            if(strlen(log) > 0) printf("%s\n", log);
            else printf("No details provided\n");
        }
    }
    
    print_hint("Details can be shown automatically. Do to so, run olvsh --errlog\n");
    
    return 0;
}