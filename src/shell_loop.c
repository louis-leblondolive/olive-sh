#include "shell_loop.h"


int run_shell(char **envp){

    lexer_res_t lex_res;
    parse_res_t parse_res;

    // ----- SHELL INIT -----------------------------------------------------------------------------
    int init_res = 0;

    // Init signal handlers
    if(init_shell_sa_handlers() != 0){
        perror("sigaction");
        exit(1);
    }
    
    // Init environment 
    env_t env = NULL;
    init_res = env_array_to_chain(envp, &env);
    if(init_res != 0){
        print_error("SYSTEM ERROR", "failed to initialize environment", "environment initialization failed");
        free_env(env);
        return init_res;
    }
    init_res = env_export(&env, "ERRLOG", "");
    init_res += env_export(&env, "ERRTIME", "");
    init_res += env_export(&env, "ERRCMD", "");
    init_res += env_export(&env, "?", "0");
    init_res += env_export(&env, "PS4", "+");
    if(init_res != 0){
        print_error("SYSTEM ERROR", "failed to initialize environment", "initial variables export failed");
        free_env(env);
        return init_res;
    }

    // Init job control 
    init_res += save_shell_pid();
    init_res += init_foreground_job();
    init_res += set_shell_foreground();

    if(init_res != 0){
        print_error("SYSTEM ERROR", "failed to initialize job control interface", "shell couldn't be assigned as foreground job");
        free_env(env);
        free_foreground_job();
        return init_res;
    }


    // Init job table
    job_table_t *job_tbl = job_table_init();
    init_res += set_main_job_table(job_tbl);

    if(init_res != 0){
        print_error("SYSTEM ERROR", "failed to initialize job control interface", "job table initialization failed");
        free_env(env);
        free_foreground_job();
        return init_res;
    }


    // ----- MAIN SHELL LOOP -----------------------------------------------------------------------------
    while(1){

        if(sigsetjmp(jump_buffer, 1) != 0){
            if(lex_res.tk_chain) free_token_chain(lex_res.tk_chain);
            if(parse_res.ast) free_ast(parse_res.ast);
            rl_free_line_state();
            rl_cleanup_after_signal();
        }

        lex_res.tk_chain = NULL;
        parse_res.ast = NULL;
        jump_active = 1;

        // Read user command line
        char *line = readline("> ");
        jump_active = 0;
        
        if(!line) continue;
        if(*line) add_history(line);


        // ----- LEXING ----------------------------------------------------- 
        lex_res.error_pos = 0;

        lex_res = lex_input(line, strlen(line));

        // Handle lexing error
        if(!lex_res.success){
            print_error("LEXING ERROR", lex_res.error, lex_res.error_info);

            env_export(&env, "ERRCMD", "%s", line);
            env_export(&env, "ERRLOG", "%s", lex_res.error_info);
            env_export(&env, "?", "2");

            free_token_chain(lex_res.tk_chain);
            free(lex_res.error);
            free(lex_res.error_info);
            continue;
        }

        // Debug 
        print_token_chain(lex_res.tk_chain);
        print_debug("Done lexing\n");


        // ----- PARSING -----------------------------------------------------

        parse_res = build_ast(lex_res.tk_chain);

        // Handle parsing error 
        if(!parse_res.success){
            print_error("PARSING ERROR", parse_res.error, parse_res.error_info);

            env_export(&env, "ERRCMD", "%s", line);
            env_export(&env, "ERRLOG", "%s", parse_res.error_info);
            env_export(&env, "?", "2");

            free_token_chain(lex_res.tk_chain);
            free_ast(parse_res.ast);
            free(parse_res.error);
            free(parse_res.error_info);
            continue;
        }

        // Debug    
        print_ast(parse_res.ast, 0);
        print_debug("Done parsing\n");


        // ----- EXECUTION ---------------------------------------------------
        int err_pipe[2];
        if(pipe(err_pipe) != 0){
            perror("pipe");

            free_token_chain(lex_res.tk_chain);
            free_ast(parse_res.ast);

            env_export(&env, "ERRCMD", "%s", line);
            env_export(&env, "ERRLOG", "olive-sh: pipe initialization issue");
            env_export(&env, "?", "2");

            continue;
        }

        exec_res_t res = run_ast(&env, parse_res.ast, STDIN_FILENO, STDOUT_FILENO, err_pipe[1]);

        close(err_pipe[1]);

        switch(res.kind){

            case RES_STOPPED:
                env_export(&env, "?", "%d", 128 + res.stop_sig);
                set_shell_foreground();
                break;

            case RES_SIGNALED:
                env_export(&env, "?", "%d", 128 + res.term_sig);
                // custom signal errors here
                break;

            case RES_EXITED:

                env_export(&env, "?", "%d", res.exit_code);

                if(res.exit_code != 0){
        
                    if(cfg_infos.errexit) exit(res.exit_code);

                    // error-causing command 
                    env_export(&env, "ERRCMD", "%s", line);

                    // error code 
                    char err_descr[32 + 26];
                    snprintf(err_descr, sizeof(err_descr), "process exited with code %d", res.exit_code);

                    // errlog
                    char errlog[MAX_ERROR_LEN];
                    ssize_t n = read(err_pipe[0], errlog, sizeof(errlog) - 1);

                    if(n < 0) { errlog[0] = '\0'; n = 1; }
                    else{ errlog[n] = '\0'; }

                    env_export(&env, "ERRLOG", "%s", errlog);

                    print_error("EXECUTION ERROR", err_descr, errlog);
                }
                break;

            
            default:
                print_error("SYSTEM ERROR", "couldn't determine termination cause", "");
                env_export(&env, "?", "%d", 2);
                break;
        }

        
        
        close(err_pipe[0]);
        
        print_debug("Done execution\n");

        // ----- CLEAN --------------------------------------------------
        free_token_chain(lex_res.tk_chain);
        free_ast(parse_res.ast);
    }

    free_env(env);
    free_foreground_job();
    free_main_job_table();

    return 0;
}