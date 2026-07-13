#include "executor.h"
#include "exec_internal.h"


exec_res_t run_ast(env_t *env, job_table_t *job_tbl,
     ast_node_t *ast, int std_fd_in, int std_fd_out, int std_fd_err){

    if(!ast) return exec_res_from_builtin(0);

    exec_res_t cache_res;
    switch(ast->token){

        // --- OPERATORS --------------------------------------------------

        case TOKEN_SEQ:
            run_ast(env, job_tbl, ast->left, std_fd_in, std_fd_out, std_fd_err);
            return run_ast(env, job_tbl, ast->right, std_fd_in, std_fd_out, std_fd_err);


        case TOKEN_AND:
            cache_res = run_ast(env, job_tbl, ast->left, std_fd_in, std_fd_out, std_fd_err);

            if(cache_res.kind != RES_EXITED || cache_res.exit_code != 0) 
                return cache_res;

            return run_ast(env, job_tbl, ast->right, std_fd_in, std_fd_out, std_fd_err);


        case TOKEN_OR:
            cache_res = run_ast(env, job_tbl, ast->left, std_fd_in, std_fd_out, std_fd_err);

            if(cache_res.kind == RES_EXITED && cache_res.exit_code == 0) 
                return cache_res;

            return run_ast(env, job_tbl, ast->right, std_fd_in, std_fd_out, std_fd_err);

        // --- PIPELINE ----------------------------------------------------

        case TOKEN_PIPE: {

            bool is_leader = is_shell_foreground();

            char *cmd = strdup(ast->str_cmd);
            job_t *leader_job = NULL;

            if(is_leader){

                int err_pipe[2];
                if(pipe(err_pipe) != 0){
                    perror("pipe");
                    return exec_res_from_builtin(1);
                }

                pid_t leader_pid = fork();
                switch(leader_pid){

                    case -1:    
                        perror("fork");
                        return exec_res_from_builtin(1);

                    case 0: 
                        if(reset_sa_handlers() != 0){
                            perror("signal");
                            exit(1);
                        }

                            // Init new leader
                        leader_pid = getpid();
                        setpgid(0, leader_pid);
                        
                        leader_job = job_init(leader_pid, leader_pid, err_pipe[0], cmd);
                        set_foreground_job(leader_job);

                            // Run pipe node 
                        exec_res_t pipe_res = run_pipe_children(env, job_tbl, ast, leader_pid, 
                            std_fd_in, std_fd_out, err_pipe[1]);
                        
                        close_pipe(err_pipe);

                        switch(pipe_res.kind){
                            case RES_EXITED: exit(pipe_res.exit_code);
                            case RES_STOPPED: exit(pipe_res.stop_sig);
                            case RES_SIGNALED: exit(pipe_res.term_sig);
                            default: exit(1);
                        }
                        

                    default:
                        close(err_pipe[1]);

                            // Init new leader
                        leader_job = job_init(leader_pid, leader_pid, err_pipe[0], cmd);
                        set_foreground_job(leader_job);

                            // Run pipe node foreground 
                        setpgid(leader_pid, leader_pid);
                        tcsetpgrp(STDIN_FILENO, leader_pid);

                        int exit_status; 
                        waitpid(leader_pid, &exit_status, WUNTRACED);

                            // Clean and exit 
                        if(WIFSTOPPED(exit_status)){
                            if(suspend_job(job_tbl, leader_job) != 0){
                                dprintf(std_fd_err, "Error during job suspension\n");
                                close(err_pipe[0]);
                                return exec_res_from_builtin(1);
                            }

                            return exec_res_from_waitpid_status(exit_status);
                        }

                        if(exit_status != 0) relay_child_error(err_pipe[0], std_fd_err);
                        close(err_pipe[0]);

                        set_shell_foreground();

                        return exec_res_from_waitpid_status(exit_status);
                }  
            }
            else { // Intermediate supervisor  
                pid_t group_pgid = get_foreground_job()->pgid;

                return run_pipe_children(env, job_tbl, ast, group_pgid, 
                    std_fd_in, std_fd_out, std_fd_err);
            }
        }
        
        // --- BACKGROUND ---------------------------------------------------

        case TOKEN_AMP: 
            print_error("EXECUTION ERROR", "'&' operator not yet implemented", "");
            return exec_res_from_builtin(1);

        // --- SIMPLE COMMAND ------------------------------------------------

        case TOKEN_WORD: {   // Running command

            // ----- SET ARGUMENTS UP -----
            int argc = 0; 
            char **argv = NULL;
            char **envp = NULL;

            if(setup_params(env, ast, &argc, &argv, &envp) != 0){

                char *errlog = expand_var(env, "ERRLOG");
                write(std_fd_err, errlog, strlen(errlog));
                return exec_res_from_builtin(1);
            }

            char *cmd_name = strdup(argv[0]);

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
                return exec_res_from_builtin(1);
            }

            // ----- RUN -----
            exec_res_t exec_res;

            int builtin_id = is_builtin(cmd_name);
            if(builtin_id >= 0){  // -------------- Run builtin command 
                int exit_status = run_builtin(builtin_id, argc, argv, env, fd_in, fd_out);

                if(exit_status != 0){
                    char *errlog = expand_var(env, "ERRLOG");
                    write(std_fd_err, errlog, strlen(errlog));
                }

                exec_res = exec_res_from_builtin(exit_status);
            }
            
            else { // ----------------------------- Run external command 

                bool is_leader = is_shell_foreground();

                if(is_leader){

                    int err_pipe[2];
                    if(pipe(err_pipe) != 0){
                        perror("pipe");
                        clean_exec_vars(argv, envp);
                        clean_io_fds(fd_in, fd_out, std_fd_in, std_fd_out);
                        return exec_res_from_builtin(1);
                    }

                    char *cmd = strdup(ast->str_cmd);
                    job_t *leader_job = NULL;
                    int exit_status = -1;

                    pid_t leader_pid = fork();
                    switch(leader_pid){

                        case -1:
                            perror("fork");
                            clean_exec_vars(argv, envp);
                            clean_io_fds(fd_in, fd_out, std_fd_in, std_fd_out);
                            return exec_res_from_builtin(1);

                        case 0:
                            if(reset_sa_handlers() != 0){
                                perror("signal");
                                exit(1);
                            }

                            leader_pid = getpid();
                            setpgid(0, leader_pid);

                                // Init leader job  
                            
                            leader_job = job_init(leader_pid, leader_pid, err_pipe[0], cmd);
                            set_foreground_job(leader_job);

                                // Run command
                            exit_status = run_cmd_async(env, argv, envp, fd_in, fd_out, err_pipe[1]);
                            close_pipe(err_pipe);

                            exit(exit_status);


                        default:
                            close(err_pipe[1]);

                                // Init leader job  
                            leader_job = job_init(leader_pid, leader_pid, err_pipe[0], cmd);
                            set_foreground_job(leader_job);

                                // Run command foreground
                            setpgid(leader_pid, leader_pid);
                            tcsetpgrp(STDIN_FILENO, leader_pid);

                            waitpid(leader_pid, &exit_status, WUNTRACED);

                                // Clean and exit 
                            if(WIFSTOPPED(exit_status)){
                                if(suspend_job(job_tbl, leader_job) != 0){
                                    dprintf(std_fd_err, "Error during job suspension\n");
                                    close(err_pipe[0]);
                                    return exec_res_from_builtin(1);
                                }

                                return exec_res_from_waitpid_status(exit_status);
                            }

                            if(exit_status != 0) relay_child_error(err_pipe[0], std_fd_err);
                            close(err_pipe[0]);

                            set_shell_foreground();

                            return exec_res_from_waitpid_status(exit_status);
                    }
                } 
                else { // already in a forked process

                    int exit_status = run_cmd_async(env, argv, envp, fd_in, fd_out, std_fd_err);
                    exec_res = exec_res_from_builtin(exit_status);
                }

            } // end run external


            // ----- CLEAN -----
            clean_exec_vars(argv, envp);
            clean_io_fds(fd_in, fd_out, std_fd_in, std_fd_out);

            print_debug("Done running command\n");

            return exec_res;
        }


        default:        // unreachable
            return exec_res_from_builtin(1);
    }

    return exec_res_from_builtin(0);
}