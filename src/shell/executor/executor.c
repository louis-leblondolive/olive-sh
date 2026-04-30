#include "executor.h"
#include "exec_internal.h"


int run_ast(env_t *env, ast_node_t *ast){
    if(!ast) return 0;

    int cache_res;
    switch(ast->token){

        case TOKEN_WORD:
            return run_cmd(env, ast);

        case TOKEN_PIPE:
            // pipe here 
            run_ast(env, ast->left);
            return run_ast(env, ast->right);

        case TOKEN_AND:
            cache_res = run_ast(env, ast->left);
            if(cache_res != 0) return cache_res;
            return run_ast(env, ast->right);

        case TOKEN_OR:
            cache_res = run_ast(env, ast->left);
            if(cache_res == 0) return 0;
            return run_ast(env, ast->right);
        
        case TOKEN_SEQ:
            run_ast(env, ast->left);
            return run_ast(env, ast->right);
        
        default:
            return 1;
    }

    return 0;
}