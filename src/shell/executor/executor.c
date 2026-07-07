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

            if(open_cloexec_pipe(io_pipe) != 0) return 1;
            if(open_cloexec_pipe(left_err_pipe) != 0) { close_pipe(io_pipe); return 1; }
            if(open_cloexec_pipe(right_err_pipe) != 0) { 
                close_pipe(io_pipe); close_pipe(left_err_pipe); return 1; }

            // --- Run left and right nodes ---------------
            
            bool is_leader = is_shell_foreground();

                // init left child 
            pid_t left_chld = fork();
            if(left_chld == -1) { 
                perror("fork"); 
                close_pipe(io_pipe);
                close_pipe(left_err_pipe); close_pipe(right_err_pipe);
                return 1; 
            }

                // determine pgid
            pid_t group_pgid;
            if(!is_leader) group_pgid = g_foreground_job->pg_id;
            else {
                if(left_chld == 0) group_pgid = getpid();
                else group_pgid = left_chld;
            }   
                
            if(is_leader){  // set new leader 
                char *cmd = NULL;   // update later
                job_t *new_leader = job_init(group_pgid, cmd);

                if(!new_leader){
                    close_pipe(io_pipe); close_pipe(left_err_pipe); close_pipe(right_err_pipe);
                    if(left_chld == 0) exit(1);
                    else return 1;
                }

                update_foreground_job(new_leader);
            }

                // run left child 
            setpgid(left_chld, group_pgid);

            if(left_chld == 0) {    
                close(io_pipe[0]);
                exit(run_ast(env, ast->left, std_fd_in, io_pipe[1], left_err_pipe[1]));
            }

                // init right child 
            pid_t right_chld = fork();
            if(right_chld == -1) { 
                perror("fork"); 
                close_pipe(io_pipe);
                close_pipe(left_err_pipe); close_pipe(right_err_pipe);
                return 1; 
            }

            setpgid(right_chld, group_pgid);

                // run right child 
            if(right_chld == 0) {
                close(io_pipe[1]);
                exit(run_ast(env, ast->right, io_pipe[0], std_fd_out, right_err_pipe[1]));
            }   

                // foreground new leader
            if(is_leader) tcsetpgrp(STDIN_FILENO, group_pgid);

                // clean pipes 
            close_pipe(io_pipe);
            close(left_err_pipe[1]);
            close(right_err_pipe[1]);

            // --- Wait for jobs ----------
            pid_t done_id;
            int res; 
            int l_status = 0; int r_status = 0; 
            bool l_done = false; bool r_done = false; 

            while(!(r_done && l_done)){
                
                done_id = waitpid(-g_foreground_job->pg_id, &res, WUNTRACED);
                if(done_id < 0){
                    if(errno == EINTR) continue;
                    else break;
                }

                if(done_id == left_chld){  l_status = res; l_done = true; }
                if(done_id == right_chld){ r_status = res; r_done = true; }
            }

            if(is_leader) set_shell_foreground();

            // --- Clean and exit ---------------
            l_status = clean_result(l_status);
            r_status = clean_result(r_status);

            int exit_status;

            if(l_status != 0 && cfg_infos.pipefail){
                relay_child_error(left_err_pipe[0], std_fd_err); 
                exit_status = l_status;
            }
            else{
                if(r_status != 0) relay_child_error(right_err_pipe[0], std_fd_err); 
                exit_status = r_status;
            }

            close(left_err_pipe[0]);
            close(right_err_pipe[0]);

            return exit_status;
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

                bool is_leader = is_shell_foreground();

                pid_t cmd_pid = fork();
                if(cmd_pid == -1){ perror("fork"); exit_status = 1; break; }
                
                    // determine pgid 
                pid_t leader_pid;
                if(!is_leader) leader_pid = g_foreground_job->pg_id;
                else {
                    if(cmd_pid == 0) leader_pid = getpid();
                    else leader_pid = cmd_pid;
                }
                
                if(cmd_pid == 0){
                    setpgid(0, leader_pid);
                    exit(run_cmd_async(env, argv, envp, fd_in, fd_out, std_fd_err));
                }

                if(is_leader){  // set new leader 
                    job_t *new_leader_job = job_init(leader_pid, cmd_name);
                    
                    if(!new_leader_job){
                        if(cmd_pid == 0) exit(1);
                        else { exit_status = 1; break; }
                    }

                    update_foreground_job(new_leader_job);
                    tcsetpgrp(STDIN_FILENO, leader_pid);
                }

                    // wait and clean
                int raw_res;
                waitpid(cmd_pid, &raw_res, WUNTRACED);

                if(is_leader) set_shell_foreground();

                exit_status = clean_result(raw_res);
            }

            // ----- CLEAN -----
            clean_exec_vars(argv, envp);
            clean_io_fds(fd_in, fd_out, std_fd_in, std_fd_out);

            print_debug("Done running command\n");

            return exit_status;
        }


        default:        // unreachable
            return 1;
    }

    return 0;
}