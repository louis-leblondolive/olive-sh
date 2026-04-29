# --- Compiler Configuration ---
CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g -std=c11
LDFLAGS = -lreadline

# --- Directories ---
SRC_DIR     = src
BIN_DIR     = bin
OBJ_DIR     = obj

# --- Include Paths ---
INC_DIRS    = include \
			  include/builtins \
			  include/ds \
			  include/env \
			  include/shell \
			  include/utils \
			  src/builtins \
			  src/ds \
			  src/env \
              src/shell/lexer \
              src/shell/parser \
			  src/shell/executor \
              src/utils

INCS        = $(addprefix -I, $(INC_DIRS))

# --- Source Discovery ---
SRCS        = $(shell find $(SRC_DIR) -name "*.c")
OBJS        = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- Binary Output ---
MAIN_BIN    = $(BIN_DIR)/shell_main

# --- Primary Rules ---
all: $(BIN_DIR) $(OBJ_DIR) $(MAIN_BIN)

$(MAIN_BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

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