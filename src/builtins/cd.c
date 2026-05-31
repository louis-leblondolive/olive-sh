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
            char err_buf[33 + strlen(path)];
            snprintf(err_buf, sizeof(err_buf), "cd: no such file or directory \"%s\"", path);
            env_export(env, "ERRLOG", err_buf);
        }
        else if(errno == ENOTDIR){
            char err_buff[23 + strlen(path)];
            snprintf(err_buff, sizeof(err_buff), "cd: not a directory \"%s\"", path);
            env_export(env, "ERRLOG", err_buff);
        }
        else env_export(env, "ERRLOG", "cd: internal error");

        return 1;
    }

    return 0;
}