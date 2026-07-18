//! relic.c
//!
//! Relic definitions. Holds the registry of all twenty-six relics with
//! their static effect bodies, grouped by the aspect they affect:
//! economy, meter, cards, combinations, and board. Relics are attached to
//! the human seat at battle start by the battle.c run-walk; a relic that
//! observes the enemy (Soul Shard) is attached to the enemy list with the
//! human as its beneficiary in args[0].
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                                    ECONOMY
\*----------------------------------------------------------------------------*/

/// eff_ledger
///
/// Merchant's Ledger: every card sells for five more.
///
/// Params:
/// - context -> unused
/// - x       -> int* sell cost to raise
///
/// Return: true, the bonus always applies
///
static bool eff_ledger(EffectContext* context, void* x) {
    (void) context;

    *(int*) x += 5;

    return true;
}

/// eff_minted_coin
///
/// Minted Coin: five extra currency at the start of every turn.
///
/// Params:
/// - context -> unused
/// - x       -> int* income to raise
///
/// Return: true, the bonus always applies
///
static bool eff_minted_coin(EffectContext* context, void* x) {
    (void) context;

    *(int*) x += 5;

    return true;
}

/// eff_tax_stamp
///
/// Tax Stamp: playing a card with a play cost yields ten currency.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> Card* being played
///
/// Return: true when a paid card triggered the refund
///
static bool eff_tax_stamp(EffectContext* context, void* x) {
    Card* card = x;

    if (card->play_cost <= 0) {
        return false;
    }

    Side side = (Side) (uintptr_t) context->args[0];

    battle_player(battle_current(), side)->cp += 10;

    return true;
}

#define BULK_STATE ((uintptr_t) 0x42554c4b)

/// bulk_mark
///
/// Finds, or lazily creates, Bulk Discount's shared per-turn state on the
/// owner's effect list. The mark stores the committed buy count in args[2],
/// the cheapest committed cost in args[3], the discount granted so far in
/// args[4], and the pending count, cheapest, and granted for the buy being
/// priced in args[5], args[6], and args[7].
///
/// Params:
/// - battle -> battle whose list holds the state
/// - side   -> owning side
///
/// Return: the state mark, or nullptr when attachment failed
///
static Effect* bulk_mark(BattleState* battle, Side side) {
    LinkedList* list = &battle_player(battle, side)->effects;
    Effect* mark = effect_find_mark(list, BULK_STATE, (void*) (uintptr_t) side);

    if (mark) {
        return mark;
    }

    Effect seed = {
        .func      = eff_noop,
        .lasts_for = ENTIRE_BATTLE,
    };

    mark = effect_attach(list, &seed);

    if (mark) {
        mark->context->args[0] = (void*) (uintptr_t) side;
        mark->context->args[1] = (void*) BULK_STATE;
    }

    return mark;
}

/// eff_bulk_reset
///
/// Bulk Discount bookkeeping: clears the per-turn buy state at the start of
/// the owner's turn.
///
/// Params:
/// - context -> args[0] owning side
/// - x       -> unused turn number
///
/// Return: false, a reset is silent bookkeeping
///
static bool eff_bulk_reset(EffectContext* context, void* x) {
    (void) x;

    Side    side = (Side) (uintptr_t) context->args[0];
    Effect* mark = bulk_mark(battle_current(), side);

    if (mark) {
        for (size_t slot = 2; slot <= 7; slot++) {
            mark->context->args[slot] = nullptr;
        }
    }

    return false;
}

