#include "builtins.h"
#include <stdio.h>

builtin_t builtins[] = {
    {"true", builtin_true},
    {"false", builtin_false},
    {"echo", builtin_echo},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {"env", builtin_env},
    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"errlog", builtin_errlog},
    {"olvsh", builtin_olvsh},
    {"jobs", builtin_jobs},
    {"bg", builtin_bg},
    {"fg", builtin_fg},
    {NULL, NULL}
};