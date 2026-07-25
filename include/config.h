#ifndef CONFIG
#define CONFIG

#include <stdbool.h>

/**
 * @file config.h
 * @brief Global shell configuration and runtime options.
 */

#define MAX_WORD_LENGTH 2048    /** Maximum length of a single word. */
#define MAX_ERROR_LEN 4096      /** Maximum length of an error message. */


/**
 * @brief Runtime configuration flags for the shell. 
 */
typedef struct config_infos_s {

    // verbosity parameters
    bool debug;     /** Enables internal debug output. */
    bool hints;     /** Enables hints output. */
    bool errlog;    /** Enable display of error details. */
    bool xtrace;    /** Echoes each command before execution. */
   
    // error behaviour 
    bool pipefail;  /** Pipeline error status reflects the first failing command. */
    bool errexit;   /** Shell exits on any command failure. */
    bool nounset;   /** Treats unset variable expansion as an error. */

    // tty detection 
    bool interactive; /** True if the shell is interactive */
    
} config_infos_t;

/**
 * @brief Global variable holding the shell configuration. 
 */
extern config_infos_t cfg_infos;

#endif