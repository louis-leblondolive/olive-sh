#ifndef PRINTER
#define PRINTER

#define BOLD_RED     "\033[1;31m"
#define BOLD_GREEN   "\033[1;32m"
#define BOLD_ORANGE  "\033[1;33m"
#define BOLD_BLUE    "\033[1;34m"
#define RESET        "\033[0m"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"
#include "token_chain.h"
#include "ast.h"

/**
 * @file printer.h
 * @brief Custom ouput management.
 */

/**
 * @brief Prints an info and adds the "[INFO]" beacon.
 */
void print_info(char *format, ...);

/**
 * @brief Prints an info and adds the "[DEBUG]" beacon.
 */
void print_debug(char *format, ...);

/**
 * @brief Prints a hint and adds the "Hint:" beacon.
 */
void print_hint(char *format, ...);

/**
 * @brief Prints an error to the format : title - description and 
 * adds a "See more details" prompt.
 */
void print_error(char *title, char *description, char *errlog);

/**
 * @brief Prints a token chained list.
 */
void print_token_chain(token_chain_t *tk_chain);

/**
 * @brief Prints an abstract syntax tree.
 */
void print_ast(ast_node_t *ast, int depth);

#endif