# Makefile
# Project: H1C (Http/1.x) Server
# Derek Tan

# compiler vars
CC := clang -std=c11
CFLAGS := -g -Wall -Werror -O0

# executable dir
BIN_DIR := ./bin

# build files dir
BUILD_DIR := ./build

# implementation code dir
SRC_DIR := ./src

# header include dir
HEADER_DIR := ./include

# auto generate object file target names
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# executable generate path
EXE := $(BIN_DIR)/h1cserver_c

vpath %.c $(SRC_DIR)

.PHONY: tell all clean

# utility rule: show SLOC
sloc:
	@wc -l $(HEADER_DIR)/**/*.h $(SRC_DIR)/*.c

# debug rule: show all targets and deps
tell:
	@echo "Code:"
	@echo $(SRCS)
	@echo "Objs:"
	@echo $(OBJS)

# build rules: compiles code and links object files into executable
all: $(EXE)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -I$(HEADER_DIR) -o $@

# clean rule: only remove old executables!
clean:
	rm -f $(EXE)
