#include "env.h"
#include "env_internal.h"


env_t init_env_var(char *name, char *value){

    env_t var = (env_t)malloc(sizeof(env_var_t));
    if(!var) return NULL;

    var->next = NULL;

    var->name = strdup(name);
    if(!var->name){
        free(var);
        return NULL;
    }

    var->value = strdup(value);
    if(!var->value){
        free(var->name);
        free(var);
        return NULL;
    }

    return var;
}


void free_env_var(env_t var){
    if(!var) return;

    if(var->name) free(var->name);
    if(var->value) free(var->value);
    free(var);
}


size_t env_var_count(env_t env){
    size_t count = 0;

    while(env != NULL){
        count ++;
        env = env->next;
    }
    return count; 
}


char *format_env_var(env_t var){
    if(!var || !var->name || !var->value) return NULL;

    size_t name_size = strlen(var->name);
    size_t value_size = strlen(var->value);
    size_t res_size = name_size + value_size + 2;   // Counting '=' and final '\0'

    char *res = (char*)malloc(sizeof(char) * res_size);
    if(!res) return NULL;

    for (size_t i = 0; i < name_size; i++){
       res[i] = var->name[i];
    }

    res[name_size] = '=';

    for (size_t i = 0; i < value_size; i++){
        res[name_size + 1 + i] = var->value[i];
    }
    
    res[res_size - 1] = '\0';
    
    return res;
}