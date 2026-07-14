#ifndef SIGNALS
#define SIGNALS

#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "job_ctl.h"

extern sigjmp_buf jump_buffer;
extern volatile sig_atomic_t jump_active;  

int init_shell_sa_handlers(void);
int reset_sa_handlers(void);
sigset_t block_sigchld(void);
void restore_sigmask(sigset_t old_mask);

#endif