/// eff_bulk_buy
///
/// Bulk Discount pricing: keeps the turn's total discount equal to the
/// single cheapest piece bought once three or more are committed. Each buy
/// is adjusted by the difference between the discount already granted and
/// the discount the running cheapest now warrants, so a later, cheaper buy
/// repays the earlier over-discount and itself becomes the free piece.
///
/// Params:
/// - context -> args[0] owning side
/// - x       -> int* buy cost being priced
///
/// Return: true when the cost was adjusted
///
static bool eff_bulk_buy(EffectContext* context, void* x) {
    Side    side = (Side) (uintptr_t) context->args[0];
    Effect* mark = bulk_mark(battle_current(), side);

    if (!mark) {
        return false;
    }

    int    cost            = *(int*) x;
    size_t count           = (size_t) (uintptr_t) mark->context->args[2];
    int    lowest          = (int) (uintptr_t) mark->context->args[3];
    int    granted         = (int) (uintptr_t) mark->context->args[4];

    size_t pending_count   = count + 1;
    int    pending_lowest  = count == 0 || cost < lowest ? cost : lowest;
    int    pending_granted = pending_count >= 3 ? pending_lowest : 0;
    int    adjust          = granted - pending_granted;

    mark->context->args[5] = (void*) (uintptr_t) pending_count;
    mark->context->args[6] = (void*) (uintptr_t) pending_lowest;
    mark->context->args[7] = (void*) (uintptr_t) pending_granted;

    if (adjust == 0) {
        return false;
    }

    *(int*) x = cost + adjust;

    return true;
}

/// eff_bulk_commit
///
/// Bulk Discount bookkeeping: promotes the pending pricing state to the
/// committed state once a buy actually lands, so failed or merely priced
/// buys never count toward the discount.
///
/// Params:
/// - context -> args[0] owning side
/// - x       -> PieceInfo* the piece just bought
///
/// Return: false, the commit is silent bookkeeping
///
static bool eff_bulk_commit(EffectContext* context, void* x) {
    PieceInfo* piece = x;
    Side       side  = (Side) (uintptr_t) context->args[0];

    if (!piece || piece->side != side) {
        return false;
    }

    Effect* mark = bulk_mark(battle_current(), side);

    if (!mark) {
        return false;
    }

    mark->context->args[2] = mark->context->args[5];
    mark->context->args[3] = mark->context->args[6];
    mark->context->args[4] = mark->context->args[7];

    return false;
}

/// eff_trade_routes
///
/// Trade Routes: drops the foreign-kingdom buy markup by dividing the
/// twenty-percent surcharge back out of a foreign piece's cost (a rounding
/// of at most one currency).
///
/// Params:
/// - context -> unused
/// - x       -> int* buy cost to relieve
///
/// Return: true when a foreign piece's markup was removed
///
static bool eff_trade_routes(EffectContext* context, void* x) {
    (void) context;

    BattleState* battle = battle_current();
    const Piece* piece  = battle_buy_piece();

    if (!piece || piece->kingdom == KINGDOM_NONE || !battle->node ||
        !battle->node->kingdom || piece->kingdom == battle->node->kingdom->id) {
        return false;
    }

    *(int*) x = *(int*) x * 100 / 120;

    return true;
}

/// eff_war_chest
///
/// War Chest: unspent currency at end of turn adds a fifth of itself to
/// the meter.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> unused turn number
///
/// Return: true when there was currency to convert
///
static bool eff_war_chest(EffectContext* context, void* x) {
    (void) x;

    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();
    PlayerState* player = battle_player(battle, side);

    if (player->cp <= 0) {
        return false;
    }

    battle_meter_gain(battle, side, player->cp * 20 / 100);

    return true;
}

/*----------------------------------------------------------------------------*\
                                     METER
\*----------------------------------------------------------------------------*/

/// eff_soul_shard
///
/// Soul Shard: gaining a flipped piece adds thirty to the meter. Lives on
/// the human seat; ON_PIECE_FLIP fires the gaining side's list, so it runs
/// only when a piece flips onto the human side.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> PieceInfo* that just flipped
///
/// Return: true when the beneficiary gained the piece
///
static bool eff_soul_shard(EffectContext* context, void* x) {
    PieceInfo* piece = x;
    Side       benef = (Side) (uintptr_t) context->args[0];

    if (piece->side != benef) {
        return false;
    }

    battle_meter_gain(battle_current(), benef, 30);

    return true;
}

/// eff_veterans_bond
///
/// Veteran's Bond: pieces valued fifty or more contribute twenty extra to
/// the meter. Acts only on the effective-value query.
///
/// Params:
/// - context -> unused
/// - x       -> int* damage or effective value
///
/// Return: true when a high-value piece got the bonus
///
static bool eff_veterans_bond(EffectContext* context, void* x) {
    (void) context;

    if (battle_victim() || battle_subject()->piece->value < 50) {
        return false;
    }

    *(int*) x += 20;

    return true;
}

