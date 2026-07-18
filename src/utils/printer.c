#include "printer.h"


#include "printer.h"


void print_info(char *format, ...){
    va_list args;

    printf(BOLD_BLUE);
    printf("[INFO] ");
    printf(RESET);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void print_debug(char *format, ...){
    if(!cfg_infos.debug) return;
    va_list args;

    printf(BOLD_GREEN);
    printf("[DEBUG] ");
    printf(RESET);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void print_hint(char *format, ...){
    if(!cfg_infos.hints) return;
    va_list args;

    printf(BOLD_GREEN);
    printf("Hint: ");
    printf(RESET);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


void print_error(char *error_title, char *error_description, char *errlog){

    fprintf(stderr, BOLD_RED);
    fprintf(stderr, "%s", error_title);
    fprintf(stderr, RESET);
    fprintf(stderr, " - %s\n", error_description);
    if(!cfg_infos.errlog) fprintf(stderr, "Run errlog for more details.\n");
    else if(errlog) fprintf(stderr, "%s\n", errlog);
}


void print_segment_chain(segment_t *seg_chain){
    for(segment_t *seg = seg_chain; seg != NULL; seg = seg->next){
        printf("  ");
        if(seg->type == SEG_LITERAL) printf("(LITERAL)");
        else printf("(VAR)");
        printf("[%s]", seg->value);
    }
}


void print_token_chain(token_chain_t *tk_chain){
    if(!cfg_infos.debug) return;
    printf("\n");
    print_debug("Lexed node chain :\n");
    for (token_node_t *node = tk_chain->first; node != NULL; node = node->next){
        
        switch (node->token)
        {
        case TOKEN_WORD:
            printf(BOLD_ORANGE);
            printf("WORD");
            printf(RESET);
            printf(" - segment chain : ");
            print_segment_chain(node->first_seg);
            printf("\n");
            break;

        case TOKEN_PIPE:
            printf(BOLD_ORANGE);
            printf("PIPE\n");
            printf(RESET);
            break;

        case TOKEN_REDIR_IN:
            printf(BOLD_ORANGE);
            printf("REDIR IN\n");
            printf(RESET);
            break;

        case TOKEN_REDIR_OUT:
            printf(BOLD_ORANGE);
            printf("REDIR OUT\n");
            printf(RESET);
            break;

        case TOKEN_HEREDOC:
            printf(BOLD_ORANGE);
            printf("HEREDOC\n");
            printf(RESET);
            break;

        case TOKEN_APPEND:
            printf(BOLD_ORANGE);
            printf("APPEND\n");
            printf(RESET);
            break;

        case TOKEN_AND:
            printf(BOLD_ORANGE);
            printf("AND\n");
            printf(RESET);
            break;

        case TOKEN_OR:
            printf(BOLD_ORANGE);
            printf("OR\n");
            printf(RESET);
            break;

        case TOKEN_SEQ:
            printf(BOLD_ORANGE);
            printf("SEQ\n");
            printf(RESET);
            break;
        
        case TOKEN_AMP:
            printf(BOLD_ORANGE);
            printf("AMP\n");
            printf(RESET);
            break;

        default:
            break;
        }
    }
    printf("\n");
}


static void print_depth(int depth){
    for(int i = 0; i < depth; i++){
        printf("\t");
    }
}

void print_ast(ast_node_t *ast, int depth){
    if(!cfg_infos.debug) return;
    if(!ast) return;

    print_ast(ast->left, depth + 1);

    if(ast->token == TOKEN_WORD){ // command node

        printf(BOLD_ORANGE);
        print_depth(depth); printf(" -- COMMAND NODE -- \n");
        print_depth(depth); printf("COMMAND :");
        if(ast->argv->first){
            print_segment_chain(ast->argv->first->seg_chain);
        } 
        printf("\n");
       
        
        print_depth(depth); printf("ARGUMENTS : ");
        printf(RESET);
        int n = 1;
        if(ast->argv->first){
            arg_t *arg = ast->argv->first->next;
            while(arg != NULL){
                printf("  (%d)",n);
                print_segment_chain(arg->seg_chain);
                arg = arg->next;
                n ++;
            }
        }
        printf("\n");

        printf(BOLD_ORANGE);
        print_depth(depth); printf("REDIRS : ");
        printf(RESET);
        if(ast->redirs){
            redir_t *red = ast->redirs->first;
            while(red != NULL){
                printf("    ");
                printf("%s : target : ", token_to_str(red->type));

                print_segment_chain(red->target);

                red = red->next;
            }
        }
        printf("\n");
    }
    else {  // Operator node    
        print_depth(depth);
        printf("%s\n", token_to_str(ast->token));
    }   

    print_ast(ast->right, depth + 1);
}