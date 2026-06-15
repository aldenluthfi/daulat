//!
//! Card effects: draw extra, skip, peek, target removal.
//! Card effects are registered at battle start from CardTemplate.on_play.
//! Hand management lives in card.c; card effects modify that state.
//! Created: 2026-06-13
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              CARD MODIFIERS
\*--------------------------------------------------------------------------*/

/// Increase draw count by delta this turn.
void eff_draw_extra(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Remove a card from hand without selling it.
void eff_discard_card(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}

/// Peek at the top N cards of the cardset.
void eff_peek_cards(
    struct EffectCtx* context,
    const EffectArg*  args,
    size_t            count
) {
    (void)context;
    (void)args;
    (void)count;
}
