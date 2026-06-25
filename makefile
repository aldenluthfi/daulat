# =============================================================================
# Compiler and flags
# =============================================================================
CC = gcc
RM = rm -rf

# Directories
SRC_DIR = src
INC_DIR = incl
BUILD_DIR = build
BIN_DIR = bin

# =============================================================================
# Source files: all C files under the source directory
# =============================================================================
ENGINE_SRC := $(shell find $(SRC_DIR) -name '*.c')

ENGINE_OBJ := $(ENGINE_SRC:%=$(BUILD_DIR)/%.o)

ENGINE_BIN := $(BIN_DIR)/daulat

INC_FLAGS := -I$(INC_DIR)

# =============================================================================
# Modes
# =============================================================================
DEBUG_FLAGS   := -std=c23 -Wall -O0 -DDEBUG -g -Wextra -Werror
RELEASE_FLAGS := -std=c23 -Wall -O3 -DNDEBUG

debug:   CFLAGS_ENGINE := $(DEBUG_FLAGS) $(INC_FLAGS)
debug:   all

release: CFLAGS_ENGINE := $(RELEASE_FLAGS) $(INC_FLAGS)
release: all

# =============================================================================
# Build target
# =============================================================================
all: $(BUILD_DIR) $(BIN_DIR) $(ENGINE_BIN)

$(ENGINE_BIN): $(ENGINE_OBJ)
	$(CC) $(ENGINE_OBJ) -o $@

# Engine objects compile without SDL include path.
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_ENGINE) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# =============================================================================
# Clean
# =============================================================================
clean:
	$(RM) $(BUILD_DIR) $(ENGINE_BIN)

.PHONY: all debug release clean test
