//! card.h
//!
//! Card templates and hand management for the Regnum battle engine.
//! Cards are bought with CP, played for effects, or sold for sell_value.
//! Hand slots are filled from the deck on draw and freed on play or sell.
//!
//! Created: 2026-06-13
//! Author : Alden Luthfi

#ifndef CARD_H
#define CARD_H

#include "effect.h"
#include "types.h"

struct BattleState;

/// A function that enumerates valid targets for a card.
typedef size_t (*TargetFunc)(
    const struct BattleState* battle,
    Side                      side,
    const CardInstance*       card,
    TargetSpec*               out,
    size_t                    cap
);

/*--------------------------------------------------------------------------*\
                              CARD TEMPLATE
\*--------------------------------------------------------------------------*/

/// Immutable description of a card type. Lives in src/data/data_*.c.
typedef struct CardTemplate {
    uint16_t    id;
    const char* name;
    Kingdom     kingdom;
    Tier        tier;
    int         play_cost; /* -1 if no play action */
    int         sell_value;
    Effect      on_play[MAX_CARD_EFFECTS];
    uint8_t     play_effect_count;
    Effect      on_sell[MAX_CARD_EFFECTS];
    uint8_t     sell_effect_count;
    TargetFunc  target_query;
} CardTemplate;

/*--------------------------------------------------------------------------*\
                              CARD API
\*--------------------------------------------------------------------------*/

/// Draw cards for a side. Returns the number drawn.
size_t card_draw(struct BattleState* battle, Side side);

/// Add a card to a side's hand.
void card_add_to_hand(
    struct BattleState* battle,
    Side                side,
    const CardTemplate* template
);

/// Remove a card from a side's hand by index. Returns true if removed.
bool card_remove_from_hand(struct BattleState* battle, Side side, uint8_t index);

/// Get a card from a side's hand.
const CardInstance*
card_get(const struct BattleState* battle, Side side, uint8_t index);

#endif /* CARD_H */