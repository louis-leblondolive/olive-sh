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
        //print_error("system error while initializing environment\n");
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

        lex_res = lex_input(line, strlen(line));

        // Handle lexing error
        if(!lex_res.success){
            print_error("LEXING ERROR", lex_res.error);

            env_export(&env, "ERRLOG", lex_res.error_info);
            
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
            print_error("PARSING ERROR", parse_res.error);

            env_export(&env, "ERRLOG", parse_res.error_info);

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

        int res = run_ast(&env, parse_res.ast);

        char buf[32];
        snprintf(buf, sizeof(buf), "%d", res);
        env_export(&env, "?", buf);

        print_info("exec res : %d\n", res);

        // ----- FREE ALLOCATED DATA ------------------------------------------
        free_token_chain(lex_res.tk_chain);
        free_ast(parse_res.ast);
    }

    return 0;
}