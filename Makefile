# Compiler and flags
CC = gcc
RM = rm -rf

# Directories
SRC_DIR = src
INC_DIR = incl
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c)
OBJ_FILES = $(SRC_FILES:%=$(BUILD_DIR)/%.o)

# Targets
TARGET = $(BIN_DIR)/regnum

# Common flags
INC_FLAGS = $(addprefix -I, $(INC_DIR))

# SDL3 linkage (prefer pkg-config; fall back to SDL_PATH override)
SDL3_CFLAGS = $(shell pkg-config --cflags sdl3 2>/dev/null)
SDL3_LIBS   = $(shell pkg-config --libs sdl3 2>/dev/null)
ifeq ($(SDL3_CFLAGS),)
SDL_PATH   ?= /opt/homebrew
SDL3_CFLAGS = -I$(SDL_PATH)/include
SDL3_LIBS   = -L$(SDL_PATH)/lib -Wl,-rpath,$(SDL_PATH)/lib -lSDL3
endif

# =============================================================================
# Debug mode: symbols, no optimization, assertions enabled, strict warnings
# =============================================================================
debug: CFLAGS += -g -O0 -DDEBUG -Wall -Wextra -Werror -std=c11
debug: CFLAGS += $(INC_FLAGS) $(SDL3_CFLAGS)
debug: LDFLAGS += $(SDL3_LIBS)
debug: all

# =============================================================================
# Release mode: optimized, no debug info, stripped
# =============================================================================
release: CFLAGS += -O3 -ffast-math -DNDEBUG -Wall -std=c11
release: CFLAGS += $(INC_FLAGS) $(SDL3_CFLAGS)
release: LDFLAGS += $(SDL3_LIBS)
release: all

# =============================================================================
# Build target
# =============================================================================
all: $(BUILD_DIR) $(BIN_DIR) $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@ $(LDFLAGS)

# Pattern rule for object files + header dependencies
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

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
