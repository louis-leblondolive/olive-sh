#include "parser.h"
#include "parser_internal.h"


parse_res_t build_ast(token_chain_t *tk_chain){

    parse_res_t parse_res;
    parse_res.ast = NULL;
    parse_res.success = true;
    parse_res.error = NULL;
    parse_res.error_info = NULL;

    if(!tk_chain) return parse_res; // Empty input

    ast_node_t *cur_ast = build_seq_node(NULL, tk_chain, &parse_res);

    if(!cur_ast || !parse_res.success){    // error while parsing
        return parse_res;      
    }

    parse_res.ast = cur_ast;

    return parse_res;
}