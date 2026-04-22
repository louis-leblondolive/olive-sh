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

#include "token_list.h"


// Prints an error in red 
void print_error(char *format, ...);

// Prints an info and adds the "[INFO]" beacon
void print_info(char *format, ...);

// Prints an info and adds the "[DEBUG]" beacon
void print_debug(char *format, ...);

// Prints a token chained list
void print_token_chain(token_chain_t *tk_chain);


#endif