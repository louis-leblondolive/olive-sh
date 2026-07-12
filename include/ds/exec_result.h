#ifndef EXEC_RESULT
#define EXEC_RESULT

typedef enum {
     RES_EXITED,
     RES_SIGNALED, 
     RES_STOPPED
} res_kind_e;

typedef struct exec_res_s {
     res_kind_e kind;
     union {
          int exit_code;
          int term_sig;
          int stop_sig;
     };
} exec_res_t;

#endif