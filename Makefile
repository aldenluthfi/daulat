# Compiler and flags
CC = gcc
RM = rm -rf

# Directories
SRC_DIR = src
INC_DIR = incl
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/**/*.c)
OBJ_FILES = $(SRC_FILES:%=$(BUILD_DIR)/%.o)

# Targets
TARGET = $(BIN_DIR)/regnum

# Common flags
INC_FLAGS = $(addprefix -I, $(INC_DIR))

# =============================================================================
# Debug mode: symbols, no optimization, assertions enabled
# =============================================================================
debug: CFLAGS += -g -O0 -DDEBUG -Wall -Wextra
debug: CFLAGS += $(INC_FLAGS)
debug: all

# =============================================================================
# Release mode: optimized, no debug info, stripped
# =============================================================================
release: CFLAGS += -O3 -ffast-math -DNDEBUG -Wall
release: CFLAGS += $(INC_FLAGS)
release: all

# =============================================================================
# Build target
# =============================================================================
all: $(BUILD_DIR) $(BIN_DIR) $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@ $(LDFLAGS)

# Pattern rule for object files
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create output directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# =============================================================================
# Clean
# =============================================================================
clean:
	$(RM) $(BUILD_DIR) $(TARGET)

.PHONY: all debug release clean
