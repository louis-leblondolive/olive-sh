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


pid_t run_cmd_async(env_t *env, ast_node_t *cmd_node, int fd_in, int fd_out, int fd_err){

    print_debug("Running command asynchronously\n");

    pid_t pid = fork();

    switch (pid){

        case -1:
            perror("fork");
            return -1;
        
        // Child process
        case 0:     
            if(reset_sa_handlers() != 0){
                perror("sigaction");
                exit(1);
            }

            // --- SETUP REDIRS AND I/O ----------------------------------------------
            for(redir_t *red = cmd_node->redirs->first; red != NULL; red = red->next){

                char *red_target = expand_segment_chain(env, red->target);
                if(!red_target){
                    char *unbound_info = expand_var(env, "ERRLOG");
                    dprintf(fd_err, "olive-sh: failed to resolve redirection target (%s)\n", unbound_info);
                    exit(1);
                }

                if(red->type == TOKEN_REDIR_IN){
                    if(fd_in != STDOUT_FILENO) close(fd_in);
                    fd_in = open(red_target, O_RDONLY, 0644);
                    if(fd_in < 0){
                        dprintf(fd_err, "olive-sh: < %s: failed to open redirection target", red_target);
                        exit(1);
                    }
                }
                else if(red->type == TOKEN_REDIR_OUT){
                    if(fd_out != STDIN_FILENO) close(fd_in);
                    fd_out = open(red_target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fd_out < 0){
                        dprintf(fd_err, "olive-sh: > %s: failed to open redirection target", red_target);
                        exit(1);
                    }
                }
                else if(red->type == TOKEN_APPEND){
                    if(fd_out != STDOUT_FILENO) close(fd_in);
                    fd_out = open(red_target, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if(fd_out < 0){
                        dprintf(fd_err, "olive-sh: >> %s: failed to open redirection target", red_target);
                        exit(1);
                    }
                }
            }

            if(fd_in != STDIN_FILENO){
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            if(fd_out != STDOUT_FILENO){
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            } 
            if(fd_err != STDERR_FILENO){
                dup2(fd_err, STDERR_FILENO);
                close(fd_err);
            }

            // --- SETUP PARAMETERS --------------------------------------------------
            char **argv = arg_chain_to_array(env, cmd_node->argv);
            if(!argv){
                char *unbound_info = expand_var(env, "ERRLOG");
                dprintf(STDERR_FILENO, "olive-sh: couldn't resolve argument chain (%s)\n", unbound_info);
                exit(1);
            }

            char *cmd_name = argv[0];
            
            char **envp = env_chain_to_array(env);
            if(!envp){
                char *unbound_info = expand_var(env, "ERRLOG");
                dprintf(STDERR_FILENO, "olive-sh: couldn't resolve environment (%s)\n", unbound_info);
                exit(1);
            }

            // --- FIND COMMAND PATH --------------------------------------------------
            char *cmd_path = NULL;

            if(strchr(cmd_name, '/') != NULL){      // Run from given path 

                // Resolving path
                cmd_path = realpath(cmd_name, NULL);
                if(!cmd_path){
                    dprintf(STDERR_FILENO, "olive-sh: %s: no such file or directory\n", cmd_name);
                    exit(127);
                }

                // Permission test 
                struct stat buf;
                if(stat(cmd_path, &buf) != 0){
                    dprintf(STDERR_FILENO, "olive-sh: %s: no such file or directory\n", cmd_name);
                    exit(127);
                }
                if(!S_ISREG(buf.st_mode)){
                    dprintf(STDERR_FILENO, "olive-sh: %s is a directory\n", cmd_name);
                    exit(126);
                }
                if(access(cmd_path, X_OK) != 0){
                    dprintf(STDERR_FILENO, "olive-sh: %s: permission denied\n", cmd_name);
                    exit(126);
                }
            }
            else{    // Run a command 
                cmd_path = find_cmd_path(env, cmd_name);
                if(!cmd_path){
                    dprintf(STDERR_FILENO, "olive-sh: %s: command not found\n", cmd_name);
                    exit(127);
                }
            }

            print_debug("Found command %s at %s\n", cmd_name, cmd_path);
            
            execve(cmd_path, argv, envp);

            dprintf(STDERR_FILENO, "olive-sh: %s: %s\n", cmd_name, strerror(errno));
            if(errno == ENOENT) exit(127);
            else exit(126);

        // Parent process
        default:    
            return pid;
    }
}


int run_cmd(env_t *env, ast_node_t *cmd_node){
    if(!env || !cmd_node) return 0;

    print_debug("Running command\n");
    
    int argc = count_args(cmd_node->argv);
    print_debug("argc setup\n");

    // ----- SET ARGUMENTS UP -----
    char **argv = arg_chain_to_array(env, cmd_node->argv);
    if(!argv){
        char *unbound_info = expand_var(env, "ERRLOG");
        env_export(env, "ERRLOG", "olive-sh: failed to resolve argument chain (%s)", unbound_info);
        free(unbound_info);
        return 1;
    }
    print_debug("argv setup\n");

    char *cmd_name = argv[0];
    print_debug("name setup\n");

    if(cfg_infos.xtrace){
        char *ps4 = expand_var(env, "PS4");
        fprintf(stderr, "%s ", ps4);
        for (int i = 0; i < argc; i++) fprintf(stderr, "%s ", argv[i]);
        printf("\n");
        free(ps4);
    }
            
    // ------ RUN BUILTINS -----------------------------------------
    for (size_t i = 0; builtins[i].name != NULL; i++){
        if(strcmp(cmd_name, builtins[i].name) == 0){

            // --- ENVP --- 
            char **envp = env_chain_to_array(env);
            if(!envp) { 
                char *unbound_info = expand_var(env, "ERRLOG");
                env_export(env, "ERRLOG", "olive-sh: failed to resolve environment (%s)", unbound_info);
                free(unbound_info);
                free_arg_array(argv); 
                return 1; 
            }
            print_debug("envp setup\n");

            // --- REDIRS ---   
            int fd_in = STDIN_FILENO;
            int fd_out = STDOUT_FILENO;
            
            for(redir_t *red = cmd_node->redirs->first; red != NULL; red = red->next){

                char *red_target = expand_segment_chain(env, red->target);
                if(!red_target){
                    char *unbound_info = expand_var(env, "ERRLOG");
                    env_export(env, "ERRLOG", "olive-sh: failed to resolve redirection target (%s)", unbound_info);
                    free(unbound_info);
                    free_arg_array(argv);
                    return 1;
                }

                if(red->type == TOKEN_REDIR_IN) fd_in = open(red_target, O_RDONLY, 0644);
                else if(red->type == TOKEN_REDIR_OUT) fd_out = open(red_target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                else if(red->type == TOKEN_APPEND) fd_out = open(red_target, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }

            int save_in = -1;
            int save_out = -1;
            if(fd_in != STDIN_FILENO){
                save_in = dup(STDIN_FILENO);
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }
            if(fd_out != STDOUT_FILENO){
                save_out = dup(STDOUT_FILENO);
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            // ---------- RUN ---------- 
            int res = builtins[i].func(argc, argv, env);

            // Clean 
            free_arg_array(argv);
            free_env_array(envp);
            if(save_in != -1){
                dup2(save_in, STDIN_FILENO); close(save_in);
            }
            if(save_out != -1){
                 dup2(save_out, STDOUT_FILENO); close(save_out);
            } 

            return res;
        } 
    }
    
    free_arg_array(argv);
   

    // ----- RUN EXTERNALS ------------------------------------------

        // set error pipe up 
    int pipe_err[2];
    if(pipe(pipe_err) < 0){
        env_export(env, "ERRLOG", "internal error setting up stderr pipe");
        return 1;
    }

    pid_t cmd_pid = run_cmd_async(env, cmd_node, STDIN_FILENO, STDOUT_FILENO, pipe_err[1]);
                // Redirections are reprocessed in run_cmd_async
    close(pipe_err[1]);

    int res;
    waitpid(cmd_pid, &res, 0);

    char err_buff[MAX_ERROR_LEN];
    ssize_t n = read(pipe_err[0], err_buff, sizeof(err_buff) - 1);
    close(pipe_err[0]);
    if(n < 0) n = 0;
    err_buff[n] = '\0';
            
    int real_res;
    if(WIFSIGNALED(res))
        real_res =  128 + WTERMSIG(res);
    else real_res = WEXITSTATUS(res);

    if(real_res != 0) env_export(env, "ERRLOG", "%s", err_buff);

    return real_res;
}


int run_pipe(env_t *env, ast_node_t *pipe_node){

    print_debug("Handling pipe\n");

    size_t cmd_count = count_leaves(pipe_node);  // count >= 2, checked during parsing
    ast_node_t **cmd_tab = leaves_table(pipe_node, cmd_count);
    int (*std_pipe_tab)[2] = (int (*)[2])malloc(sizeof(int (*)[2]) * (cmd_count - 1));
    int (*err_pipe_tab)[2] = (int (*)[2])malloc(sizeof(int (*)[2]) * cmd_count);

    // --- Initialize pipes --- 
    for (size_t i = 0; i < cmd_count - 1; i++){
        if (pipe(std_pipe_tab[i]) != 0){
            perror("pipe");
            exit(1);
        }
        fcntl(std_pipe_tab[i][0], F_SETFD, FD_CLOEXEC);
        fcntl(std_pipe_tab[i][1], F_SETFD, FD_CLOEXEC);
    }
    for (size_t i = 0; i < cmd_count; i++){
        if (pipe(err_pipe_tab[i]) != 0){
            perror("pipe");
            exit(1);
        }
        fcntl(err_pipe_tab[i][0], F_SETFD, FD_CLOEXEC);
        fcntl(err_pipe_tab[i][1], F_SETFD, FD_CLOEXEC);
    }
    

    // --- Run commands asynchronously --- 
    pid_t *pid_tab = (pid_t*)malloc(sizeof(pid_t) * cmd_count);
    for (size_t i = 0; i < cmd_count; i++){

        int fd_in = (i == 0) ? STDIN_FILENO : std_pipe_tab[i-1][0];
        int fd_out = (i == cmd_count - 1) ? STDOUT_FILENO : std_pipe_tab[i][1];

        pid_tab[i] = run_cmd_async(env, cmd_tab[i], fd_in, fd_out, err_pipe_tab[i][1]);

        // fork failure
        if(pid_tab[i] < 0){ 
            env_export(env, "ERRLOG", "fork failure");
            for (size_t j = 0; j < i; j++){
                kill(pid_tab[j], SIGKILL);
                waitpid(pid_tab[j], NULL, 0);
            }
            for(size_t i = 0; i < cmd_count - 1; i++){ 
                close(std_pipe_tab[i][0]);  close(std_pipe_tab[i][1]); 
                close(err_pipe_tab[i][0]);  close(err_pipe_tab[i][1]);
            }

            free(pid_tab);
            free(std_pipe_tab);
            free(err_pipe_tab);

            return 1;
        }
    }
    
    print_debug("All process started\n");

    for(size_t i = 0; i < cmd_count - 1; i++){
        close(std_pipe_tab[i][0]);
        close(std_pipe_tab[i][1]);
        close(err_pipe_tab[i][1]);
    }   
    close(err_pipe_tab[cmd_count - 1][1]);

    // --- Handle commands return ---
    int real_res;
    size_t i = 0;
    while(i < cmd_count){

        int res;
        print_debug("Waiting for process %d to exit...", pid_tab[i]);
        waitpid(pid_tab[i], &res, 0);

        if(WIFSIGNALED(res))
            real_res =  128 + WTERMSIG(res);
        else real_res = WEXITSTATUS(res);

        if(real_res != 0){
            if(cfg_infos.pipefail) break;
            else if(i == cmd_count - 1) break;
        }

        close(err_pipe_tab[i][0]);
        i ++;
    }
    
        // Error during pipe execution 
    if(real_res != 0){
        char err_buff[MAX_ERROR_LEN];
        size_t n = read(err_pipe_tab[i][0], err_buff, sizeof(err_buff) - 1);
        close(err_pipe_tab[i][0]);

        if(n < 0) err_buff[0] = '\0';
        err_buff[n] = '\0';
        
        env_export(env, "ERRLOG", "%s", err_buff);
        
        for(size_t j = i + 1; j < cmd_count; j++){
            close(err_pipe_tab[j][0]);
            kill(pid_tab[j], SIGKILL);
        }
    }

    print_debug("Done handling pipe\n");

    free(pid_tab);
    free(std_pipe_tab);
    free(err_pipe_tab);

    return real_res;
}