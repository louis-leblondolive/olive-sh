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


void sig_chld_handler(int s){
    (void)s;

    job_table_t *job_tbl = get_main_job_table();

    pid_t done_id = -1;
    while((done_id = waitpid(-1, NULL, WNOHANG))){
        
        if(!job_tbl || !job_tbl->tbl) continue;
        for (size_t i = 1; i < job_tbl->capacity; i++){
            if(job_tbl->tbl[i]->leader_pid == done_id) job_tbl->tbl[i]->status = DONE;
        }
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
    if(sigaction(SIGTTIN, &sa_ignore, NULL) == -1) return 1;
    if(sigaction(SIGTTOU, &sa_ignore, NULL) == -1) return 1;

    // Handle SIGINT
    struct sigaction sa_int;
    sa_int.sa_flags = SA_RESTART;
    sa_int.sa_handler = sig_int_handler;
    sigemptyset(&sa_int.sa_mask);
    if(sigaction(SIGINT, &sa_int, NULL) == -1) return 1;

    // Handle SIGCHLD
    struct sigaction sa_chld;
    sa_chld.sa_flags = SA_RESTART;
    sa_chld.sa_handler = sig_chld_handler;
    sigemptyset(&sa_chld.sa_mask);
    if(sigaction(SIGCHLD, &sa_chld, NULL) == -1) return 1;

    return 0;
}


int reset_sa_handlers(void){

    if(signal(SIGINT, SIG_DFL) == SIG_ERR) return 1;
    if(signal(SIGQUIT, SIG_DFL) == SIG_ERR) return 1;
    if(signal(SIGTSTP, SIG_DFL) == SIG_ERR) return 1;
    if(signal(SIGTTIN, SIG_DFL) == SIG_ERR) return 1;
    if(signal(SIGTTOU, SIG_DFL) == SIG_ERR) return 1;

    return 0;
}


sigset_t block_sigchld(void){
    sigset_t block_mask, old_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCHLD);

    sigprocmask(SIG_BLOCK, &block_mask, &old_mask);
    return old_mask;
}


void restore_sigmask(sigset_t old_mask){
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
}