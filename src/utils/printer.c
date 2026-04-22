#include "printer.h"


#include "printer.h"


void print_error(char *format, ...){
    va_list args;

    fprintf(stderr, BOLD_RED);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, RESET);
}

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
    va_list args;

    printf(BOLD_GREEN);
    printf("[DEBUG] ");
    printf(RESET);

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}


void print_token_chain(token_chain_t *tk_chain){

    printf("\n");
    print_debug("Lexed node chain :\n");
    for (token_node_t *node = tk_chain->first; node != NULL; node = node->next){
        
        switch (node->token)
        {
        case TOKEN_WORD:
            printf(BOLD_ORANGE);
            printf("WORD");
            printf(RESET);
            printf(" - segment chain :\n");
            for(segment_t *seg = node->first_seg; seg != NULL; seg = seg->next){
                printf("  ");
                if(seg->type == SEG_LITTERAL) printf("(LITTERAL)");
                else printf("(VAR)");
                printf("[%s]", seg->value);
            }
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
        
        default:
            break;
        }
    }
    printf("\n");
}