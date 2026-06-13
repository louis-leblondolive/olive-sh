#include "builtins.h"

#include <stdlib.h>
#include "env.h"


int builtin_olvsh(int argc, char **argv, env_t *env){

    if(argc <= 1) return 0;

    for (int i = 1; i < argc; i++){

        if(strncmp("--", argv[i], 2) == 0){
            // option
            bool enable_mode = true;
            char *opt;

            if(strncmp("--no-", argv[i], 5) == 0){
                enable_mode = false;
                opt = argv[i] + 5;
            } else opt = argv[i] + 2;

            if(strncmp(opt, "hints", MAX_WORD_LENGTH) == 0) cfg_infos.hints = enable_mode;
            else if(strncmp(opt, "debug", MAX_WORD_LENGTH) == 0) cfg_infos.debug = enable_mode;
            else if(strncmp(opt, "errlog", MAX_WORD_LENGTH) == 0) cfg_infos.errlog = enable_mode;
            else if(strncmp(opt, "pipefail", MAX_WORD_LENGTH) == 0) cfg_infos.pipefail = enable_mode;
            else if(strncmp(opt, "errexit", MAX_WORD_LENGTH) == 0) cfg_infos.errexit = enable_mode;
            else if(strncmp(opt, "nounset", MAX_WORD_LENGTH) == 0) cfg_infos.nounset = enable_mode;

            else {
                env_export(env, "ERRLOG", "olive-sh: no such option : --%s", opt);
                return 1;
            }
        }
    }

    return 0;
}