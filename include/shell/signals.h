#ifndef SIGNALS
#define SIGNALS

#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

extern sigjmp_buf jump_buffer;
extern volatile sig_atomic_t jump_active;  

int init_shell_sa_handlers(void);
int reset_sa_handlers(void);

#endif