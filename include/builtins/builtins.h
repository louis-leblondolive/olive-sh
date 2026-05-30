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
int builtin_env(int argc, char **argv, env_t *env);
int builtin_pwd(int argc, char **argv, env_t *env);
int builtin_exit(int argc, char **argv, env_t *env);
int builtin_errlog(int argc, char **argv, env_t *env);
int builtin_olvsh(int argc, char **argv, env_t *env);

#endif