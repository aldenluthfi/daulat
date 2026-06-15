//! engine.c
//!
//! Engine state machine implementation. Owns the lifecycle for
//! profile / run / battle pointers, parses one frontend verb per
//! call, dispatches to the active screen's handle hook, and
//! applies any scheduled transition.
//!
//! Created: 2026-06-15
//! Author : Alden Luthfi

#include <stdlib.h>
#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              LIFECYCLE
\*--------------------------------------------------------------------------*/

bool engine_init(EngineState* engine, FILE* in, FILE* out) {
    if (engine == NULL)
        return false;
    memset(engine, 0, sizeof(*engine));
    engine->in  = in;
    engine->out = out;

    engine->profile = calloc(1, sizeof(Profile));
    if (engine->profile == NULL) {
        log_err("engine_init: Profile allocation failed");
        return false;
    }
    if (!profile_load(engine->profile))
        log_info("engine_init: no profile found; starting fresh");

    engine->current            = SCREEN_TITLE;
    engine->next               = SCREEN_TITLE;
    engine->transition_pending = false;
    engine->quitting           = false;

    const Screen* title = screen_get(SCREEN_TITLE);
    if (title != NULL && title->enter != NULL)
        title->enter(engine);
    if (title != NULL && title->emit != NULL)
        title->emit(engine);
    fflush(engine->out);
    return true;
}

void engine_destroy(EngineState* engine) {
    if (engine == NULL)
        return;
    if (engine->profile != NULL) {
        profile_save(engine->profile);
        free(engine->profile);
        engine->profile = NULL;
    }
    if (engine->run != NULL) {
        free(engine->run);
        engine->run = NULL;
    }
    if (engine->battle != NULL) {
        free(engine->battle);
        engine->battle = NULL;
    }
}

/*--------------------------------------------------------------------------*\
                              DISPATCH
\*--------------------------------------------------------------------------*/

void engine_handle_line(EngineState* engine, const char* line) {
    if (engine == NULL || line == NULL)
        return;
    ProtocolVerb parsed;
    protocol_parse(line, &parsed);
    if (parsed.verb[0] == '\0')
        return;
    if (strcmp(parsed.verb, "quit") == 0) {
        engine->quitting = true;
        protocol_emit_quit(engine->out);
        fflush(engine->out);
        return;
    }
    if (strcmp(parsed.verb, "goto") == 0) {
        const char* tail = parsed.tail;
        for (ScreenId id = 0; id < SCREEN_COUNT; id++) {
            const char* name = screen_name(id);
            if (name != NULL && strcmp(tail, name) == 0) {
                screen_goto(engine, id);
                break;
            }
        }
    } else if (strcmp(parsed.verb, "open_codex") == 0) {
        const char* kind = parsed.tail;
        if (kind[0] == '\0')
            kind = "piece";
        size_t length = strlen(kind);
        if (length >= sizeof(engine->codex.kind))
            length = sizeof(engine->codex.kind) - 1;
        memcpy(engine->codex.kind, kind, length);
        engine->codex.kind[length] = '\0';
        engine->codex.open         = true;
        protocol_emit_popup(
            engine->out,
            "codex",
            "kind=%s",
            engine->codex.kind
        );
    } else if (strcmp(parsed.verb, "close_popup") == 0) {
        engine->codex.open = false;
        protocol_emit_popup(engine->out, "close", "");
    } else {
        const Screen* active = screen_get(engine->current);
        if (active != NULL && active->handle != NULL)
            active->handle(engine, &parsed);
    }
    screen_apply_transition(engine);
    fflush(engine->out);
}
