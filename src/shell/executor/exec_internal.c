#include "exec_internal.h"


char *find_cmd_path(env_t *env, char *cmd){

    char *path = expand_var(env, "PATH");
    if(!path) return NULL;

    char *cmd_path = NULL;
    char *path_dir;

    for (path_dir = strtok(path, ":");  path_dir != NULL; path_dir = strtok(NULL, ":")){
        
        char full_path[strlen(path_dir) + 1 + strlen(cmd) + 1];
        snprintf(full_path, sizeof(full_path), "%s/%s", path_dir, cmd);

        print_debug("Searching for %s at %s\n", cmd, full_path);

        if(access(full_path, F_OK | X_OK) == 0){

            cmd_path = strdup(full_path);
            break;
        }
    }

    free(path);
    
    return cmd_path;
}



int run_cmd(env_t *env, ast_node_t *cmd_node){
    if(!env || !cmd_node) return 0;

    print_debug("Running command\n");
    
    // ------ SET PARAMETERS UP -------------------------------------
    int argc = count_args(cmd_node->argv);
    print_debug("argc setup\n");

    char **argv = arg_chain_to_array(env, cmd_node->argv);
    if(!argv) return 1;
    print_debug("argv setup\n");

    char *cmd_name = argv[0];
    print_debug("name setup\n");

    // Setup redirs here


    // ------ RUN BUILTINS -----------------------------------------
    for (size_t i = 0; builtins[i].name != NULL; i++){
        if(strcmp(cmd_name, builtins[i].name) == 0){
            int res = builtins[i].func(argc, argv, env);
            free_arg_array(argv);
            return res;
        } 
    }
    

    // ----- RUN EXTERNALS ------------------------------------------
    char *cmd_path = find_cmd_path(env, cmd_name);
    if(!cmd_path){
        print_error("command not found: '%s'\n", cmd_name);
        return 1;
    }


    print_debug("found command %s at %s\n", cmd_name, cmd_path);

    char **envp = env_chain_to_array(env);
    if(!envp) return 1;

    pid_t pid = fork();
    int res;
    switch (pid){

        case -1:
            perror("fork");
            free(cmd_path);
            free_env_array(envp);
            return 1;
        
        // Child process
        case 0:     
            if(reset_sa_handlers() != 0){
                perror("sigaction");
                exit(1);
            }

            execve(cmd_path, argv, envp);
            
            perror("execve");
            exit(1);

        // Parent process
        default:
            wait(&res);

            free(cmd_path);
            free(envp);

            return WEXITSTATUS(res);
    }

    return 0; /* unreachable */
}