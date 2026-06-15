//! app.h
//!
//! SDL frontend lifecycle and state. Owns the SDL window, renderer,
//! input edge-detector, child-engine pipes, and the mirrored
//! FrontendModel the views render from. This header is SDL-coupled
//! and lives outside prelude.h; only src/sdl/ files include it.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#include <SDL3/SDL.h>

#include "input.h"

/*--------------------------------------------------------------------------*\
                              MODEL
\*--------------------------------------------------------------------------*/

/// Mirror of the engine's current view, hydrated from `< SHOW`,
/// `< STATE`, `< POPUP` lines. The frontend re-renders from this
/// every frame.
typedef struct FrontendModel {
    char screen[32];
    char detail[256];
    char popup[32];
    char popup_detail[256];
    bool engine_quit;
} FrontendModel;

/*--------------------------------------------------------------------------*\
                              APP STATE
\*--------------------------------------------------------------------------*/

/// Owner of SDL3 handles, the input edge-detector, the engine
/// child process handles, and the FrontendModel.
typedef struct App {
    SDL_Window*   window;
    SDL_Renderer* renderer;

    Input    input;
    uint64_t last_tick_ns;

    pid_t  engine_pid;
    FILE*  engine_in; /* parent writes "> ..." here          */
    FILE*  engine_out; /* parent reads  "< ..." here          */
    char   read_buffer[1024];
    size_t read_length;

    FrontendModel model;
} App;

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

/// Fork the engine binary as a child, set up pipes, create the
/// SDL window + renderer. Returns SDL_APP_FAILURE on any error.
SDL_AppResult app_init(App** out_app, const char* engine_binary_path);

/// One frame: drain engine output, render the model. Returns
/// SDL_APP_SUCCESS when the engine quit or the user requested
/// quit.
SDL_AppResult app_iterate(App* app);

/// Translate one SDL_Event into an engine verb when it is a key
/// press; pass through SDL_EVENT_QUIT.
SDL_AppResult app_event(App* app, const SDL_Event* event);

/// Send `> quit` to the child, wait, tear down SDL.
void app_quit(App* app);

/*--------------------------------------------------------------------------*\
                              PIPE I/O
\*--------------------------------------------------------------------------*/

/// Write one verb line ("> <text>\n") to the engine.
void app_send(App* app, const char* fmt, ...);

/// Parse one engine line into the FrontendModel.
void app_consume_line(App* app, const char* line);

#endif /* APP_H */
