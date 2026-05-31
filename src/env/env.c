#include "env.h"
#include "env_internal.h"


void free_env(env_t env){
    if(!env) return; 

    while(env != NULL){
        env_t cache = env->next;
        free_env_var(env);
        env = cache;
    }
}


int env_export(env_t *env, char *var_name, char *var_value){
   if(!var_name) return 1;

   env_t cur_var = *env;
   env_t prev_var = NULL;

   while(cur_var != NULL){

        // Found variable, update value
        if(strcmp(cur_var->name, var_name) == 0){    
            
            free(cur_var->value);
            if(var_value){
                cur_var->value = strdup(var_value);
                if(!cur_var->value) return 1;
            } else cur_var->value = NULL;

            if(strcmp(var_name, "ERRLOG") == 0){
                time_t t = time(NULL);
                struct tm *tm = localtime(&t);
                char err_time[64];
                strftime(err_time, sizeof(err_time), "%Y-%m-%d %H:%M:%S", tm);
                env_export(env, "ERRTIME", err_time);
            }
            
            return 0;
        }
        
        prev_var = cur_var;
        cur_var = cur_var->next;
   }
   
   // Variable not found, adding variable to env
   env_t new_var = init_env_var(var_name, var_value);
   if(!new_var) return 1;

   if(prev_var) prev_var->next = new_var;
   else{
        *env = new_var;
   }

   return 0;
}


int env_unset(env_t *env, char *var){
    if(!var) return -1;

    env_t cur_var = *env;
    env_t prev_var = NULL;

    while(cur_var != NULL){

        if(strcmp(cur_var->name, var) == 0){ // Remove variable

            if(prev_var) prev_var->next = cur_var->next;
            else *env = cur_var->next;
        
            free_env_var(cur_var);
            return 0;
        }

        prev_var = cur_var;
        cur_var = cur_var->next;
    }

    // Variable not found
    return 0;
}


char *expand_var(env_t *env, char *var){
    if(!env || !var) return NULL;

    env_t cur_var = *env;
    while(cur_var != NULL){

        if(strcmp(cur_var->name, var) == 0){
            
            char *res = strdup(cur_var->value);
            if(!res) return NULL;
            return res;
        }

        cur_var = cur_var->next;
    }

    return NULL;
}


void free_env_array(char **env_arr){

    size_t i = 0;
    while(env_arr[i] != NULL){
        free(env_arr[i]);
        i ++;
    }
    free(env_arr);
}


char **env_chain_to_array(env_t *env){
    if(!env) return NULL;

    size_t var_count = env_var_count(env);

    char **env_array = (char**)calloc(var_count + 1, sizeof(char*));
    if(!env_array) return NULL;

    env_array[var_count] = NULL;    // sentinel

    env_t cur_var = *env;
    for (size_t i = 0; i < var_count; i++){

        env_array[i] = format_env_var(cur_var);
        if(!env_array[i]){
            free_env_array(env_array);
            return NULL;
        }

        cur_var = cur_var->next;
    }
    
    return env_array;
}


int env_array_to_chain(char **env_arr, env_t *env){
    
    if(!env_arr || !env) return -1;
    
    for (size_t i = 0; env_arr[i] != NULL; i++){
        
        char *sep = strchr(env_arr[i], '=');
        if(!sep) return -1;

        char *name = strndup(env_arr[i], sep - env_arr[i]);
        char *value = strdup(sep + 1);

        int res = env_export(env, name, value);
        free(name);
        free(value);

        if(res != 0) return -1;
    }
    
    return 0;
}