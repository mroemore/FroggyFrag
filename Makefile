CC = zig cc
CFLAGS = -Iinc -lraylib -lm -Llib -lGL -lrt -ldl -lX11 -DLINUX

DEBUG_FLAGS = -g

SRC_DIR = src
OUT_DIR = bin

TARGET = ani

SRCS = 	$(SRC_DIR)/main.c
OBJS = $(SRCS:.c=.o)

all: CFLAGS += $(DEBUG_FLAGS)
all: $(OUT_DIR)/$(TARGET)

dbg: all
dbg:
	(cd bin/ && chmod +x $(TARGET) && gdb -ex "run" $(TARGET))

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
	rm -f $(OBJS) $(OUT_DIR)/$(TARGET)
