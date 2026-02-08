# Keikaku Programming Language - Makefile
#
# "All proceeds according to plan."

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -I../include
LDFLAGS = -lm

# Debug build
DEBUG_CFLAGS = -Wall -Wextra -std=c11 -g -O0 -DDEBUG -I../include

# Source files
SOURCES = main.c lexer.c parser.c ast.c interpreter.c
OBJECTS = $(SOURCES:.c=.o)
DEBUG_OBJECTS = $(SOURCES:.c=.debug.o)

# Output
TARGET = keikaku
DEBUG_TARGET = keikaku-debug

# Build directory
BUILD_DIR = ../build

.PHONY: all clean debug install test

all: $(BUILD_DIR)/$(TARGET)

debug: $(BUILD_DIR)/$(DEBUG_TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "  ✓ Build complete. The keikaku has been compiled."

$(BUILD_DIR)/$(DEBUG_TARGET): $(DEBUG_OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(DEBUG_OBJECTS) -o $@ $(LDFLAGS)
	@echo "  ✓ Debug build complete."

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.debug.o: %.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(DEBUG_OBJECTS)
	rm -f $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(DEBUG_TARGET)
	@echo "  ✓ Cleanup complete."

install: $(BUILD_DIR)/$(TARGET)
	sudo cp $(BUILD_DIR)/$(TARGET) /usr/local/bin/
	@echo "  ✓ Installed to /usr/local/bin/keikaku"

test: $(BUILD_DIR)/$(TARGET)
	@echo "Running tests..."
	@cd ../tests && ./run_tests.sh

# Dependencies
main.o: main.c lexer.h parser.h ast.h interpreter.h
lexer.o: lexer.c lexer.h
parser.o: parser.c parser.h lexer.h ast.h
ast.o: ast.c ast.h
interpreter.o: interpreter.c interpreter.h ast.h
