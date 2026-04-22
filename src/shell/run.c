#include "run.h"


int run_shell(){

    token_chain_t *tk_chain;
    lex_exit_status_e lex_res;

    while(1){

        // Read user command line
        char *line = readline("> ");
        if(!line) continue;
        if(*line) add_history(line);

        // Lex line 
        tk_chain = init_token_chain();
        lex_res = build_token_list(line, strlen(line), tk_chain);
        print_debug("Done lexing\n");

        if(lex_res != LEX_OK){
            free_token_chain(tk_chain);
            const char *err_reason = lex_exit_status_to_str(lex_res);
            print_error("%s\n", err_reason);
            
            if(lex_res == LEX_FATAL) exit(1);
            continue;
        }

        // Debug 
        print_token_chain(tk_chain);


        free_token_chain(tk_chain);
    }

    return 0;
}