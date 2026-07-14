#include "exec_result.h"



exec_res_t exec_res_from_builtin(int exit_code){

    exec_res_t res;
    res.exit_code = exit_code;
    res.kind = RES_EXITED;

    return res;
}


exec_res_t exec_res_from_waitpid_status(int status){

    exec_res_t res;
    
    if(WIFSIGNALED(status)){
        res.kind = RES_SIGNALED;
        res.term_sig = WTERMSIG(status);
    }

    else if(WIFSTOPPED(status)){
        res.kind = RES_STOPPED;
        res.stop_sig = WSTOPSIG(status);
    }

    else {
        res.kind = RES_EXITED;
        res.exit_code = WEXITSTATUS(status);
    }

    return res;
}