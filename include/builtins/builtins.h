#ifndef BUILTINS
#define BUILTINS

#include "env.h"

typedef struct builtin_s {
    char *name;
    int (*func)(int, char **, env_t *);

} builtin_t;

extern builtin_t builtins[];

int builtin_echo(int argc, char **argv, env_t *env);
int builtin_export(int argc, char **argv, env_t *env);
int builtin_unset(int argc, char **argv, env_t *env);


#endif