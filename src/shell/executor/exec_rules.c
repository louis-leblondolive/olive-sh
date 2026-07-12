#include "exec_rules.h"


exec_res_t exec_res_from_builtin(int exit_code){

    exec_res_t res;
    res.exit_code = exit_code;
    res.kind = RES_EXITED;

    return res;
}


exec_res_t exec_res_from_waitpid_status(int status){

    exec_res_t res;
    
    if(WIFSIGNALED(status)){
        res.kind = RES_SIGNALED;
        res.term_sig = WTERMSIG(status);
    }

    else if(WIFSTOPPED(status)){
        res.kind = RES_STOPPED;
        res.stop_sig = WSTOPSIG(status);
    }

    else {
        res.kind = RES_EXITED;
        res.exit_code = status;
    }

    return res;
}


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


int open_cloexec_pipe(int fds[2]){

    if(pipe(fds) != 0){
        perror("pipe");
        return 1;
    }
    fcntl(fds[0], F_SETFD, FD_CLOEXEC);
    fcntl(fds[1], F_SETFD, FD_CLOEXEC);

    return 0;
}

void close_pipe(int fds[2]){
    close(fds[0]);
    close(fds[1]);
}