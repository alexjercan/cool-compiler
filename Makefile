CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -g

SRC_DIR=src
BUILD_DIR=build
SRC_FILES=$(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

HDR_DIR=include
LIB_DIR=lib
HDR_FILES=$(wildcard $(HDR_DIR)/*.h) $(wildcard $(LIB_DIR)/*.h)

all: $(BUILD_DIR)/main

$(BUILD_DIR)/main: $(OBJ_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HDR_FILES) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(HDR_DIR) -I$(LIB_DIR) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/main
	./$< $(ARGS)

.PHONY: all clean run