/// eff_dead_mans_pact
///
/// Dead Man's Pact: the first time the meter would empty in a battle it
/// resets to twenty instead.
///
/// Params:
/// - context -> args[0] beneficiary side, args[1] spent flag
/// - x       -> int* total meter damage to reduce
///
/// Return: true when the pact intercepted a lethal hit
///
static bool eff_dead_mans_pact(EffectContext* context, void* x) {
    if (context->args[1]) {
        return false;
    }

    PlayerState* player =
        battle_player(battle_current(), (Side) (uintptr_t) context->args[0]);

    if (player->meter - *(int*) x > 0) {
        return false;
    }

    *(int*) x        = player->meter - 20;
    context->args[1] = (void*) 1;

    return true;
}

/// eff_iron_king
///
/// Iron King: the king contributes twenty to the meter instead of ten.
///
/// Params:
/// - context -> unused
/// - x       -> int* effective value to raise
///
/// Return: true when the subject was the king
///
static bool eff_iron_king(EffectContext* context, void* x) {
    (void) context;

    if (battle_victim() || battle_subject()->piece->id != PIECE_KING) {
        return false;
    }

    *(int*) x += 10;

    return true;
}

/// eff_bloodthirst
///
/// Bloodthirst: at the start of a turn where the meter leads the enemy's,
/// gain five meter.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> unused turn number
///
/// Return: true when ahead and the bonus applied
///
static bool eff_bloodthirst(EffectContext* context, void* x) {
    (void) x;

    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    if (battle_player(battle, side)->meter <=
        battle_player(battle, battle_enemy(side))->meter) {
        return false;
    }

    battle_meter_gain(battle, side, 5);

    return true;
}

/// eff_last_breath
///
/// Last Breath: when a friendly piece flips, deal its value to the enemy
/// meter. Lives in the human list, which fires only on the human's own
/// losses.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> PieceInfo* that just flipped
///
/// Return: true, the retaliation always applies
///
static bool eff_last_breath(EffectContext* context, void* x) {
    PieceInfo*   piece  = x;
    Side         benef  = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    battle_damage(battle, battle_enemy(benef), piece->piece->value);

    return true;
}

/*----------------------------------------------------------------------------*\
                                     CARDS
\*----------------------------------------------------------------------------*/

/// eff_tacticians_scroll
///
/// Tactician's Scroll: draw four cards per turn.
///
/// Params:
/// - context -> unused
/// - x       -> int* draw count to set
///
/// Return: true, the count always applies
///
static bool eff_tacticians_scroll(EffectContext* context, void* x) {
    (void) context;

    *(int*) x = 4;

    return true;
}

/// eff_country_seal
///
/// Country Seal: selling a Country-tier card gives twenty extra currency.
///
/// Params:
/// - context -> unused
/// - x       -> int* sell cost to raise
///
/// Return: true when the sold card was Country tier
///
static bool eff_country_seal(EffectContext* context, void* x) {
    (void) context;

    Card* card = battle_subject_card();

    if (!card || card->tier != TIER_COUNTRY) {
        return false;
    }

    *(int*) x += 20;

    return true;
}

/// eff_gilded_archive
///
/// Gilded Archive: District-tier cards sell for ten more.
///
/// Params:
/// - context -> unused
/// - x       -> int* sell cost to raise
///
/// Return: true when the sold card was District tier
///
static bool eff_gilded_archive(EffectContext* context, void* x) {
    (void) context;

    Card* card = battle_subject_card();

    if (!card || card->tier != TIER_DISTRICT) {
        return false;
    }

    *(int*) x += 10;

    return true;
}

/// eff_librarians_notes
///
/// Librarian's Notes: once per turn, skip the top projected card. The skip
/// discards slot zero of the upcoming hand and refills it from the same
/// deterministic stream. args[1] stamps the turn served as turn + 1.
///
/// Params:
/// - context -> once-per-turn stamp in args[1]
/// - x       -> the acting seat's Side
///
/// Return: true, the skip always applies when usable
///
static bool eff_librarians_notes(EffectContext* context, void* x) {
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) x;

    battle_next_hand_discard(battle, side, 0);

    context->args[1] = (void*) (uintptr_t) (battle->turn + 1);

    return true;
}

