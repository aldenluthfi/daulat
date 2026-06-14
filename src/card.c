//!
//! Card draw, play, sell, and hand management.
//! Cards live in BattleState.hand[SIDE][MAX_HAND] as CardInstance structs.
//! Hand slots are filled from the deck on draw and freed on play or sell.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              DRAW
\*--------------------------------------------------------------------------*/

size_t card_draw(BattleState *bs, Side side) {
    uint16_t csz = bs->cardset_count[side];
    if (csz == 0)
        return 0;
    uint8_t current = bs->hand_count[side];
    if (current >= MAX_HAND)
        return 0;
    uint64_t idx = rng_range(&bs->rng, csz);
    const CardTemplate *tmpl = bs->cardset[side][idx];
    bs->hand[side][current].tmpl = tmpl;
    bs->hand[side][current].flags = 0;
    bs->hand_count[side]++;
    struct EffectCtx ctx = {0};
    ctx.as.card.side = side;
    ctx.as.card.card = &bs->hand[side][current];
    bus_emit(&bs->bus, bs, TRIGGER_CARD_DRAWN, &ctx);
    return 1;
}

void card_add_to_hand(BattleState *bs, Side side, const CardTemplate *tmpl) {
    uint8_t current = bs->hand_count[side];
    if (current >= MAX_HAND)
        return;
    bs->hand[side][current].tmpl = tmpl;
    bs->hand[side][current].flags = 0;
    bs->hand_count[side]++;
}

bool card_remove_from_hand(BattleState *bs, Side side, uint8_t idx) {
    if (idx >= bs->hand_count[side])
        return false;
    bs->hand_count[side]--;
    for (uint8_t i = idx; i < bs->hand_count[side]; i++) {
        bs->hand[side][i] = bs->hand[side][i + 1];
    }
    return true;
}

const CardInstance *card_get(const BattleState *bs, Side side, uint8_t idx) {
    if (idx >= bs->hand_count[side])
        return NULL;
    return &bs->hand[side][idx];
}