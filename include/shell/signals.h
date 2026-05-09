#ifndef SIGNALS
#define SIGNALS

#include <signal.h>
#include <unistd.h>
#include <errno.h>

int init_shell_sa_handlers(void);
int reset_sa_handlers(void);

#endif