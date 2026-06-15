//! screen.c
//!
//! Engine-side screen registry. Maps ScreenId to the concrete
//! Screen v-tables declared in screens.h, plus the wire-token
//! mapping used by `< SHOW` lines and the `goto <screen>` verb.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              REGISTRY
\*--------------------------------------------------------------------------*/

static const Screen* const REGISTRY[SCREEN_COUNT] = {
    [SCREEN_TITLE]    = &SCREEN_TITLE_V,
    [SCREEN_MAP]      = &SCREEN_MAP_V,
    [SCREEN_BATTLE]   = &SCREEN_BATTLE_V,
    [SCREEN_EVENT]    = &SCREEN_EVENT_V,
    [SCREEN_RESULTS]  = &SCREEN_RESULTS_V,
    [SCREEN_CODEX]    = &SCREEN_CODEX_V,
    [SCREEN_MASTERY]  = &SCREEN_MASTERY_V,
    [SCREEN_SETTINGS] = &SCREEN_SETTINGS_V,
};

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

const Screen* screen_get(ScreenId id) {
    if (id < 0 || id >= SCREEN_COUNT)
        return NULL;
    return REGISTRY[id];
}

void screen_goto(EngineState* engine, ScreenId id) {
    if (engine == NULL)
        return;
    if (id < 0 || id >= SCREEN_COUNT) {
        log_warn("screen_goto: out-of-range id %d", id);
        return;
    }
    engine->next               = id;
    engine->transition_pending = true;
}

void screen_apply_transition(EngineState* engine) {
    if (engine == NULL || !engine->transition_pending)
        return;
    const Screen* old_screen = screen_get(engine->current);
    const Screen* new_screen = screen_get(engine->next);
    if (old_screen != NULL && old_screen->leave != NULL)
        old_screen->leave(engine);
    engine->current            = engine->next;
    engine->transition_pending = false;
    if (new_screen != NULL && new_screen->enter != NULL)
        new_screen->enter(engine);
    if (new_screen != NULL && new_screen->emit != NULL)
        new_screen->emit(engine);
}
