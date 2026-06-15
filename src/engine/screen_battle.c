//! screen_battle.c
//!
//! Battle screen. Verbs forwarded to the engine when a battle is
//! in progress are routed through battle_action_*; for now the
//! screen is a stub that emits a header line so the frontend can
//! draw a battle screen.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              HOOKS
\*--------------------------------------------------------------------------*/

static void battle_handle(EngineState* engine, const ProtocolVerb* verb) {
    (void)engine;
    (void)verb;
}

static void battle_emit(EngineState* engine) {
    BattleState* battle = engine->battle;
    if (battle == NULL) {
        protocol_emit_show(engine->out, SCREEN_BATTLE, "turn=0 active=none");
        return;
    }
    const char* side =
        (battle->active_side == SIDE_PLAYER) ? "player" : "enemy";
    protocol_emit_show(
        engine->out, SCREEN_BATTLE,
        "turn=%u active=%s cp=%d meter=%d/%d",
        battle->turn_no, side,
        battle->cp[SIDE_PLAYER],
        battle->meter[SIDE_PLAYER],
        battle->meter_cap[SIDE_PLAYER]
    );
}

/*--------------------------------------------------------------------------*\
                              V-TABLE
\*--------------------------------------------------------------------------*/

const Screen SCREEN_BATTLE_V = {
    .enter  = NULL,
    .leave  = NULL,
    .handle = battle_handle,
    .emit   = battle_emit,
};
