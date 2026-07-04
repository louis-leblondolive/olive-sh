#include "executor.h"
#include "exec_internal.h"


int run_ast(env_t *env, ast_node_t *ast, int std_fd_in, int std_fd_out, int std_fd_err){
    if(!ast) return 0;

    int cache_res;
    switch(ast->token){

        // --- OPERATORS --------------------------------------------------

        case TOKEN_SEQ:
            run_ast(env, ast->left, std_fd_in, std_fd_out, std_fd_err);
            return run_ast(env, ast->right, std_fd_in, std_fd_out, std_fd_err);

        case TOKEN_AND:
            cache_res = run_ast(env, ast->left, std_fd_in, std_fd_out, std_fd_err);
            if(cache_res != 0) return cache_res;
            return run_ast(env, ast->right, std_fd_in, std_fd_out, std_fd_err);

        case TOKEN_OR:
            cache_res = run_ast(env, ast->left, std_fd_in, std_fd_out, std_fd_err);
            if(cache_res == 0) return 0;
            return run_ast(env, ast->right, std_fd_in, std_fd_out, std_fd_err);

        // --- PIPELINE ----------------------------------------------------

        case TOKEN_PIPE: {

            // --- Setup pipes ---------------
            int io_pipe[2];
            int left_err_pipe[2];
            int right_err_pipe[2];
            if(pipe(io_pipe) != 0){
                perror("pipe"); return 1;
            }
            if(pipe(left_err_pipe) != 0){
                perror("pipe"); close_pipe(io_pipe); return 1;
            }
            if(pipe(right_err_pipe) != 0){
                perror("pipe"); 
                close_pipe(io_pipe); close_pipe(left_err_pipe);
                return 1;
            }
            fcntl(io_pipe[0], F_SETFD, FD_CLOEXEC); fcntl(io_pipe[1], F_SETFD, FD_CLOEXEC);
            fcntl(left_err_pipe[0], F_SETFD, FD_CLOEXEC); fcntl(left_err_pipe[1], F_SETFD, FD_CLOEXEC); 
            fcntl(right_err_pipe[0], F_SETFD, FD_CLOEXEC); fcntl(right_err_pipe[1], F_SETFD, FD_CLOEXEC); 

            // --- Run left and right nodes ---------------
            pid_t left_chld = fork();
            if(left_chld == -1) { 
                perror("fork"); 
                close_pipe(io_pipe);
                close_pipe(left_err_pipe); close_pipe(right_err_pipe);
                return 1; 
            }
            else if(left_chld == 0) {
                close(io_pipe[0]);
                exit(run_ast(env, ast->left, std_fd_in, io_pipe[1], left_err_pipe[1]));
            }

            pid_t right_chld = fork();
            if(right_chld == -1) { 
                perror("fork"); 
                close_pipe(io_pipe);
                close_pipe(left_err_pipe); close_pipe(right_err_pipe);
                return 1; 
            }
            else if(right_chld == 0) {
                close(io_pipe[1]);
                exit(run_ast(env, ast->right, io_pipe[0], std_fd_out, right_err_pipe[1]));
            }

            // --- Clean and exit ---------------
            close_pipe(io_pipe);
            close(left_err_pipe[1]);
            close(right_err_pipe[1]);

            int l_status, r_status;
            waitpid(left_chld, &l_status, 0);
            waitpid(right_chld, &r_status, 0);

            l_status = clean_result(l_status);
            r_status = clean_result(r_status);

            if(l_status != 0 && cfg_infos.pipefail){

                char errlog[MAX_ERROR_LEN];
                ssize_t n = read(left_err_pipe[0], errlog, sizeof(errlog) - 1);
                close(left_err_pipe[0]);

                if(n < 0) { errlog[0] = '\0'; n = 1; }
                else { errlog[n] = '\0'; }

                write(std_fd_err, errlog, (size_t)n);

                close(right_err_pipe[0]);
                return l_status;
            }
            close(left_err_pipe[0]);

            if(r_status != 0){

                char errlog[MAX_ERROR_LEN];
                ssize_t n = read(right_err_pipe[0], errlog, sizeof(errlog) - 1);

                if(n < 0) { errlog[0] = '\0'; n = 1; }
                else { errlog[n] = '\0'; }

                write(std_fd_err, errlog, (size_t)n);
            }
            close(right_err_pipe[0]);

            return r_status;
        }
        
        // --- BACKGROUND ---------------------------------------------------

        case TOKEN_AMP: 
            print_error("EXECUTION ERROR", "'&' operator not yet implemented", "");
            return 1;

        // --- SIMPLE COMMAND ------------------------------------------------

        case TOKEN_WORD: {   // Running command

            // ----- SET ARGUMENTS UP -----
            int argc = 0; 
            char **argv = NULL;
            char **envp = NULL;

            if(setup_params(env, ast, &argc, &argv, &envp) != 0){

                char *errlog = expand_var(env, "ERRLOG");
                write(std_fd_err, errlog, strlen(errlog));
                return 1;
            }

            char *cmd_name = argv[0];

            if(cfg_infos.xtrace){
                char *ps4 = expand_var(env, "PS4");
                fprintf(stderr, "%s ", ps4);
                for (int i = 0; i < argc; i++) fprintf(stderr, "%s ", argv[i]);
                printf("\n");
                free(ps4);
            }
            
            // ----- SET I/O UP -----

            int fd_in = std_fd_in;
            int fd_out = std_fd_out;

            if(setup_redirs(env, ast, &fd_in, &fd_out) != 0){

                char *errlog = expand_var(env, "ERRLOG");
                write(std_fd_err, errlog, strlen(errlog));
               
                clean_exec_vars(argv, envp);
                clean_io_fds(fd_in, fd_out, std_fd_in, std_fd_out);
                return 1;
            }

            // ----- RUN -----
            int exit_status;

            int builtin_id = is_builtin(cmd_name);
            if(builtin_id >= 0){  // -------------- Run builtin command 
                exit_status = run_builtin(builtin_id, argc, argv, env, fd_in, fd_out);

                if(exit_status != 0){
                    char *errlog = expand_var(env, "ERRLOG");
                    write(std_fd_err, errlog, strlen(errlog));
                }
            }
            
            else { // ----------------------------- Run external command 

                pid_t cmd_pid = run_cmd_async(env, argv, envp, fd_in, fd_out, std_fd_err);

                int raw_res;
                waitpid(cmd_pid, &raw_res, 0);

                exit_status = clean_result(raw_res);
            }

            // ----- CLEAN -----
            clean_exec_vars(argv, envp);
            clean_io_fds(fd_in, fd_out, std_fd_in, std_fd_out);

            return exit_status;
        }

        default:        // unreachable
            return 1;
    }

    return 0;
}