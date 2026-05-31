#ifndef CONFIG
#define CONFIG

#include <stdbool.h>

#define MAX_WORD_LENGTH 2048
#define MAX_ERROR_LEN 4096


typedef struct config_infos_s {

    // verbosity parameters
    bool debug;
    bool hints;
    bool errlog;

} config_infos_t;

extern config_infos_t cfg_infos;

#endif