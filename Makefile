# --- Compiler Configuration ---
CC      = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c11
LDFLAGS = -lreadline
# Include paths so you can use #include "header.h" instead of relative paths
INCS    = -Isrc -Isrc/shell -Isrc/utils -Isrc/builtins

# --- Directories ---
SRC_DIR     = src
BIN_DIR     = bin
OBJ_DIR     = obj
BUILTIN_DIR = src/builtins

# --- Source Discovery ---
# Find all core logic files (lexer, parser, utils, etc.) excluding main and builtins
CORE_SRCS = $(shell find src/shell src/utils -name "*.c")
CORE_OBJS = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(CORE_SRCS))

# Main shell entry point
MAIN_SRC = src/main.c
MAIN_OBJ = $(OBJ_DIR)/main.o
MAIN_BIN = $(BIN_DIR)/shell_main

# Individual builtins (one binary per .c file)
BUILTIN_SRCS = $(wildcard $(BUILTIN_DIR)/*.c)
BUILTIN_BINS = $(patsubst $(BUILTIN_DIR)/%.c, $(BIN_DIR)/%, $(BUILTIN_SRCS))

# --- Primary Rules ---
all: $(BIN_DIR) $(OBJ_DIR) $(MAIN_BIN) $(BUILTIN_BINS)

# Link the main shell executable
$(MAIN_BIN): $(MAIN_OBJ) $(CORE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Compile each builtin into its own standalone binary
# These are linked with CORE_OBJS in case they need your utility functions
$(BIN_DIR)/%: $(BUILTIN_DIR)/%.c $(CORE_OBJS)
	$(CC) $(CFLAGS) $(INCS) $^ -o $@ $(LDFLAGS)

# Pattern rule for object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

# Create necessary directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# --- Cleanup ---
clean:
	rm -rf $(OBJ_DIR)
fclean: clean
	rm -rf $(BIN_DIR)
re: fclean all
.PHONY: all clean fclean re