/// eff_deep_hand
///
/// Deep Hand: once per battle, draw two extra cards immediately onto the
/// chosen turn. args[0] marks the single battle-long use as spent.
///
/// Params:
/// - context -> once-per-battle used flag in args[0]
/// - x       -> the acting seat's Side
///
/// Return: true, the bonus draw always applies when usable
///
static bool eff_deep_hand(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) x;

    battle_draw(battle_current(), side, 2);

    context->args[0] = (void*) 1;

    return true;
}

/*----------------------------------------------------------------------------*\
                                 COMBINATIONS
\*----------------------------------------------------------------------------*/

/// eff_alchemists_kit
///
/// Alchemist's Kit: combinations cost no actions.
///
/// Params:
/// - context -> unused
/// - x       -> int* combine action cost to zero
///
/// Return: true, the discount always applies
///
static bool eff_alchemists_kit(EffectContext* context, void* x) {
    (void) context;

    *(int*) x = 0;

    return true;
}

/// eff_philosophers_stone
///
/// Philosopher's Stone: once per battle a combined piece permanently
/// gains twenty value for that battle.
///
/// Params:
/// - context -> args[1] spent flag
/// - x       -> PieceInfo* combination result
///
/// Return: true when the one-shot bonus applied
///
static bool eff_philosophers_stone(EffectContext* context, void* x) {
    if (context->args[1]) {
        return false;
    }

    PieceInfo* piece     = x;
    piece->piece->value += 20;
    context->args[1]     = (void*) 1;

    return true;
}

/// eff_inherited_power
///
/// Inherited Power: every combined piece gains five value above its
/// result value.
///
/// Params:
/// - context -> unused
/// - x       -> PieceInfo* combination result
///
/// Return: true, the bonus always applies
///
static bool eff_inherited_power(EffectContext* context, void* x) {
    (void) context;

    PieceInfo* piece     = x;
    piece->piece->value += 5;

    return true;
}

/*----------------------------------------------------------------------------*\
                                     BOARD
\*----------------------------------------------------------------------------*/

/// eff_forward_command
///
/// Forward Command: pieces deal five extra damage while standing in enemy
/// territory.
///
/// Params:
/// - context -> unused
/// - x       -> int* damage to raise
///
/// Return: true when the attacker stood in enemy territory
///
static bool eff_forward_command(EffectContext* context, void* x) {
    (void) context;

    if (!battle_victim()) {
        return false;
    }

    PieceInfo* subject = battle_subject();

    if (battle_territory(battle_current(), subject->square) !=
        battle_enemy(subject->side)) {
        return false;
    }

    *(int*) x += 5;

    return true;
}

/// eff_eagle_eye
///
/// Eagle Eye: nothing hides pieces from the board. As a board-state effect
/// itself it runs on every board read, resets the whole view to visible,
/// and swaps every OTHER QUERY_BOARD_STATE effect's func for eff_noop — on
/// the human seat and on every piece — so any blinder, including one
/// attached mid-battle, is neutralised the next read. It skips itself.
/// Naming no item (it filters by trigger) keeps the engine agnostic and,
/// resetting the view up front, makes fire order irrelevant.
///
/// Params:
/// - context -> args[0] human side
/// - x       -> Board* whose visible flags are all set
///
/// Return: true, the reveal always applies
///
static bool eff_eagle_eye(EffectContext* context, void* x) {
    Board*       board  = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    for (size_t cell = 0; cell < MAX_BOARD_SIZE; cell++) {
        board->visible[cell] = true;
    }

    LinkedList* list = &battle_player(battle, side)->effects;

    for (LLNode* node = list->head; node; node = node->next) {
        Effect* effect = node->data;

        if (effect->trigger == QUERY_BOARD_STATE &&
            effect->func != eff_eagle_eye) {
            effect->func = eff_noop;
        }
    }

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* piece = battle->board.piece_board[index];

        if (!piece || piece == &VOID_CELL) {
            continue;
        }

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            Effect* effect = &piece->piece->effects[slot];

            if (effect->trigger == QUERY_BOARD_STATE &&
                effect->func != eff_eagle_eye) {
                effect->func = eff_noop;
            }
        }
    }

    return true;
}

