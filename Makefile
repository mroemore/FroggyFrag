CC = gcc
CFLAGS = -Iinc -lraylib -lm -Llib -lGL -lrt -ldl -lX11 -DLINUX

DEBUG_FLAGS = -g
RELEASE_FLAGS = -O0

SRC_DIR = src
OUT_DIR = bin
TEST_DIR = tests

TARGET = froggy-frag
TEST_TARGET = $(OUT_DIR)/$(TARGET)_test

SRCS = 	$(SRC_DIR)/main.c \
		$(SRC_DIR)/cJSON.c \
		$(SRC_DIR)/conf.c \
		$(SRC_DIR)/anim2.c \
		$(SRC_DIR)/callback.c \
		$(SRC_DIR)/ease.c \
		$(SRC_DIR)/gui.c

OBJS = $(SRCS:.c=.o)

# Test sources and objects
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(TEST_SRCS:.c=.o)

all: $(OUT_DIR)/$(TARGET)

dbg: CFLAGS += $(DEBUG_FLAGS)
dbg: all
dbg:
	(cd bin/ && chmod +x $(TARGET) && gdb -ex "run" $(TARGET))

release: CFLAGS += $(RELEASE_FLAGS)
release: all

$(OUT_DIR)/$(TARGET): $(OBJS) | $(OUT_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

# Rule to compile .c files into .o files in the src directory
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Ensure the output directory exists
$(OUT_DIR):
	mkdir -p $(OUT_DIR)

# Clean up object files in the src directory and the target binary
clean:
	rm -f $(OBJS) $(OUT_DIR)/$(TARGET) $(TEST_OBJS) $(TEST_TARGET)

# Test target
test: CFLAGS += -I$(TEST_DIR)
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_OBJS) | $(OUT_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

# Rule to compile test .c files into .o files in the tests directory
$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) -I$(TEST_DIR)
