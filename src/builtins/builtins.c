#include "builtins.h"
#include <stdio.h>

builtin_t builtins[] = {
    {"echo", builtin_echo},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {"env", builtin_env},
    {"pwd", builtin_pwd},
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"errlog", builtin_errlog},
    {"olvsh", builtin_olvsh},
    {NULL, NULL}
} ;