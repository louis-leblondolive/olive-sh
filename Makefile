# --- Compiler Configuration ---
CC      = gcc
READLINE_DIR = /opt/homebrew/opt/readline
CFLAGS  = -Wall -Wextra -Werror -g -std=c11 -I$(READLINE_DIR)/include -D_POSIX_C_SOURCE=200809L
LDFLAGS = -L$(READLINE_DIR)/lib -lreadline

# --- Debug Configuration ---
DEBUG_CFLAGS  = $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -O0
DEBUG_LDFLAGS = $(LDFLAGS) -fsanitize=address,undefined

# --- Directories ---
SRC_DIR       = src
BIN_DIR       = bin
OBJ_DIR       = obj
OBJ_DIR_DEBUG = obj-debug

# --- Include Paths ---
INC_DIRS    = include \
              include/builtins \
              include/ds \
              include/env \
              include/shell \
              include/utils \
              src/builtins \
              src \
              src/ds \
              src/env \
              src/shell/signals \
              src/shell/lexer \
              src/shell/parser \
              src/shell/executor \
              src/utils

INCS        = $(addprefix -I, $(INC_DIRS))

# --- Source Discovery ---
SRCS        = $(shell find $(SRC_DIR) -name "*.c")
OBJS        = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_DEBUG  = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR_DEBUG)/%.o, $(SRCS))

# --- Binary Output ---
MAIN_BIN       = $(BIN_DIR)/olvsh
MAIN_BIN_DEBUG = $(BIN_DIR)/olvsh-debug

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

# --- Debug Rules ---
debug: $(BIN_DIR) $(OBJ_DIR_DEBUG) $(MAIN_BIN_DEBUG)

$(MAIN_BIN_DEBUG): $(OBJS_DEBUG)
	$(CC) $(DEBUG_CFLAGS) $(OBJS_DEBUG) -o $@ $(DEBUG_LDFLAGS)

$(OBJ_DIR_DEBUG)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_CFLAGS) $(INCS) -c $< -o $@

$(OBJ_DIR_DEBUG):
	mkdir -p $(OBJ_DIR_DEBUG)

# --- Cleanup ---
clean:
	rm -rf $(OBJ_DIR) $(OBJ_DIR_DEBUG)

fclean: clean
	rm -rf $(BIN_DIR)

re: fclean all

.PHONY: all clean fclean re debug