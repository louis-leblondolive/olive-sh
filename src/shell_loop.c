#include "shell_loop.h"


int run_shell(char **envp){

    lexer_res_t lex_res;
    parse_res_t parse_res;

    // Init signal handlers
    if(init_shell_sa_handlers() != 0){
        perror("sigaction");
        exit(1);
    }

    // Init environment 
    env_t env = NULL;
    int init_res = env_array_to_chain(envp, &env);
    if(init_res != 0){
        print_error("SYSTEM ERROR", "failed to initialize environment", "environment initialization failed");
        free_env(env);
        return init_res;
    }
    init_res = env_export(&env, "ERRLOG", "");
    init_res += env_export(&env, "ERRTIME", "");
    init_res += env_export(&env, "ERRCMD", "");
    init_res += env_export(&env, "?", "0");
    if(init_res != 0){
        print_error("SYSTEM ERROR", "failed to initialize environment", "initial variables export failed");
        free_env(env);
        return init_res;
    }


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


        // ----- EXECUTION ----------------------------------------------------

        char *cache_err_time = expand_var(&env, "ERRTIME");

        int res = run_ast(&env, parse_res.ast);

        env_export(&env, "?", "%d", res);

        if(res != 0){ // an error occured

            if(cfg_infos.errexit) exit(res);

            // signal error 
            char err_descr[32 + 26];
            snprintf(err_descr, sizeof(err_descr), "process exited with code %d", res);
            
            char *err_time = expand_var(&env, "ERRTIME");
            if(strncmp(cache_err_time, err_time, MAX_ERROR_LEN) == 0){ 
                // ERRLOG not affected by error 
                print_debug("Errlog not modified by last error\n");
                env_export(&env, "ERRLOG", ""); // no details attached to this error
                print_error("EXECUTION ERROR", err_descr, NULL);

            } else {
                // ERRLOG set to error description 
                char *errlog = expand_var(&env, "ERRLOG");
                print_error("EXECUTION ERROR", err_descr, errlog);
                free(errlog);
            }

            env_export(&env, "ERRCMD", "%s", line);
            
            free(err_time);
        }

        
        // ----- FREE ALLOCATED DATA ------------------------------------------
        free_token_chain(lex_res.tk_chain);
        free_ast(parse_res.ast);
        free(cache_err_time);
    }

    return 0;
}