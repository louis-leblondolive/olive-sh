#include "token_chain.h"

//  ----- SEGMENT OPERATIONS -----------------------------------------------

static segment_t *init_segment(void){
    segment_t *res = (segment_t*)malloc(sizeof(segment_t));
    if(!res) return NULL;

    res->type = SEG_LITERAL;           // segment is literal by default
    res->next = NULL;
    res->value[0] = '\0';
    return res;
}


void free_segment_chain(segment_t *chain){
    if(!chain) return;
    while(chain != NULL){
        segment_t *cache = chain->next;
        free(chain);
        chain = cache;
    }
    return;
}


void free_node_segment_chain(token_node_t *node){

    if(!node) return;

    while(node->first_seg != NULL){
        segment_t *cache = node->first_seg->next;
        free(node->first_seg);
        node->first_seg = cache;
    }
    
    node->last_seg = NULL;
}


int add_segment(token_node_t *node){

    if(!node) return -1;

    segment_t *new = init_segment();
    if(!new) return -1;

    if(node->first_seg == NULL){
        node->first_seg = new;
        node->last_seg = new;
    }
    else {
        if(!node->last_seg) return -1;
        node->last_seg->next = new;
        node->last_seg = new;
    }

    return 0;
}


void clean_node_segment_chain(token_node_t *node){
    if(!node || !node->first_seg) return;

    // Remove empty leading segments
    while(node->first_seg && node->first_seg->value[0] == '\0'){
        segment_t *cache = node->first_seg->next;
        free(node->first_seg);
        node->first_seg = cache;
    }

    // Remove empty segments in the rest of the chain.
    if(!node->first_seg) return;
    segment_t *prev = node->first_seg;
    segment_t *cur = node->first_seg->next;

    while(cur != NULL){
        if(cur->value[0] == '\0'){     // empty segment

            segment_t *cache = cur->next;
            free(cur);
            prev->next = cache;
            cur = cache;
        }   
        else{
            prev = cur;
            cur = cur->next;
        }
    }
}


//  ----- TOKEN CHAINS OPERATIONS -----------------------------------------------

char *token_to_str(token_e token){
    switch(token){
        case TOKEN_WORD: return "Word"; 
        case TOKEN_PIPE: return "'|'";     
        case TOKEN_REDIR_IN: return "'<'";
        case TOKEN_REDIR_OUT: return "'>'";
        case TOKEN_HEREDOC: return "'<<'";   
        case TOKEN_APPEND: return "'>>'";    
        case TOKEN_AND: return "'&&'";   
        case TOKEN_OR:  return "'||'";     
        case TOKEN_SEQ: return "';'";
        default: return "";
    }
}

static token_node_t *init_token_node(void){
    token_node_t *res = (token_node_t*)malloc(sizeof(token_node_t));
    if(!res) return NULL;
    res->first_seg = NULL;
    res->last_seg = NULL;
    res->prev = NULL;
    res->next = NULL;
    return res;
}


token_chain_t *init_token_chain(void){

    token_chain_t *tk_chain = (token_chain_t*)malloc(sizeof(token_chain_t));
    if(!tk_chain) return NULL;

    tk_chain->first = NULL;
    tk_chain->last = NULL;

    return tk_chain;
}



void free_token_chain(token_chain_t *tk_chain){

    if(!tk_chain) return;

    token_node_t *cur = tk_chain->first;
    
    while(cur != NULL){

        token_node_t *cache = cur->next;
        free_node_segment_chain(cur);
        free(cur);
        cur = cache;
    }

    free(tk_chain);
}


int add_token_node(token_chain_t *tk_chain){

    if(!tk_chain) return -1;

    token_node_t *new = init_token_node();
    if(!new) return -1;

    if(tk_chain->first == NULL){
        tk_chain->first = new;
        tk_chain->last = new;
    }
    else {
        if(!tk_chain->last) return -1;
        tk_chain->last->next = new;
        tk_chain->last = new;
    }

    return 0;
}


token_node_t *token_chain_pop(token_chain_t *tk_chain){

    if(!tk_chain || !tk_chain->first) return NULL;

    token_node_t *res = tk_chain->first;

    if(tk_chain->first == tk_chain->last) tk_chain->last = NULL;        

    tk_chain->first = res->next;

    if(tk_chain->first) tk_chain->first->prev = NULL; 

    res->next = NULL;
    res->prev = NULL;
    return res;
}