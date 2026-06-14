//! card.c
//!
//! Card draw, play, sell, and hand management.
//! Cards live in BattleState.hand[SIDE][MAX_HAND] as CardInstance structs.
//! Hand slots are filled from the deck on draw and freed on play or sell.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              DRAW
\*--------------------------------------------------------------------------*/

/// card_draw
///
/// Draw a card from the cardset into the hand. Emits TRIGGER_CARD_DRAWN.
///
/// Params:
/// - BattleState* bs -> battle state with cardset and hand
/// - Side side -> side to draw for
///
/// Return:
/// size_t -> number of cards drawn (0 or 1)
///
size_t card_draw(BattleState* bs, Side side) {
    uint16_t csz = bs->cardset_count[side];
    if (csz == 0)
        return 0;
    uint8_t current = bs->hand_count[side];
    if (current >= MAX_HAND)
        return 0;
    uint64_t            idx       = rng_range(&bs->rng, csz);
    const CardTemplate* tmpl      = bs->cardset[side][idx];
    bs->hand[side][current].tmpl  = tmpl;
    bs->hand[side][current].flags = 0;
    bs->hand_count[side]++;
    struct EffectCtx ctx = {0};
    ctx.as.card.side     = side;
    ctx.as.card.card     = &bs->hand[side][current];
    bus_emit(&bs->bus, bs, TRIGGER_CARD_DRAWN, &ctx);
    return 1;
}

/// card_add_to_hand
///
/// Add a card directly to a side's hand without drawing.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - Side side -> side to receive the card
/// - const CardTemplate* tmpl -> card template to add
///
void card_add_to_hand(BattleState* bs, Side side, const CardTemplate* tmpl) {
    uint8_t current = bs->hand_count[side];
    if (current >= MAX_HAND)
        return;
    bs->hand[side][current].tmpl  = tmpl;
    bs->hand[side][current].flags = 0;
    bs->hand_count[side]++;
}

/// card_remove_from_hand
///
/// Remove a card from a side's hand by index.
///
/// Params:
/// - BattleState* bs -> battle state to modify
/// - Side side -> side whose hand to modify
/// - uint8_t idx -> index of card to remove
///
/// Return:
/// bool -> true if removal succeeded
///
bool card_remove_from_hand(BattleState* bs, Side side, uint8_t idx) {
    if (idx >= bs->hand_count[side])
        return false;
    bs->hand_count[side]--;
    for (uint8_t i = idx; i < bs->hand_count[side]; i++) {
        bs->hand[side][i] = bs->hand[side][i + 1];
    }
    return true;
}

/// card_get
///
/// Get a card from a side's hand by index.
///
/// Params:
/// - const BattleState* bs -> battle state to query
/// - Side side -> side whose hand to query
/// - uint8_t idx -> index of card to retrieve
///
/// Return:
/// const CardInstance* -> card at index or NULL if out of bounds
///
const CardInstance* card_get(const BattleState* bs, Side side, uint8_t idx) {
    if (idx >= bs->hand_count[side])
        return NULL;
    return &bs->hand[side][idx];
}