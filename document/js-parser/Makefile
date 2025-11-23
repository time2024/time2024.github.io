# Makefile for JavaScript Parser

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
RE2C = re2c
BISON = bison

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests

TARGET = js-parser

# Source files
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/ast.c
GENERATED_SRCS = $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.tab.c

# Object files
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/lexer.o $(BUILD_DIR)/parser.tab.o

.PHONY: all clean test

all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Generate lexer from re2c
$(SRC_DIR)/lexer.c: $(SRC_DIR)/lexer.re
	$(RE2C) -o $@ -8 --input-encoding utf8 $<

# Generate parser from bison
$(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.tab.h: $(SRC_DIR)/parser.y
	$(BISON) -d -v -o $(SRC_DIR)/parser.tab.c $<

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Special rule for generated files
$(BUILD_DIR)/lexer.o: $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.tab.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-unused-variable -Wno-unused-function -c $< -o $@

$(BUILD_DIR)/parser.tab.o: $(SRC_DIR)/parser.tab.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Wno-unused-function -c $< -o $@

# Link executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo ""
	@echo "======================================"
	@echo "✓ Build successful!"
	@echo "======================================"
	@echo ""

# Run tests
test: $(TARGET)
	@echo ""
	@echo "======================================"
	@echo "Running Positive Tests"
	@echo "======================================"
	@for file in $(TEST_DIR)/positive/*.js; do \
		echo ""; \
		echo "Testing: $$file"; \
		./$(TARGET) $$file || exit 1; \
	done
	@echo ""
	@echo "======================================"
	@echo "Running Negative Tests (should fail)"
	@echo "======================================"
	@for file in $(TEST_DIR)/negative/*.js; do \
		echo ""; \
		echo "Testing: $$file"; \
		./$(TARGET) $$file 2>/dev/null && { echo "ERROR: Should have failed!"; exit 1; } || echo "  ✓ Failed as expected"; \
	done
	@echo ""
	@echo "======================================"
	@echo "✓ All tests passed!"
	@echo "======================================"
	@echo ""

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	rm -f $(SRC_DIR)/lexer.c $(SRC_DIR)/parser.tab.c $(SRC_DIR)/parser.tab.h
	rm -f $(SRC_DIR)/parser.output
	@echo "Clean complete"

help:
	@echo "JavaScript Parser - Makefile Help"
	@echo ""
	@echo "Available targets:"
	@echo "  make          - Build the parser"
	@echo "  make test     - Run all tests"
	@echo "  make clean    - Remove build files"
	@echo "  make help     - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  ./js-parser <javascript_file>"
	@echo "  ./js-parser -v <javascript_file>  (verbose mode)"
