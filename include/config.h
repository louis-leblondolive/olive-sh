#ifndef CONFIG
#define CONFIG

#include <stdbool.h>

#define MAX_WORD_LENGTH 2048
#define MAX_ERROR_LEN 2048


typedef struct config_infos_s {

    // verbosity parameters
    bool quiet;
    bool verbose;

} config_infos_t;

extern config_infos_t cfg_infos;

#endif