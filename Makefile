CC = gcc
CFLAGS = -Iinc -lraylib -lm -lGL -lrt -ldl -lX11 -lvterm -DLINUX

DEBUG_FLAGS = -g
RELEASE_FLAGS = -O0

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/froggy-frag
ICONDIR = $(PREFIX)/share/icons/hicolor
DESKTOPDIR = $(PREFIX)/share/applications

SRC_DIR = src
OUT_DIR = bin
TEST_DIR = tests
RES_DIR = bin/resources

TARGET = froggy-frag
TEST_TARGET = $(OUT_DIR)/$(TARGET)_test

SRCS = 	$(SRC_DIR)/main.c \
		$(SRC_DIR)/cJSON.c \
		$(SRC_DIR)/conf.c \
		$(SRC_DIR)/animation.c \
		$(SRC_DIR)/callback.c \
		$(SRC_DIR)/ease.c \
		$(SRC_DIR)/gui.c \
		$(SRC_DIR)/button.c \
		$(SRC_DIR)/terminal.c

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

# Install target
install: $(OUT_DIR)/$(TARGET)
	@echo "Installing froggy-frag..."
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(DATADIR)/shaders
	install -d $(DESTDIR)$(DATADIR)/images
	install -d $(DESTDIR)$(DATADIR)/fonts
	install -d $(DESTDIR)$(ICONDIR)/64x64/apps
	install -d $(DESTDIR)$(ICONDIR)/128x128/apps
	install -d $(DESTDIR)$(DESKTOPDIR)
	install -m 755 $(OUT_DIR)/$(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -m 644 $(RES_DIR)/shaders/*.glsl $(DESTDIR)$(DATADIR)/shaders/ 2>/dev/null || true
	install -m 644 $(RES_DIR)/images/* $(DESTDIR)$(DATADIR)/images/ 2>/dev/null || true
	install -m 644 $(RES_DIR)/fonts/* $(DESTDIR)$(DATADIR)/fonts/ 2>/dev/null || true
	install -m 644 $(RES_DIR)/FroggyOutlined64px.png $(DESTDIR)$(ICONDIR)/64x64/apps/froggy-frag.png 2>/dev/null || true
	install -m 644 $(RES_DIR)/FroggyOutlined128px.png $(DESTDIR)$(ICONDIR)/128x128/apps/froggy-frag.png 2>/dev/null || true
	install -m 644 froggy-frag.desktop $(DESTDIR)$(DESKTOPDIR)/froggy-frag.desktop
	@echo "Installation complete."
	@echo "Binary: $(DESTDIR)$(BINDIR)/$(TARGET)"
	@echo "Data:   $(DESTDIR)$(DATADIR)"

# Uninstall target
uninstall:
	@echo "Uninstalling froggy-frag..."
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -rf $(DESTDIR)$(DATADIR)
	rm -f $(DESTDIR)$(ICONDIR)/64x64/apps/froggy-frag.png
	rm -f $(DESTDIR)$(ICONDIR)/128x64/apps/froggy-frag.png
	rm -f $(DESTDIR)$(DESKTOPDIR)/froggy-frag.desktop
	@echo "Uninstallation complete."

.PHONY: all clean dbg release install uninstall test
