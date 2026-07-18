#ifndef SIGNALS
#define SIGNALS

#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "job_ctl.h"

/**
 * @file signals.h
 * @brief Signal handlers and management implementation. 
 */

/**
 * @brief Global jump buffer to store program state in case of interruption.
 */
extern sigjmp_buf jump_buffer;

/**
 * @brief Global variable indicating if jump operation is to be used.  
 */
extern volatile sig_atomic_t jump_active;  


/**
 * @brief Initializes the signal handlers used by the shell. 
 * @return 0 on success, 1 otherwise. 
 */
int init_shell_sa_handlers(void);

/**
 * @brief Resets all signal handlers affected by the shell initialization.
 * @return 0 on success, 1 otherwise.  
 */
int reset_sa_handlers(void);

/**
 * @brief Masks SIGCHLD from being delivered to the process.
 * @return The process signal mask as it was before SIGCHLD was blocked. 
 */
sigset_t block_sigchld(void);

/**
 * @brief Restores a signal old_mask. 
 */
void restore_sigmask(sigset_t old_mask);

#endif