/// eff_fortified_line
///
/// Fortified Line: a piece that did not move this turn deals five extra
/// damage. Reads the piece's last-move turn stamp and compares it to the
/// current turn, so no per-turn reset of move counts is needed.
///
/// Params:
/// - context -> args[0] beneficiary side
/// - x       -> int* damage to raise
///
/// Return: true when a still piece got the bonus
///
static bool eff_fortified_line(EffectContext* context, void* x) {
    if (!battle_victim()) {
        return false;
    }

    Side         side    = (Side) (uintptr_t) context->args[0];
    PieceInfo*   subject = battle_subject();
    BattleState* battle  = battle_current();

    if (!subject || subject->side != side) {
        return false;
    }

    if (battle_piece_move_turn(subject) == battle->turn) {
        return false;
    }

    *(int*) x += 5;

    return true;
}

/// eff_warlords_banner
///
/// Warlord's Banner: pieces adjacent to the friendly king deal five extra
/// damage.
///
/// Params:
/// - context -> unused
/// - x       -> int* damage to raise
///
/// Return: true when the attacker stood beside its king
///
static bool eff_warlords_banner(EffectContext* context, void* x) {
    (void) context;

    if (!battle_victim()) {
        return false;
    }

    PieceInfo* subject = battle_subject();
    PieceInfo* king    = battle_find_king(battle_current(), subject->side);

    if (!king) {
        return false;
    }

    int dx = subject->square.x - king->square.x;
    int dy = subject->square.y - king->square.y;

    dx     = dx < 0 ? -dx : dx;
    dy     = dy < 0 ? -dy : dy;

    if ((dx || dy) && dx <= 1 && dy <= 1) {
        *(int*) x += 5;

        return true;
    }

    return false;
}

/*----------------------------------------------------------------------------*\
                                    ARCHIVE
\*----------------------------------------------------------------------------*/

/// eff_masters_notes
///
/// Master's Notes: archive nodes reveal one extra recipe. Folds into the
/// run-side archive reveal count.
///
/// Params:
/// - context -> unused
/// - x       -> int* archive reveal count to raise
///
/// Return: true, the extra reveal always applies
///
static bool eff_masters_notes(EffectContext* context, void* x) {
    (void) context;

    *(int*) x += 1;

    return true;
}

/*----------------------------------------------------------------------------*\
                                   REGISTRY
\*----------------------------------------------------------------------------*/

