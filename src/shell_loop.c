#include "shell_loop.h"


int run_shell(){

    env_t *env = NULL;

    token_chain_t *tk_chain;
    lex_exit_status_e lex_res;
    parse_res_t parse_res;

    while(1){

        // Read user command line
        char *line = readline("> ");
        if(!line) continue;
        if(*line) add_history(line);

        // ----- LEXING ----------------------------------------------------- 
        tk_chain = init_token_chain();
        lex_res = build_token_list(line, strlen(line), tk_chain);

        // Handle lexing error
        if(lex_res != LEX_OK){
            free_token_chain(tk_chain);
            const char *err_reason = lex_exit_status_to_str(lex_res);
            print_error("%s\n", err_reason);
            
            if(lex_res == LEX_FATAL) exit(1);
            continue;
        }

        // Debug 
        print_token_chain(tk_chain);
        print_debug("Done lexing\n");


        // ----- PARSING -----------------------------------------------------

        parse_res = build_ast(tk_chain);

        // Handle parsing error 
        if(!parse_res.success){
            free_token_chain(tk_chain);
            free_ast(parse_res.ast);
            print_error("%s\n", parse_res.error);
            continue;
        }

        // Debug    
        print_ast(parse_res.ast, 0);
        print_debug("Done parsing\n");


        // ----- EXECUTION ----------------------------------------------------

        int res = run_ast(env, parse_res.ast);
        print_debug("exec res : %d\n", res);

        // ----- FREE ALLOCATED DATA ------------------------------------------
        free_token_chain(tk_chain);
        free_ast(parse_res.ast);
    }

    return 0;
}