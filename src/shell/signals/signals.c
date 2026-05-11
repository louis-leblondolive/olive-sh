#include "signals.h"


sigjmp_buf jump_buffer;
volatile sig_atomic_t jump_active = 0;  


void sig_int_handler(int s){
    (void)s;
    write(STDOUT_FILENO, "\n", 1);
    if(jump_active){
        siglongjmp(jump_buffer, 1);
    }
}


int init_shell_sa_handlers(void){

    // Signals to ignore
    struct sigaction sa_ignore;
    sa_ignore.sa_flags = SA_RESTART;
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);

    if(sigaction(SIGQUIT, &sa_ignore, NULL) == -1) return 1;

    if(sigaction(SIGTSTP, &sa_ignore, NULL) == -1) return 1;


    // Handle SIGINT
    struct sigaction sa_int;
    sa_int.sa_flags = SA_RESTART;
    sa_int.sa_handler = sig_int_handler;
    sigemptyset(&sa_int.sa_mask);
    if(sigaction(SIGINT, &sa_int, NULL) == -1) return 1;

    return 0;
}


int reset_sa_handlers(void){

    if(signal(SIGINT, SIG_DFL) == SIG_ERR) return 1;
    if(signal(SIGQUIT, SIG_DFL) == SIG_ERR) return 1;
    if(signal(SIGTSTP, SIG_DFL) == SIG_ERR) return 1;

    return 0;
}