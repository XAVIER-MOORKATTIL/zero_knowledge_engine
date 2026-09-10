CC = gcc
CFLAGS = -Wall -Wextra -O3 -msse2 -Iinclude
SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all clean test harness

all: $(BUILD_DIR) harness

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

harness: $(OBJS) $(TEST_DIR)/test_harness.c
	$(CC) $(CFLAGS) $^ -o $@ -lm

test: harness
	./harness

clean:
	rm -rf $(BUILD_DIR) harness *.db
