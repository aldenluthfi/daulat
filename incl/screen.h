//! screen.h
//!
//! Screen v-table and dispatch enum for the Regnum app shell.
//! Every visible mode (title, map, battle, event, results, codex,
//! mastery, settings) implements a `Screen` and registers it via
//! `screens.h`. The active screen handles all input, ticks, and
//! draws for the current frame.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef SCREEN_H
#define SCREEN_H

#include <SDL3/SDL.h>

struct App;

/*--------------------------------------------------------------------------*\
                              SCREEN IDS
\*--------------------------------------------------------------------------*/

/// ScreenId
///
/// Dense enum naming every screen. SCREEN_COUNT is the registry size.
///
typedef enum {
    SCREEN_TITLE = 0,
    SCREEN_MAP,
    SCREEN_BATTLE,
    SCREEN_EVENT,
    SCREEN_RESULTS,
    SCREEN_CODEX,
    SCREEN_MASTERY,
    SCREEN_SETTINGS,
    SCREEN_COUNT
} ScreenId;

/*--------------------------------------------------------------------------*\
                              SCREEN V-TABLE
\*--------------------------------------------------------------------------*/

/// Screen
///
/// V-table of per-screen lifecycle and per-frame hooks. Any hook may
/// be NULL; the dispatcher treats NULL as a no-op.
///
typedef struct Screen {
    void (*enter)(struct App* app);
    void (*leave)(struct App* app);
    void (*event)(struct App* app, const SDL_Event* event);
    void (*tick)(struct App* app, float dt);
    void (*render)(struct App* app, SDL_Renderer* renderer);
} Screen;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// screen_get
///
/// Look up the Screen v-table for a given id. Returns NULL if the id
/// is out of range or no screen was registered.
///
/// Params:
/// - ScreenId id -> id to look up
///
/// Return:
/// const Screen* -> screen v-table, or NULL
///
const Screen* screen_get(ScreenId id);

/// screen_goto
///
/// Request a transition to a new screen. The transition is deferred
/// until the next frame so the current screen's tick / render can
/// finish without surprise re-entry.
///
/// Params:
/// - struct App* app -> app holding the active screen state
/// - ScreenId    id  -> id of the screen to enter next
///
void screen_goto(struct App* app, ScreenId id);

/// screen_apply_transition
///
/// If a transition was requested, call the outgoing screen's `leave`
/// and the incoming screen's `enter`. Called by the app loop once per
/// frame, before tick.
///
/// Params:
/// - struct App* app -> app to advance
///
void screen_apply_transition(struct App* app);

#endif /* SCREEN_H */
