#include "shell_loop.h"
#include "config.h"

// initialize configuration informations
config_infos_t cfg_infos;


int main(int argc, char *argv[]){
    
    // verbosity parameters 
    cfg_infos.quiet = false;
    cfg_infos.verbose = false;

    for (int i = 0; i < argc; i++){
        if(strcmp(argv[i], "-v") == 0) cfg_infos.verbose = true;
        if(strcmp(argv[i], "-q") == 0) cfg_infos.quiet = true;
    }
    if(cfg_infos.quiet && cfg_infos.verbose){
        cfg_infos.quiet = false;
        cfg_infos.verbose = false;
    }

    int run_res = run_shell();

    return run_res;
}