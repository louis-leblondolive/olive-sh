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
    if(!argv){
        env_export(env, "ERRLOG", "couldn't resolve argument chain");
        return 1;
    }
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
    char *cmd_path = NULL; 

    
    if(strchr(cmd_name, '/') != NULL){      // Run from given path 

        // Resolving path
        cmd_path = realpath(cmd_name, NULL);
        if(!cmd_path){
            char buff[strlen(cmd_name) + 29];
            snprintf(buff, strlen(cmd_name) + 30, "file or directory not found: %s", cmd_name);
            env_export(env, "ERRLOG", buff);
            return 1;
        }

        // Permission test 
        struct stat buf;
        if(stat(cmd_path, &buf) != 0){
            env_export(env, "ERRLOG", "couldn't read file stats");
            return 1;
        }
        if(!S_ISREG(buf.st_mode)){
            env_export(env, "ERRLOG", "not a regular file");
            return 1;
        }
        if(access(cmd_path, X_OK) != 0){
            env_export(env, "ERRLOG", "permission denied");
            return 1;
        }
    }
    else{    // Run a command 

        cmd_path = find_cmd_path(env, cmd_name);
        if(!cmd_path){
            char buff[strlen(cmd_name) + 19];
            snprintf(buff, strlen(cmd_name) + 20, "command not found: %s", cmd_name);
            env_export(env, "ERRLOG", buff);
            print_debug("Command not found\n");
            return 1;
        }
    }

    print_debug("Found command %s at %s\n", cmd_name, cmd_path);

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
            waitpid(pid, &res, 0);

            free(cmd_path);
            free(envp);

            if(WIFSIGNALED(res))
                return 128 + WTERMSIG(res);
            return WEXITSTATUS(res);
    }

    return 0; /* unreachable */
}