CC=clang
CFLAGS=-Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -g

SRC_DIR=src
BUILD_DIR=build
SRC_FILES=$(wildcard $(SRC_DIR)/**/*.c $(SRC_DIR)/*.c)
OBJ_FILES=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

HDR_DIR=include
HDR_FILES=$(wildcard $(HDR_DIR)/**/*.h $(HDR_DIR)/*.h)

all: $(BUILD_DIR)/main
	cp $(BUILD_DIR)/main coolc

$(BUILD_DIR)/main: $(OBJ_FILES) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HDR_FILES) | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(HDR_DIR) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
	rm -f coolc

examples: all
	./coolc examples/gol.cl --module prelude -o build/gol
	./coolc examples/rule110.cl --module prelude --module data -o build/rule110
	./coolc examples/raylib.cl --module raylib -o build/raylib
	./coolc examples/game.cl --module raylib -o build/game
	./coolc examples/snake.cl --module raylib --module random -o build/snake
	./coolc examples/server.cl --module prelude -o build/server
	./coolc examples/threading.cl --module threading -o build/threading

game: all
	./coolc examples/multi/server.cl examples/multi/message.cl --module threading --module data --module random -o build/game-server
	./coolc examples/multi/client.cl examples/multi/message.cl --module threading --module raylib -o build/game-client

dist: clean all
	rm -rf coolc.tar.gz
	tar -czf coolc.tar.gz coolc lib


.PHONY: all clean examples dist
