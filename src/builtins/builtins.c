#include "builtins.h"
#include <stdio.h>

builtin_t builtins[] = {
    {"echo", builtin_echo},
    {"export", builtin_export},
    {"unset", builtin_unset},
    {NULL, NULL}
} ;