/// RELIC_REGISTRY
///
/// The twenty-six relics as const templates. Run-scoped relics with no
/// battle effect (their behaviour lives at the map, draw, or pricing call
/// sites and is gated on run->relics) carry only a name and description.
///
const Relic RELIC_REGISTRY[RELIC_COUNT] = {
    [RELIC_MERCHANTS_LEDGER] =
        {
            .name    = "Merchant's Ledger",
            .desc    = "Card sell values +5 cp.",
            .id      = RELIC_MERCHANTS_LEDGER,
            .effects = {{
                .func      = eff_ledger,
                .name      = "Merchant's Ledger",
                .trigger   = QUERY_CARD_SELL_COST,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_MINTED_COIN] =
        {
            .name    = "Minted Coin",
            .desc    = "+5 cp at the start of every turn.",
            .id      = RELIC_MINTED_COIN,
            .effects = {{
                .func      = eff_minted_coin,
                .name      = "Minted Coin",
                .trigger   = QUERY_CP_INCOME,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_TAX_STAMP] =
        {
            .name    = "Tax Stamp",
            .desc    = "+10 cp when you play a card with a play cost.",
            .id      = RELIC_TAX_STAMP,
            .effects = {{
                .func      = eff_tax_stamp,
                .name      = "Tax Stamp",
                .trigger   = ON_CARD_PLAY,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_BULK_DISCOUNT] =
        {
            .name = "Bulk Discount",
            .desc = "Buying 3+ pieces in a turn, one is free.",
            .id   = RELIC_BULK_DISCOUNT,
            .effects =
                {
                    {
                        .func      = eff_bulk_reset,
                        .name      = "Bulk Discount",
                        .trigger   = ON_TURN_START,
                        .lasts_for = ENTIRE_BATTLE,
                    },
                    {
                        .func      = eff_bulk_buy,
                        .name      = "Bulk Discount",
                        .trigger   = QUERY_PIECE_CP_COST_BUY,
                        .lasts_for = ENTIRE_BATTLE,
                    },
                    {
                        .func      = eff_bulk_commit,
                        .name      = "Bulk Discount",
                        .trigger   = ON_PIECE_BUY,
                        .lasts_for = ENTIRE_BATTLE,
                    },
                },
        },
    [RELIC_WAR_CHEST] =
        {
            .name    = "War Chest",
            .desc    = "Unspent cp at end of turn adds 20% of itself to meter.",
            .id      = RELIC_WAR_CHEST,
            .effects = {{
                .func      = eff_war_chest,
                .name      = "War Chest",
                .trigger   = ON_TURN_END,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_TRADE_ROUTES] =
        {
            .name    = "Trade Routes",
            .desc    = "The foreign kingdom markup is removed for the run.",
            .id      = RELIC_TRADE_ROUTES,
            .effects = {{
                .func      = eff_trade_routes,
                .name      = "Trade Routes",
                .trigger   = QUERY_PIECE_CP_COST_BUY,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_SOUL_SHARD] =
        {
            .name    = "Soul Shard",
            .desc    = "Gaining a flipped piece adds 30 to your meter.",
            .id      = RELIC_SOUL_SHARD,
            .effects = {{
                .func      = eff_soul_shard,
                .name      = "Soul Shard",
                .trigger   = ON_PIECE_FLIP,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_VETERANS_BOND] =
        {
            .name    = "Veteran's Bond",
            .desc    = "Pieces valued 50+ contribute 20 extra to your meter.",
            .id      = RELIC_VETERANS_BOND,
            .effects = {{
                .func      = eff_veterans_bond,
                .name      = "Veteran's Bond",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_DEAD_MANS_PACT] =
        {
            .name    = "Dead Man's Pact",
            .desc    = "First lethal meter hit resets to 20 instead.",
            .id      = RELIC_DEAD_MANS_PACT,
            .effects = {{
                .func      = eff_dead_mans_pact,
                .name      = "Dead Man's Pact",
                .trigger   = QUERY_METER_DAMAGE_TAKEN,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_IRON_KING] =
        {
            .name    = "Iron King",
            .desc    = "Your king contributes 20 to your meter instead of 10.",
            .id      = RELIC_IRON_KING,
            .effects = {{
                .func      = eff_iron_king,
                .name      = "Iron King",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_BLOODTHIRST] =
        {
            .name    = "Bloodthirst",
            .desc    = "Start of a turn leading the enemy meter: gain 5 meter.",
            .id      = RELIC_BLOODTHIRST,
            .effects = {{
                .func      = eff_bloodthirst,
                .name      = "Bloodthirst",
                .trigger   = ON_TURN_START,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_LAST_BREATH] =
        {
            .name    = "Last Breath",
            .desc    = "A flipped friendly piece deals its value to the enemy.",
            .id      = RELIC_LAST_BREATH,
            .effects = {{
                .func      = eff_last_breath,
                .name      = "Last Breath",
                .trigger   = ON_PIECE_FLIP,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_TACTICIANS_SCROLL] =
        {
            .name    = "Tactician's Scroll",
            .desc    = "Draw 4 cards per turn.",
            .id      = RELIC_TACTICIANS_SCROLL,
            .effects = {{
                .func      = eff_tacticians_scroll,
                .name      = "Tactician's Scroll",
                .trigger   = QUERY_CARD_DRAW_COUNT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_LIBRARIANS_NOTES] =
        {
            .name = "Librarian's Notes",
            .desc = "Once per turn, peek the top card and skip it.",
            .id   = RELIC_LIBRARIANS_NOTES,
            .effects = {{
                .func      = eff_librarians_notes,
                .name      = "Librarian's Notes",
                .trigger   = ON_ITEM_ACTIVATE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_COUNTRY_SEAL] =
        {
            .name    = "Country Seal",
            .desc    = "Selling a Country-tier card gives +20 cp.",
            .id      = RELIC_COUNTRY_SEAL,
            .effects = {{
                .func      = eff_country_seal,
                .name      = "Country Seal",
                .trigger   = QUERY_CARD_SELL_COST,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_DEEP_HAND] =
        {
            .name = "Deep Hand",
            .desc = "Once per battle, draw 2 extra on a turn of your choice.",
            .id   = RELIC_DEEP_HAND,
            .effects = {{
                .func      = eff_deep_hand,
                .name      = "Deep Hand",
                .trigger   = ON_ITEM_ACTIVATE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_GILDED_ARCHIVE] =
        {
            .name    = "Gilded Archive",
            .desc    = "District-tier cards sell for +10 cp.",
            .id      = RELIC_GILDED_ARCHIVE,
            .effects = {{
                .func      = eff_gilded_archive,
                .name      = "Gilded Archive",
                .trigger   = QUERY_CARD_SELL_COST,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_ALCHEMISTS_KIT] =
        {
            .name    = "Alchemist's Kit",
            .desc    = "Combinations cost 0 actions.",
            .id      = RELIC_ALCHEMISTS_KIT,
            .effects = {{
                .func      = eff_alchemists_kit,
                .name      = "Alchemist's Kit",
                .trigger   = QUERY_PIECE_ACTION_COST_COMBINE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_MASTERS_NOTES] =
        {
            .name    = "Master's Notes",
            .desc    = "Archive nodes reveal 2 recipes instead of 1.",
            .id      = RELIC_MASTERS_NOTES,
            .effects = {{
                .func    = eff_masters_notes,
                .name    = "Master's Notes",
                .trigger = QUERY_ARCHIVE_REVEAL_COUNT,
            }},
        },
    [RELIC_PHILOSOPHERS_STONE] =
        {
            .name    = "Philosopher's Stone",
            .desc    = "Once per battle, a combined piece gains +20 value.",
            .id      = RELIC_PHILOSOPHERS_STONE,
            .effects = {{
                .func      = eff_philosophers_stone,
                .name      = "Philosopher's Stone",
                .trigger   = ON_PIECE_COMBINE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_INHERITED_POWER] =
        {
            .name    = "Inherited Power",
            .desc    = "Combined pieces gain +5 value above their result.",
            .id      = RELIC_INHERITED_POWER,
            .effects = {{
                .func      = eff_inherited_power,
                .name      = "Inherited Power",
                .trigger   = ON_PIECE_COMBINE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_EAGLE_EYE] =
        {
            .name    = "Eagle Eye",
            .desc    = "Enemy pieces are always visible.",
            .id      = RELIC_EAGLE_EYE,
            .effects = {{
                .func      = eff_eagle_eye,
                .name      = "Eagle Eye",
                .trigger   = QUERY_BOARD_STATE,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_SURVEYORS_MAP] =
        {
            .name = "Surveyor's Map",
            .desc =
                "One random battle node's modifier is pre-revealed per map.",
            .id = RELIC_SURVEYORS_MAP,
        },
    [RELIC_FORWARD_COMMAND] =
        {
            .name    = "Forward Command",
            .desc    = "+5 damage while occupying enemy territory.",
            .id      = RELIC_FORWARD_COMMAND,
            .effects = {{
                .func      = eff_forward_command,
                .name      = "Forward Command",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_FORTIFIED_LINE] =
        {
            .name    = "Fortified Line",
            .desc    = "Pieces that did not move this turn deal +5 damage.",
            .id      = RELIC_FORTIFIED_LINE,
            .effects = {{
                .func      = eff_fortified_line,
                .name      = "Fortified Line",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
    [RELIC_WARLORDS_BANNER] =
        {
            .name    = "Warlord's Banner",
            .desc    = "Pieces adjacent to your king deal +5 damage.",
            .id      = RELIC_WARLORDS_BANNER,
            .effects = {{
                .func      = eff_warlords_banner,
                .name      = "Warlord's Banner",
                .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                .lasts_for = ENTIRE_BATTLE,
            }},
        },
};
