#ifndef BUILTINS
#define BUILTINS

#include "env.h"
#include "exec_result.h"

typedef struct builtin_s {
    char *name;
    exec_res_t (*func)(int, char **, env_t *);

} builtin_t;

extern builtin_t builtins[];

exec_res_t builtin_true(int argc, char **argv, env_t *env);
exec_res_t builtin_false(int argc, char **argv, env_t *env);
exec_res_t builtin_echo(int argc, char **argv, env_t *env);
exec_res_t builtin_export(int argc, char **argv, env_t *env);
exec_res_t builtin_unset(int argc, char **argv, env_t *env);
exec_res_t builtin_env(int argc, char **argv, env_t *env);
exec_res_t builtin_pwd(int argc, char **argv, env_t *env);
exec_res_t builtin_cd(int argc, char **argv, env_t *env);
exec_res_t builtin_exit(int argc, char **argv, env_t *env);
exec_res_t builtin_errlog(int argc, char **argv, env_t *env);
exec_res_t builtin_olvsh(int argc, char **argv, env_t *env);
exec_res_t builtin_jobs(int argc, char **argv, env_t *env);
exec_res_t builtin_bg(int argc, char **argv, env_t *env);
exec_res_t builtin_fg(int argc, char **argv, env_t *env);

#endif