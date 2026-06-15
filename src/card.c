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
/// - BattleState* battle -> battle state with cardset and hand
/// - Side side -> side to draw for
///
/// Return:
/// size_t -> number of cards drawn (0 or 1)
///
size_t card_draw(BattleState* battle, Side side) {
    uint16_t csz = battle->cardset_count[side];
    if (csz == 0)
        return 0;
    uint8_t current = battle->hand_count[side];
    if (current >= MAX_HAND)
        return 0;
    uint64_t            index       = rng_range(&battle->rng, csz);
    const CardTemplate* template      = battle->cardset[side][index];
    battle->hand[side][current].template  = template;
    battle->hand[side][current].flags = 0;
    battle->hand_count[side]++;
    struct EffectCtx context = {0};
    context.as.card.side     = side;
    context.as.card.card     = &battle->hand[side][current];
    bus_emit(&battle->bus, battle, TRIGGER_CARD_DRAWN, &context);
    return 1;
}

/// card_add_to_hand
///
/// Add a card directly to a side's hand without drawing.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - Side side -> side to receive the card
/// - const CardTemplate* template -> card template to add
///
void card_add_to_hand(BattleState* battle, Side side, const CardTemplate* template) {
    uint8_t current = battle->hand_count[side];
    if (current >= MAX_HAND)
        return;
    battle->hand[side][current].template  = template;
    battle->hand[side][current].flags = 0;
    battle->hand_count[side]++;
}

/// card_remove_from_hand
///
/// Remove a card from a side's hand by index.
///
/// Params:
/// - BattleState* battle -> battle state to modify
/// - Side side -> side whose hand to modify
/// - uint8_t index -> index of card to remove
///
/// Return:
/// bool -> true if removal succeeded
///
bool card_remove_from_hand(BattleState* battle, Side side, uint8_t index) {
    if (index >= battle->hand_count[side])
        return false;
    battle->hand_count[side]--;
    for (uint8_t i = index; i < battle->hand_count[side]; i++) {
        battle->hand[side][i] = battle->hand[side][i + 1];
    }
    return true;
}

/// card_get
///
/// Get a card from a side's hand by index.
///
/// Params:
/// - const BattleState* battle -> battle state to query
/// - Side side -> side whose hand to query
/// - uint8_t index -> index of card to retrieve
///
/// Return:
/// const CardInstance* -> card at index or NULL if out of bounds
///
const CardInstance* card_get(const BattleState* battle, Side side, uint8_t index) {
    if (index >= battle->hand_count[side])
        return NULL;
    return &battle->hand[side][index];
}