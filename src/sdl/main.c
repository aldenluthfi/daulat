//! main.c
//!
//! SDL3 entry point for the Regnum frontend. Uses the
//! `SDL_MAIN_USE_CALLBACKS` pattern; control is handed off to the
//! four `SDL_App*` callbacks defined in src/sdl/app.c. The engine
//! is a separate process spawned at init via fork+pipe — see
//! src/sdl/app.c.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"

/*--------------------------------------------------------------------------*\
                              ENGINE PATH
\*--------------------------------------------------------------------------*/

static char ENGINE_PATH[1024];

/// Locate `regnum_engine` next to the running SDL binary. SDL3's
/// SDL_GetBasePath returns the executable directory with a
/// trailing slash; appending the engine binary name is enough on
/// every supported OS.
static const char* resolve_engine_path(void) {
    const char* base = SDL_GetBasePath();
    if (base == NULL || base[0] == '\0')
        base = "./";
    SDL_snprintf(ENGINE_PATH, sizeof(ENGINE_PATH), "%sregnum_engine", base);
    return ENGINE_PATH;
}

/*--------------------------------------------------------------------------*\
                              CALLBACKS
\*--------------------------------------------------------------------------*/

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    App*          app    = NULL;
    SDL_AppResult result = app_init(&app, resolve_engine_path());
    *appstate            = app;
    return result;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    return app_iterate((App*)appstate);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    return app_event((App*)appstate, event);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    (void)result;
    app_quit((App*)appstate);
}
