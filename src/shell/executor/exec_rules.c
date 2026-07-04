#include "exec_rules.h"


int is_builtin(char *cmd_name){
    for (size_t i = 0; builtins[i].name != NULL; i++){
        if(strcmp(cmd_name, builtins[i].name) == 0) return i;
    }
    return -1;
}


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


void clean_exec_vars(char **argv, char **envp){
    free_arg_array(argv);
    free_env_array(envp);
}


void clean_io_fds(int fd_in, int fd_out, int default_fd_in, int default_fd_out){
    if(fd_in > 2 && fd_in != default_fd_in) close(fd_in);
    if(fd_out > 2 && fd_out != default_fd_out) close(fd_out);
}


void close_pipe(int *pipe){
    if(!pipe) return;
    close(pipe[0]);
    close(pipe[1]);
}


int clean_result(int raw_res){
    if(WIFSIGNALED(raw_res))
        return 128 + WTERMSIG(raw_res);
    else return WEXITSTATUS(raw_res);
}



int setup_redirs(env_t *env, ast_node_t *cmd_node, int *fd_in, int *fd_out){

    for(redir_t *red = cmd_node->redirs->first; red != NULL; red = red->next){

        char *red_target = expand_segment_chain(env, red->target);
        if(!red_target){
            char *unbound_info = expand_var(env, "ERRLOG");
            env_export(env, "ERRLOG", "olive-sh: failed to resolve redirection target (%s)\n", unbound_info);
            free(unbound_info);
            free(red_target);
            return -1;
        }

        if(red->type == TOKEN_REDIR_IN){
            if(*fd_in != STDIN_FILENO) close(*fd_in);
            *fd_in = open(red_target, O_RDONLY, 0644);
            if(*fd_in < 0){
                env_export(env, "ERRLOG", "olive-sh: < %s: failed to open redirection target", red_target);
                free(red_target);
                return -1;
            }
        }
        else if(red->type == TOKEN_REDIR_OUT){
            if(*fd_out != STDOUT_FILENO) close(*fd_out);
            *fd_out = open(red_target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(*fd_out < 0){
                env_export(env, "ERRLOG", "olive-sh: > %s: failed to open redirection target", red_target);
                free(red_target);
                return -1;
            }
        }
        else if(red->type == TOKEN_APPEND){
            if(*fd_out != STDOUT_FILENO) close(*fd_out);
            *fd_out = open(red_target, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if(*fd_out < 0){
                env_export(env, "ERRLOG", "olive-sh: >> %s: failed to open redirection target", red_target);
                free(red_target);
                return -1;
            }
        }

        free(red_target);
    }

    return 0;
}


int setup_params(env_t *env, ast_node_t *cmd_node, int *argc, char ***argv, char ***envp){
    
    *argc = count_args(cmd_node->argv);
    print_debug("argc setup\n");

    *argv = arg_chain_to_array(env, cmd_node->argv);
    if(!*argv){
        char *unbound_info = expand_var(env, "ERRLOG");
        env_export(env, "ERRLOG", "olive-sh: failed to resolve argument chain (%s)", unbound_info);
        free(unbound_info);
        return -1;
    }
    print_debug("argv setup\n");
    
    *envp = env_chain_to_array(env);
    if(!*envp){
        char *unbound_info = expand_var(env, "ERRLOG");
        env_export(env, "ERRLOG", "olive-sh: couldn't resolve environment (%s)\n", unbound_info);
        free_arg_array(*argv);
        return -1;
    }
    print_debug("envp setup\n");

    return 0;
}