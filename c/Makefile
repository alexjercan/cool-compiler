CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -g
BUILD_DIR=build

SRC_DIR=src
HDR_DIR=$(SRC_DIR)/include
HDR_FILES=$(wildcard $(HDR_DIR)/*.h)
SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))
DEP_DIR=include
DEP_FILES=$(wildcard $(DEP_DIR)/*.h)

all: $(BUILD_DIR)/main

$(BUILD_DIR)/main: $(OBJ_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HDR_FILES) $(DEP_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(HDR_DIR) -I$(DEP_DIR) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/main
	./$< $(ARGS)

.PHONY: all clean run
