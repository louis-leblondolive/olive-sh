#include "builtins.h"

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "env.h"

int builtin_cd(int argc, char **argv, env_t *env){

    if(!env || !argv) return 1; 
    if(argc < 2) return 0;

    char *path = argv[1];
    
    if(chdir(path) != 0){
        if(errno == EACCES) env_export(env, "ERRLOG", "cd: access denied");
        else if(errno == ELOOP) env_export(env, "ERRLOG", "cd: looping symbolic link");
        else if(errno == ENOENT){
            env_export(env, "ERRLOG", "cd: no such file or directory \"%s\"", path);
        }
        else if(errno == ENOTDIR){
            env_export(env, "ERRLOG", "cd: not a directory \"%s\"", path);
        }
        else env_export(env, "ERRLOG", "cd: internal error");

        return 1;
    }

    return 0;
}