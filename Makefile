# Compiler and flags
CC = gcc
RM = rm -rf

# Directories
SRC_DIR = src
INC_DIR = incl
BUILD_DIR = build
BIN_DIR = bin

# =============================================================================
# Source partition: engine vs. SDL frontend
# =============================================================================
# Engine: every src/*.c plus src/engine, src/effects, src/movegens, src/data.
# Never sees SDL3/SDL.h.
ENGINE_SRC := $(wildcard $(SRC_DIR)/*.c) \
              $(wildcard $(SRC_DIR)/engine/*.c) \
              $(wildcard $(SRC_DIR)/effects/*.c) \
              $(wildcard $(SRC_DIR)/movegens/*.c) \
              $(wildcard $(SRC_DIR)/data/*.c)

# SDL frontend: src/sdl/ plus the shared protocol parser/emitter.
SDL_SRC := $(wildcard $(SRC_DIR)/sdl/*.c) $(SRC_DIR)/engine/protocol.c

ENGINE_OBJ := $(ENGINE_SRC:%=$(BUILD_DIR)/%.o)
SDL_OBJ    := $(SDL_SRC:%=$(BUILD_DIR)/sdl/%.o)

ENGINE_BIN := $(BIN_DIR)/regnum_engine
SDL_BIN    := $(BIN_DIR)/regnum

INC_FLAGS := -I$(INC_DIR)

# SDL3 linkage (prefer pkg-config; fall back to SDL_PATH override)
SDL3_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
SDL3_LIBS   := $(shell pkg-config --libs sdl3 2>/dev/null)
ifeq ($(SDL3_CFLAGS),)
SDL_PATH   ?= /opt/homebrew
SDL3_CFLAGS := -I$(SDL_PATH)/include
SDL3_LIBS   := -L$(SDL_PATH)/lib -Wl,-rpath,$(SDL_PATH)/lib -lSDL3
endif

# =============================================================================
# Modes
# =============================================================================
COMMON_WARN   := -Wall -Wextra -Werror -std=c11
DEBUG_FLAGS   := -g -O0 -DDEBUG $(COMMON_WARN)
RELEASE_FLAGS := -O3 -ffast-math -DNDEBUG -Wall -std=c11

debug:   CFLAGS_ENGINE := $(DEBUG_FLAGS) $(INC_FLAGS)
debug:   CFLAGS_SDL    := $(DEBUG_FLAGS) $(INC_FLAGS) $(SDL3_CFLAGS)
debug:   LDFLAGS_SDL   := $(SDL3_LIBS)
debug:   all

release: CFLAGS_ENGINE := $(RELEASE_FLAGS) $(INC_FLAGS)
release: CFLAGS_SDL    := $(RELEASE_FLAGS) $(INC_FLAGS) $(SDL3_CFLAGS)
release: LDFLAGS_SDL   := $(SDL3_LIBS)
release: all

# =============================================================================
# Build target
# =============================================================================
all: $(BUILD_DIR) $(BIN_DIR) $(ENGINE_BIN) $(SDL_BIN)

$(ENGINE_BIN): $(ENGINE_OBJ)
	$(CC) $(ENGINE_OBJ) -o $@

$(SDL_BIN): $(SDL_OBJ)
	$(CC) $(SDL_OBJ) -o $@ $(LDFLAGS_SDL)

# SDL frontend objects compile with SDL include path.
$(BUILD_DIR)/sdl/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_SDL) -MMD -MP -c $< -o $@

# Engine objects compile without SDL include path.
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_ENGINE) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# =============================================================================
# Test: pipe each tests/*.in into the engine and diff against tests/*.out
# =============================================================================
TEST_INPUTS := $(wildcard tests/*.in)
test: $(ENGINE_BIN)
	@status=0; \
	for input in $(TEST_INPUTS); do \
	    expected=$${input%.in}.out; \
	    sandbox=$$(mktemp -d); \
	    actual=$$sandbox/out; \
	    HOME=$$sandbox XDG_DATA_HOME=$$sandbox/.data \
	        $(ENGINE_BIN) < $$input > $$actual 2>/dev/null; \
	    if diff -u $$actual $$expected > /dev/null; then \
	        echo "PASS $$input"; \
	    else \
	        echo "FAIL $$input"; \
	        diff -u $$expected $$actual || true; \
	        status=1; \
	    fi; \
	    rm -rf $$sandbox; \
	done; \
	exit $$status

# =============================================================================
# Clean
# =============================================================================
clean:
	$(RM) $(BUILD_DIR) $(ENGINE_BIN) $(SDL_BIN)

.PHONY: all debug release clean test
