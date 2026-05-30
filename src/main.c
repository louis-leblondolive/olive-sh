#include "shell_loop.h"
#include "config.h"

// initialize configuration informations
config_infos_t cfg_infos;


int main(int argc, char *argv[], char *envp[]){
    
    (void) argc;
    (void) argv;

    // verbosity parameters 
    cfg_infos.debug = false;
    cfg_infos.hints = true;
    cfg_infos.errlog = false;

    int run_res = run_shell(envp);

    return run_res;
}