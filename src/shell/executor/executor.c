#include "executor.h"
#include "exec_internal.h"


int run_ast(env_t *env, ast_node_t *ast, int exec_in, int exec_out){
    if(!ast) return 0;

    int cache_res;
    switch(ast->token){

        // --- OPERATORS --------------------------------------------------

        case TOKEN_SEQ:
            run_ast(env, ast->left, exec_in, exec_out);
            return run_ast(env, ast->right, exec_in, exec_out);

        case TOKEN_AND:
            cache_res = run_ast(env, ast->left, exec_in, exec_out);
            if(cache_res != 0) return cache_res;
            return run_ast(env, ast->right, exec_in, exec_out);

        case TOKEN_OR:
            cache_res = run_ast(env, ast->left, exec_in, exec_out);
            if(cache_res == 0) return 0;
            return run_ast(env, ast->right, exec_in, exec_out);

        // --- PIPELINE ----------------------------------------------------

        case TOKEN_PIPE: {

            int pipeline[2];
            if(pipe(pipeline) != 0){
                perror("pipe"); return 1;
            }
            fcntl(pipeline[0], F_SETFD, FD_CLOEXEC);
            fcntl(pipeline[1], F_SETFD, FD_CLOEXEC);

            pid_t left_chld = fork();
            if(left_chld == -1) { 
                perror("fork"); 
                close(pipeline[0]); close(pipeline[1]);
                return 1; 
            }
            else if(left_chld == 0) {
                close(pipeline[0]);
                exit(run_ast(env, ast->left, exec_in, pipeline[1]));
            }

            pid_t right_chld = fork();
            if(right_chld == -1) { 
                perror("fork"); 
                close(pipeline[0]); close(pipeline[1]);
                return 1; 
            }
            else if(right_chld == 0) {
                close(pipeline[1]);
                exit(run_ast(env, ast->right, pipeline[0], exec_out));
            }

            close(pipeline[0]); 
            close(pipeline[1]);

            int l_status, r_status;
            waitpid(left_chld, &l_status, 0);
            waitpid(right_chld, &r_status, 0);

            l_status = clean_result(l_status);
            r_status = clean_result(r_status);

            if(l_status != 0 && cfg_infos.pipefail) return l_status;
            
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

            int fd_in = exec_in;
            int fd_out = exec_out;

            if(setup_redirs(env, ast, &fd_in, &fd_out) != 0){
                clean_exec_vars(argv, envp);
                clean_io_fds(fd_in, fd_out, exec_in, exec_out);
                return 1;
            }

            // ----- RUN -----
            int exit_status;

            int builtin_id = is_builtin(cmd_name);
            if(builtin_id >= 0){  // -------------- Run builtin command 
                exit_status = run_builtin(builtin_id, argc, argv, env, fd_in, fd_out);
            }
            
            else { // ----------------------------- Run external command 

                // Set error pipe up 
                int pipe_err[2];
                if(pipe(pipe_err) < 0){
                    env_export(env, "ERRLOG", "internal error setting up stderr pipe");
                    clean_exec_vars(argv, envp);
                    clean_io_fds(fd_in, fd_out, exec_in, exec_out);
                    return 1;
                }

                // Run 
                pid_t cmd_pid = run_cmd_async(env, argv, envp, fd_in, fd_out, pipe_err[1]);
                close(pipe_err[1]);

                int raw_res;
                waitpid(cmd_pid, &raw_res, 0);

                // Process exit 
                char err_buff[MAX_ERROR_LEN];
                ssize_t n = read(pipe_err[0], err_buff, sizeof(err_buff) - 1);
                close(pipe_err[0]);
                if(n < 0) n = 0;
                err_buff[n] = '\0';

                exit_status = clean_result(raw_res);

                if(exit_status != 0) env_export(env, "ERRLOG", "%s", err_buff);
            }

            // ----- CLEAN -----
            clean_exec_vars(argv, envp);
            clean_io_fds(fd_in, fd_out, exec_in, exec_out);

            return exit_status;
        }

        default:        // unreachable
            return 1;
    }

    return 0;
}