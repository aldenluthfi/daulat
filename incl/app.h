//! app.h
//!
//! Top-level application state and SDL3 lifecycle hooks. The `App`
//! struct owns the SDL window and renderer, the active screen
//! transition state, and the input edge-detector. Phase-later fields
//! (Profile, RunState, BattleState) are attached as forward-declared
//! pointers and populated in their respective phases.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef APP_H
#define APP_H

#include <SDL3/SDL.h>

#include "input.h"
#include "screen.h"

struct Profile;
struct RunState;
struct BattleState;

/*--------------------------------------------------------------------------*\
                              APP STATE
\*--------------------------------------------------------------------------*/

/// App
///
/// Owner of SDL3 handles, the input edge-detector, and the current
/// and pending screen ids. Game-state pointers are NULL until their
/// respective phases populate them.
///
typedef struct App {
    SDL_Window*   window;
    SDL_Renderer* renderer;

    ScreenId current;
    ScreenId next;
    bool     transition_pending;

    Input    input;
    uint64_t last_tick_ns;

    struct Profile*     profile;
    struct RunState*    run;
    struct BattleState* battle;
} App;

/*--------------------------------------------------------------------------*\
                              SDL3 CALLBACKS
\*--------------------------------------------------------------------------*/

/// app_init
///
/// Allocate and initialize the App: SDL_Init, window + renderer
/// creation, input reset, initial screen entry. Returns SDL_APP_FAILURE
/// on any SDL error.
///
/// Params:
/// - App** out_app -> receives the heap-allocated App pointer
///
/// Return:
/// SDL_AppResult -> SDL_APP_CONTINUE on success
///
SDL_AppResult app_init(App** out_app);

/// app_iterate
///
/// One frame: apply pending transition, begin frame, tick, render.
/// Returns SDL_APP_SUCCESS if the user requested quit.
///
/// Params:
/// - App* app -> app to advance
///
/// Return:
/// SDL_AppResult -> SDL_APP_CONTINUE or SDL_APP_SUCCESS
///
SDL_AppResult app_iterate(App* app);

/// app_event
///
/// Fold one SDL event into input state and forward to the active
/// screen's `event` hook. Returns SDL_APP_SUCCESS on SDL_EVENT_QUIT.
///
/// Params:
/// - App*           app   -> app to mutate
/// - const SDL_Event* event -> SDL event to consume
///
/// Return:
/// SDL_AppResult -> SDL_APP_CONTINUE or SDL_APP_SUCCESS
///
SDL_AppResult app_event(App* app, const SDL_Event* event);

/// app_quit
///
/// Tear down SDL3 resources and free the App. Idempotent w.r.t. NULL.
///
/// Params:
/// - App* app -> app to destroy
///
void app_quit(App* app);

#endif /* APP_H */
