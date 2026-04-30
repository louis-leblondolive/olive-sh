#include "exec_internal.h"


int run_cmd(env_t *env, ast_node_t *cmd_node){
    if(!cmd_node) return 0;

    print_debug("Running command\n");
        
    int argc = count_args(cmd_node->argv);

    print_debug("argc setup\n");

    char **argv = arg_chain_to_array(env, cmd_node->argv);
    if(!argv) return 1;

    print_debug("argv setup\n");

    char *cmd_name = argv[0];

    print_debug("name setup\n");

    // Setup redirs here

    for (size_t i = 0; builtins[i].name != NULL; i++){
        if(strcmp(cmd_name, builtins[i].name) == 0){
            int res = builtins[i].func(argc, argv, env);
            free_arg_array(argv);
            return res;
        } 
    }
    
    print_error("command not found: '%s'\n", cmd_name);

    return 0;

}