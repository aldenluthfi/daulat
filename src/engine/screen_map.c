//! screen_map.c
//!
//! Overworld map screen. Verbs:
//!   select_node <id>   move the highlight to a different node.
//!   close_popup        close any open codex/popup.
//!
//! Emits a single STATE line per node carrying the map shape.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include <string.h>

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void map_handle(EngineState* engine, const ProtocolVerb* verb) {
    if (strcmp(verb->verb, "select_node") == 0) {
        int parsed = 0;
        if (sscanf(verb->tail, "%d", &parsed) == 1)
            engine->map.node_cursor = (uint8_t)parsed;
    }
}

static void map_emit(EngineState* engine) {
    RunState* run = engine->run;
    if (run == NULL) {
        protocol_emit_show(engine->out, SCREEN_MAP, "kingdom=0 tier=0");
        return;
    }
    protocol_emit_show(
        engine->out,
        SCREEN_MAP,
        "kingdom=%d tier=%d node_cursor=%u",
        (int)run->current_kingdom,
        (int)run->current_map_tier,
        engine->map.node_cursor
    );
    for (uint8_t i = 0; i < run->current_map.node_count; i++) {
        const MapNode* node = &run->current_map.nodes[i];
        protocol_emit_state(
            engine->out,
            "map.node",
            "id=%u type=%u payload=%u x=%u y=%u",
            node->id,
            (unsigned)node->type,
            node->payload_id,
            (unsigned)node->id,
            (unsigned)i
        );
    }
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_MAP_V = {
    .enter  = NULL,
    .leave  = NULL,
    .handle = map_handle,
    .emit   = map_emit,
};
