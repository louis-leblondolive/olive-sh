#include "builtins.h"
#include <stdio.h>

builtin_t builtins[] = {
    {"echo", builtin_echo},
    {NULL, NULL}
} ;