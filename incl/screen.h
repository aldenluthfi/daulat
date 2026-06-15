//! screen.h
//!
//! Screen v-table and dispatch enum for the headless engine. A
//! screen is a (enter, leave, handle, emit) tuple operating on
//! EngineState: `enter` runs on transition in; `leave` on
//! transition out; `handle` consumes one parsed verb from the
//! frontend; `emit` writes the SHOW + STATE lines that describe
//! the current view. Rendering lives in the SDL frontend, which
//! parses the engine's `<` lines into its own model.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef SCREEN_H
#define SCREEN_H

struct EngineState;
struct ProtocolVerb;

/*--------------------------------------------------------------------------*\
                              SCREEN IDS
\*--------------------------------------------------------------------------*/

/// Dense enum naming every screen. SCREEN_COUNT is the registry
/// size.
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
                              V-TABLE
\*--------------------------------------------------------------------------*/

/// V-table of per-screen lifecycle and per-verb hooks. Any hook
/// may be NULL; the dispatcher treats NULL as a no-op.
typedef struct Screen {
    void (*enter)(struct EngineState* engine);
    void (*leave)(struct EngineState* engine);
    void (*handle)(struct EngineState* engine, const struct ProtocolVerb* verb);
    void (*emit)(struct EngineState* engine);
} Screen;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// Look up the Screen v-table for a given id. Returns NULL when
/// the id is out of range or no screen was registered.
const Screen* screen_get(ScreenId id);

/// Return the protocol token used for a screen on the wire
/// ("title", "map", "battle", ...). NULL on unknown id.
const char* screen_name(ScreenId id);

/// Request a transition to a new screen. The transition lands at
/// the next pump boundary so the current screen's emit finishes
/// without re-entry surprise.
void screen_goto(struct EngineState* engine, ScreenId id);

/// If a transition was requested, call the outgoing screen's
/// `leave`, the incoming screen's `enter`, and the incoming
/// screen's `emit`. Called by the engine dispatcher once per
/// pump.
void screen_apply_transition(struct EngineState* engine);

#endif /* SCREEN_H */
