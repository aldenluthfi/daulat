//! battle.c
//!
//! Battle engine. Owns the board, the turn loop, all player actions, the
//! resolve and cascade pipeline, the movement generation kit, and the
//! subject registers effects use to self-filter during a firing.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                               SUBJECT REGISTERS
\*----------------------------------------------------------------------------*/

const PieceInfo     VOID_CELL = {};

static BattleState* CURRENT_BATTLE;
static PieceInfo*   SUBJECT;
static PieceInfo*   VICTIM;
static Card*        SUBJECT_CARD;
static const Piece* BUY_PIECE;
static Square       MOVE_FROM    = {-1, -1};
static Square       OWNER_SQUARE = {-1, -1};
static int          CASCADE_ORIGIN;
static PieceInfo**  DAMAGERS;

static EngineState* BATTLE_ENGINE;
static Side         HUMAN_SIDE;
static Side         ACTING_SIDE;
static unsigned int BATTLE_RNG;
static PieceInfo*   FLIPPED_PIECE;
static PieceInfo*   DAMAGER_LIST[MAX_BOARD_SIZE + 1];

#define MARK_MOVED     ((uintptr_t) 1)
#define MARK_FLIPPED   ((uintptr_t) 2)
#define TURN_LAST_MOVE ((uintptr_t) 3)
#define TURN_LAST_FLIP ((uintptr_t) 4)

#define NEXT_HAND      ((uintptr_t) 0x4E455854)
#define NEXT_HAND_BASE 5

static const PieceID COMBO_RESULTS[] = {
    PIECE_SANG,
    PIECE_NORTHERN_CAVALRY,
    PIECE_HWACHA,
    PIECE_MEDEQ_SQUAD,
    PIECE_SULTANS_LEVY,
    PIECE_OLD_KING,
    PIECE_CATAPHRACT,
    PIECE_ROOK,
    PIECE_WAR_ELEPHANT,
    PIECE_HONORABLE_HORSE,
    PIECE_PROMOTED_BISHOP,
    PIECE_DAIMYO,
    PIECE_DRAGON,
    PIECE_CHANCELLOR,
    PIECE_SOVEREIGN_BANNER,
    PIECE_NONE,
};

static const PieceID RECIPES[][3] = {
    {PIECE_BING, PIECE_SABA, PIECE_SANG},
    {PIECE_FARAS, PIECE_BING, PIECE_NORTHERN_CAVALRY},
    {PIECE_PAO, PIECE_BISHOP, PIECE_HWACHA},

    {PIECE_MEDEQ, PIECE_MEDEQ, PIECE_MEDEQ_SQUAD},
    {PIECE_NEGUS_GUARD, PIECE_MEDEQ, PIECE_SULTANS_LEVY},

    {PIECE_MAKWANAM, PIECE_WAZIR, PIECE_OLD_KING},
    {PIECE_KNIGHT, PIECE_JAMAL, PIECE_CATAPHRACT},
    {PIECE_KYOSHA, PIECE_WAZIR, PIECE_ROOK},
    {PIECE_ZIRAAFA, PIECE_JAMAL, PIECE_WAR_ELEPHANT},

    {PIECE_KYOSHA, PIECE_KNIGHT, PIECE_HONORABLE_HORSE},
    {PIECE_BISHOP, PIECE_WAZIR, PIECE_PROMOTED_BISHOP},
    {PIECE_KINSHO, PIECE_GINSHO, PIECE_DAIMYO},
    {PIECE_ROOK, PIECE_WAZIR, PIECE_DRAGON},

    {PIECE_ROOK, PIECE_BISHOP, PIECE_CHANCELLOR},
    {PIECE_QUEEN, PIECE_KYOSHA, PIECE_SOVEREIGN_BANNER},

    {PIECE_NONE, PIECE_NONE, PIECE_NONE},
};

static int        KINGDOM_PLAYS[KINGDOM_COUNT];

static PieceInfo* REAP_LIST[2 * MAX_BOARD_SIZE];
static size_t     REAP_COUNT;

static Card*      PENDING_CARD;
static Side       PENDING_SIDE;
static CardTarget PENDING_TARGETS[MAX_BOARD_SIZE + 1];
static size_t     PENDING_TARGET_COUNT;
static CardTarget PENDING_PICKS[MAX_EFFECT_COUNT + 1];
static size_t     PENDING_STEP;
static size_t     PENDING_STEP_COUNT;

/// battle_reap
///
/// Frees every piece unlinked from the board since the last reap. Removal
/// only unlinks and queues a piece so effects may remove their own piece
/// while effect_fire is still walking the board; the actual free happens
/// here at turn and battle boundaries where no walk is in flight.
///
static void battle_reap(void) {
    for (size_t i = 0; i < REAP_COUNT; i++) {
        PieceInfo* piece = REAP_LIST[i];

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            free(piece->piece->effects[slot].context);
        }

        free(piece->piece);
        free(piece);
    }

    REAP_COUNT = 0;
}

/// battle_current
///
/// Returns the battle the currently firing trigger belongs to. Valid
/// only while effect_fire is walking effects.
///
/// Return: current battle, nullptr outside a firing
///
BattleState* battle_current(void) {
    return CURRENT_BATTLE;
}

/// battle_subject
///
/// Returns the piece the currently firing trigger is about: the mover,
/// the priced piece, the damage dealer, or the flip candidate.
///
/// Return: subject piece, nullptr when the trigger has none
///
PieceInfo* battle_subject(void) {
    return SUBJECT;
}

/// battle_victim
///
/// Returns the piece receiving damage during a damage query. A nullptr
/// during QUERY_PIECE_DAMAGE_DEALT means an effective-value query.
///
/// Return: victim piece, nullptr outside damage queries
///
PieceInfo* battle_victim(void) {
    return VICTIM;
}

/// battle_subject_card
///
/// Returns the card the currently firing card trigger is about.
///
/// Return: subject card, nullptr outside card triggers
///
Card* battle_subject_card(void) {
    return SUBJECT_CARD;
}

/// battle_buy_piece
///
/// Returns the piece template being priced during a buy query, letting
/// pricing effects act on the piece's kingdom or id. Null outside a buy.
///
/// Return: the template under purchase, or null
///
const Piece* battle_buy_piece(void) {
    return BUY_PIECE;
}

/// battle_move_from
///
/// Returns the origin square of the move being reported by the current
/// ON_PIECE_MOVE firing.
///
/// Return: origin square, SQUARE_END outside ON_PIECE_MOVE
///
Square battle_move_from(void) {
    return MOVE_FROM;
}

/// battle_owner_square
///
/// Returns the square whose owner the current QUERY_SQUARE_OWNER firing is
/// resolving, so an effect can condition on which square it edits.
///
/// Return: the queried square, SQUARE_END outside QUERY_SQUARE_OWNER
///
Square battle_owner_square(void) {
    return OWNER_SQUARE;
}

/// battle_damagers
///
/// Returns the null-terminated list of pieces that dealt damage in the
/// current resolve, for post-flip attribution effects.
///
/// Return: damager list, nullptr outside resolve
///
PieceInfo** battle_damagers(void) {
    return DAMAGERS;
}

/// battle_cascade_origin
///
/// Returns the receiver's meter recorded before the current damage and
/// cascade transaction, so an ON_CASCADE_END effect can tell whether the
/// transaction crossed a threshold.
///
/// Return: the pre-transaction meter, valid during ON_CASCADE_END
///
int battle_cascade_origin(void) {
    return CASCADE_ORIGIN;
}

/// battle_run
///
/// Returns the active run so data-file effects can read run state without
/// the private engine pointer.
///
/// Return: the current run, or nullptr outside a run
///
RunState* battle_run(void) {
    return BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;
}

/*----------------------------------------------------------------------------*\
                                 BOARD INDEXING
\*----------------------------------------------------------------------------*/

/// square_index
///
/// Converts a square to its board cell index at the fixed stride of
/// twenty cells per row.
///
/// Params:
/// - square -> square to convert
///
/// Return: cell index into piece_board
///
static size_t square_index(Square square) {
    return (size_t) square.y * 20 + (size_t) square.x;
}

/*----------------------------------------------------------------------------*\
                                   LIFECYCLE
\*----------------------------------------------------------------------------*/

/// battle_player
///
/// Returns the player state of the given side.
///
/// Params:
/// - battle -> battle to look into
/// - side   -> side to fetch
///
/// Return: that side's player state
///
PlayerState* battle_player(BattleState* battle, Side side) {
    return side == SIDE_WHITE ? &battle->white : &battle->black;
}

/// battle_enemy
///
/// Returns the side opposing the given side.
///
/// Params:
/// - side -> side to oppose
///
/// Return: the opposing side
///
Side battle_enemy(Side side) {
    return side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
}

/// battle_rand
///
/// Draws the next value from the battle's deterministic RNG stream, the
/// shared source for every randomised effect.
///
/// Return: a pseudo-random unsigned int
///
unsigned int battle_rand(void) {
    return (unsigned int) rand_r(&BATTLE_RNG);
}

/// battle_draw_pool
///
/// Builds the side's eligible draw pool into out: every unlocked,
/// registered card that passes QUERY_CARD_CAN_DRAW. The subject-card and
/// battle registers are saved and restored so the build is side-effect
/// free for callers firing mid-effect.
///
/// Params:
/// - battle -> battle whose run gates the pool
/// - side   -> side the draw eligibility fires for
/// - out    -> CARD_COUNT-sized buffer receiving the eligible ids
///
/// Return: the number of eligible cards written to out
///
size_t battle_draw_pool(BattleState* battle, Side side, CardID* out) {
    BattleState* saved_battle = CURRENT_BATTLE;
    Card*        saved_card   = SUBJECT_CARD;
    size_t       count        = 0;

    CURRENT_BATTLE            = battle;

    for (CardID id = 0; id < CARD_COUNT; id++) {
        if (!BATTLE_ENGINE->run->cards[id] || !CARD_REGISTRY[id]) {
            continue;
        }

        bool can     = true;

        SUBJECT_CARD = (Card*) CARD_REGISTRY[id];
        effect_fire(battle, side, QUERY_CARD_CAN_DRAW, &can);

        if (can) {
            out[count] = id;
            count++;
        }
    }

    SUBJECT_CARD   = saved_card;
    CURRENT_BATTLE = saved_battle;

    return count;
}

/// next_hand_mark
///
/// Finds, or lazily attaches, the human seat's battle-long NEXT_HAND mark.
/// The mark stores the projected upcoming hand: args[2] the count, args[3]
/// the ready flag, and args[NEXT_HAND_BASE + i] each projected Card*.
///
/// Params:
/// - battle -> battle whose human seat owns the mark
/// - side   -> human seat the mark lives on
///
/// Return: the mark effect, nullptr on allocation failure
///
static Effect* next_hand_mark(BattleState* battle, Side side) {
    LinkedList* list =
        &battle_player(battle, side)->effects;
    Effect*     mark =
        effect_find_mark(list, NEXT_HAND, (void*) (uintptr_t) side);

    if (mark) {
        return mark;
    }

    Effect seed = {
        .func      = eff_noop,
        .trigger   = ON_TURN_START,
        .lasts_for = ENTIRE_BATTLE,
    };

    mark = effect_attach(list, &seed);

    if (mark) {
        mark->context->args[0] = (void*) (uintptr_t) side;
        mark->context->args[1] = (void*) NEXT_HAND;
    }

    return mark;
}

/// battle_roll_cards
///
/// Samples count borrowed cards from the side's eligible pool into out,
/// each drawn with replacement from BATTLE_RNG. An empty pool leaves the
/// remaining slots null. Shared by the projection build, the projection
/// refill, and the immediate bonus draw.
///
/// Params:
/// - battle -> battle whose pool is sampled
/// - side   -> side the draw eligibility fires for
/// - count  -> number of cards to sample
/// - out    -> buffer receiving count borrowed Card*
///
static void battle_roll_cards(
    BattleState* battle,
    Side         side,
    size_t       count,
    Card**       out
) {
    CardID pool[CARD_COUNT];
    size_t pool_size = battle_draw_pool(battle, side, pool);

    for (size_t drawn = 0; drawn < count; drawn++) {
        if (pool_size == 0) {
            out[drawn] = nullptr;
            continue;
        }

        CardID pick = pool[(size_t) rand_r(&BATTLE_RNG) % pool_size];

        out[drawn]  = (Card*) CARD_REGISTRY[pick];
    }
}

/// battle_project_hand
///
/// Pre-rolls the human seat's upcoming hand into the NEXT_HAND mark when it
/// is not already ready: the queried draw count of cards, then fires
/// QUERY_NEXT_HAND so preview effects edit the projection before it is
/// realized. Idempotent while the mark is ready; a no-op off the human seat.
///
/// Params:
/// - battle -> battle whose projection is built
/// - side   -> side to project for
///
static void battle_project_hand(BattleState* battle, Side side) {
    if (side != HUMAN_SIDE) {
        return;
    }

    Effect* mark = next_hand_mark(battle, side);

    if (!mark || mark->context->args[3]) {
        return;
    }

    BattleState* saved_battle = CURRENT_BATTLE;

    CURRENT_BATTLE            = battle;

    int draw                  = 3;
    effect_fire(battle, side, QUERY_CARD_DRAW_COUNT, &draw);

    if (draw < 0) {
        draw = 0;
    }

    if (draw > MAX_DRAWN_CARDS) {
        draw = MAX_DRAWN_CARDS;
    }

    Card** cards           = (Card**) &mark->context->args[NEXT_HAND_BASE];
    battle_roll_cards(battle, side, (size_t) draw, cards);

    mark->context->args[2] = (void*) (uintptr_t) draw;
    mark->context->args[3] = (void*) 1;

    effect_fire(battle, side, QUERY_NEXT_HAND, cards);

    CURRENT_BATTLE         = saved_battle;
}

/// battle_realize_hand
///
/// Moves the ready NEXT_HAND projection verbatim into the human seat's hand,
/// clears the mark, and fires ON_CARDS_DRAWN on the realized hand. Because
/// the projection was already edited by every preview effect, the realized
/// hand equals what the player previewed.
///
/// Params:
/// - battle -> battle whose hand is filled
/// - side   -> side realizing its projection
///
static void battle_realize_hand(BattleState* battle, Side side) {
    if (side != HUMAN_SIDE) {
        return;
    }

    Effect* mark = next_hand_mark(battle, side);

    if (!mark || !mark->context->args[3]) {
        return;
    }

    PlayerState* player = battle_player(battle, side);
    Card**       cards  = (Card**) &mark->context->args[NEXT_HAND_BASE];
    size_t       count  = (size_t) (uintptr_t) mark->context->args[2];
    bool         drew   = false;

    for (size_t index = 0; index < count; index++) {
        if (!cards[index]) {
            continue;
        }

        size_t slot = 0;

        while (slot < MAX_DRAWN_CARDS && player->hand[slot]) {
            slot++;
        }

        if (slot >= MAX_DRAWN_CARDS) {
            break;
        }

        player->hand[slot] = cards[index];
        drew               = true;
    }

    mark->context->args[2] = 0;
    mark->context->args[3] = 0;

    if (drew) {
        BattleState* saved_battle = CURRENT_BATTLE;

        CURRENT_BATTLE            = battle;
        effect_fire(battle, side, ON_CARDS_DRAWN, player->hand);
        CURRENT_BATTLE            = saved_battle;
    }
}

/// battle_next_hand
///
/// Returns the human seat's mutable projected hand and its count, or nullptr
/// when no projection is ready. Preview effects and the protocol read and
/// edit the cards in place through this view.
///
/// Params:
/// - battle -> battle holding the projection mark
/// - side   -> human seat whose projection is read
/// - count  -> receives the projected card count
///
/// Return: the projected card list, nullptr when none is ready
///
Card** battle_next_hand(BattleState* battle, Side side, size_t* count) {
    Effect* mark = next_hand_mark(battle, side);

    if (!mark || !mark->context->args[3]) {
        if (count) {
            *count = 0;
        }

        return nullptr;
    }

    if (count) {
        *count = (size_t) (uintptr_t) mark->context->args[2];
    }

    return (Card**) &mark->context->args[NEXT_HAND_BASE];
}

/// battle_next_hand_discard
///
/// Drops one projected slot and refills the tail from the same deterministic
/// stream, so the projection keeps its count. When the pool is exhausted the
/// count shrinks by one instead. Powers Counsel and Librarian's Notes.
///
/// Params:
/// - battle -> battle holding the projection mark
/// - side   -> human seat whose projection is edited
/// - slot   -> projected index to discard
///
void battle_next_hand_discard(BattleState* battle, Side side, size_t slot) {
    size_t count;
    Card** cards = battle_next_hand(battle, side, &count);

    if (!cards || slot >= count) {
        return;
    }

    for (size_t index = slot; index + 1 < count; index++) {
        cards[index] = cards[index + 1];
    }

    Card* refill = nullptr;
    battle_roll_cards(battle, side, 1, &refill);

    if (refill) {
        cards[count - 1] = refill;
        return;
    }

    Effect* mark           = next_hand_mark(battle, side);
    mark->context->args[2] = (void*) (uintptr_t) (count - 1);
}

/// battle_item_list
///
/// Writes a side's activatable held effects, those carrying an
/// ON_ITEM_ACTIVATE trigger, into out in stable list order up to cap.
///
/// Params:
/// - battle -> battle whose seat is walked
/// - side   -> seat holding the items
/// - out    -> buffer receiving the activatable effects
/// - cap    -> capacity of out
///
/// Return: the number of activatable items found
///
size_t battle_item_list(
    BattleState* battle,
    Side         side,
    Effect**     out,
    size_t       cap
) {
    LinkedList* list  = &battle_player(battle, side)->effects;
    size_t      count = 0;

    for (LLNode* node = list->head; node; node = node->next) {
        Effect* effect = node->data;

        if (effect->func && effect->trigger == ON_ITEM_ACTIVATE) {
            if (count < cap) {
                out[count] = effect;
            }

            count++;
        }
    }

    return count;
}

/// battle_item_usable
///
/// Reports whether an activatable item may fire now, reading its own
/// once-per-battle flag in args[0] and once-per-turn stamp in args[1].
///
/// Params:
/// - battle -> battle supplying the current turn
/// - item   -> activatable effect to test
///
/// Return: true when the item is not yet spent for its scope
///
bool battle_item_usable(BattleState* battle, Effect* item) {
    if (!item) {
        return false;
    }

    bool   used_battle = item->context->args[0];
    size_t used_turn   = (size_t) (uintptr_t) item->context->args[1];

    return !used_battle && used_turn != battle->turn + 1;
}

/// battle_item_activate
///
/// Fires the index-th activatable item for the side when it is still usable,
/// passing the seat as x and logging the applied activation.
///
/// Params:
/// - battle -> battle to act in
/// - side   -> seat holding the item
/// - index  -> ordinal in battle_item_list order
///
/// Return: true when the item activated
///
bool battle_item_activate(BattleState* battle, Side side, size_t index) {
    LinkedList* list    = &battle_player(battle, side)->effects;
    size_t      ordinal = 0;

    for (LLNode* node = list->head; node; node = node->next) {
        Effect* effect = node->data;

        if (!effect->func || effect->trigger != ON_ITEM_ACTIVATE) {
            continue;
        }

        if (ordinal++ != index) {
            continue;
        }

        if (!battle_item_usable(battle, effect)) {
            return false;
        }

        BattleState* saved_battle = CURRENT_BATTLE;
        CURRENT_BATTLE            = battle;

        bool applied              =
            effect->func(effect->context, (void*) (uintptr_t) side);

        CURRENT_BATTLE            = saved_battle;

        if (applied && effect->name) {
            protocol_emit(
                "log effect name=\"%s\" trigger=%s",
                effect->name,
                effect_trigger_name(ON_ITEM_ACTIVATE)
            );
        }

        return applied;
    }

    return false;
}

/// battle_find_king
///
/// Finds a side's king on the board.
///
/// Params:
/// - battle -> battle to search
/// - side   -> owning side
///
/// Return: the king, nullptr when it is not on the board
///
PieceInfo* battle_find_king(BattleState* battle, Side side) {
    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side &&
            cell->piece->id == PIECE_KING) {
            return cell;
        }
    }

    return nullptr;
}

/// battle_meter_gain
///
/// Adds to a side's meter and clamps it to the two hundred percent
/// ceiling, the shared path for card and effect meter rewards.
///
/// Params:
/// - battle -> battle the side plays in
/// - side   -> side gaining meter
/// - amount -> meter to add, may be negative
///
void battle_meter_gain(BattleState* battle, Side side, int amount) {
    PlayerState* player  = battle_player(battle, side);
    int          max     = battle_meter_max(battle, side);

    player->meter       += amount;

    if (player->meter > 2 * max) {
        player->meter = 2 * max;
    }
}

/// battle_draw
///
/// Immediately rolls count cards into the side's hand, the bonus-draw path
/// used by active items such as Deep Hand. The regular per-turn hand comes
/// from the realized NEXT_HAND projection instead. Only the human holds a
/// hand; the AI plays no cards in this build.
///
/// Params:
/// - battle -> battle whose hand is filled
/// - side   -> side drawing cards
/// - count  -> number of cards to draw
///
void battle_draw(BattleState* battle, Side side, size_t count) {
    if (side != HUMAN_SIDE) {
        return;
    }

    if (count > MAX_DRAWN_CARDS) {
        count = MAX_DRAWN_CARDS;
    }

    BattleState* saved_battle = CURRENT_BATTLE;
    Card*        saved_card   = SUBJECT_CARD;
    PlayerState* player       = battle_player(battle, side);

    CURRENT_BATTLE            = battle;

    Card* rolled[MAX_DRAWN_CARDS];
    battle_roll_cards(battle, side, count, rolled);

    bool drew = false;

    for (size_t drawn = 0; drawn < count; drawn++) {
        if (!rolled[drawn]) {
            continue;
        }

        size_t slot = 0;

        while (slot < MAX_DRAWN_CARDS && player->hand[slot]) {
            slot++;
        }

        if (slot >= MAX_DRAWN_CARDS) {
            break;
        }

        player->hand[slot] = rolled[drawn];
        drew               = true;
    }

    if (drew) {
        effect_fire(battle, side, ON_CARDS_DRAWN, player->hand);
    }

    SUBJECT_CARD   = saved_card;
    CURRENT_BATTLE = saved_battle;
}

/// turn_start
///
/// Starts a side's turn: restores actions, applies queried income, fires
/// ON_TURN_START, resets the same-kingdom combo counters, and draws the
/// side's hand.
///
/// Params:
/// - battle -> battle whose turn starts
/// - side   -> side beginning its turn
///
static void turn_start(BattleState* battle, Side side) {
    PlayerState* player = battle_player(battle, side);

    battle_reap();

    effect_tick(&player->effects);

    player->actions = 3;

    int income      = 10;

    CURRENT_BATTLE  = battle;
    effect_fire(battle, side, QUERY_CP_INCOME, &income);

    player->cp += income;

    if (player->cp < 0) {
        player->cp = 0;
    }

    effect_fire(battle, side, ON_TURN_START, (void*) (uintptr_t) battle->turn);
    CURRENT_BATTLE = nullptr;

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        KINGDOM_PLAYS[kingdom] = 0;
    }

    battle_project_hand(battle, side);
    battle_realize_hand(battle, side);
    battle_project_hand(battle, side);
}

/// battle_finish
///
/// Ends the battle: emits the result line, frees everything the battle
/// owns, and detaches it from the engine.
///
/// Params:
/// - battle -> battle to finish
/// - winner -> side that won
///
static void battle_finish(BattleState* battle, Side winner) {
    CURRENT_BATTLE = battle;
    effect_fire(battle, winner, ON_BATTLE_END, (void*) (uintptr_t) winner);
    CURRENT_BATTLE = nullptr;

    protocol_emit("result won=%d", winner == HUMAN_SIDE ? 1 : 0);

    battle_free(battle);
    free(battle);

    BATTLE_ENGINE->battle = nullptr;

    run_battle_result(BATTLE_ENGINE, winner == HUMAN_SIDE);
}

/// battle_concede
///
/// Ends the battle immediately as a loss for the human, running the
/// standard finish and result pipeline.
///
/// Params:
/// - battle -> battle to concede
///
void battle_concede(BattleState* battle) {
    battle_finish(battle, HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE);
}

/// battle_free
///
/// Frees everything the battle owns: every live piece with its heap
/// piece copy and effect contexts, and both player effect lists.
///
/// Params:
/// - battle -> battle to deallocate
///
void battle_free(BattleState* battle) {
    battle_reap();

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL) {
            battle_remove(battle, cell);
        }
    }

    battle_reap();

    effect_clear(&battle->white.effects);
    effect_clear(&battle->black.effects);
}

/// battle_spawn
///
/// Places a new piece on the board by heap-copying the registry template
/// and allocating a zeroed context for each of its embedded effects,
/// storing the owning piece info in args[0] of each context.
///
/// Params:
/// - battle -> battle to spawn into
/// - id     -> registry piece to copy
/// - at     -> destination square, must be empty
/// - side   -> owning side
///
/// Return: the spawned piece, nullptr when the spawn is invalid
///
PieceInfo* battle_spawn(BattleState* battle, PieceID id, Square at, Side side) {
    const Piece* template = PIECE_REGISTRY[id];

    if (!template || !battle_in_bounds(battle, at) || battle_at(battle, at)) {
        return nullptr;
    }

    PieceInfo* info = malloc(sizeof(PieceInfo));
    Piece*     copy = malloc(sizeof(Piece));

    if (!info || !copy) {
        free(info);
        free(copy);
        return nullptr;
    }

    *copy = *template;

    if (BATTLE_ENGINE && side == HUMAN_SIDE) {
        const Piece* saved_buy    = BUY_PIECE;
        BattleState* saved_battle = CURRENT_BATTLE;
        int          value        = copy->value;

        BUY_PIECE                 = copy;
        CURRENT_BATTLE            = battle;
        effect_fire(battle, side, QUERY_PIECE_VALUE, &value);
        CURRENT_BATTLE = saved_battle;
        BUY_PIECE      = saved_buy;

        copy->value    = value;
    }

    info->piece  = copy;
    info->square = at;
    info->side   = side;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (!copy->effects[slot].func) {
            continue;
        }

        copy->effects[slot].context          = calloc(1, sizeof(EffectContext));
        copy->effects[slot].context->args[0] = info;
    }

    battle->board.piece_board[square_index(at)] = info;

    return info;
}

/// piece_mark
///
/// Reads a piece's magic counter mark for a tag: a noop mark carrying the
/// tag in args[1] and the value in args[2]. Backs the move and flip
/// counters and their last-turn stamps.
///
/// Params:
/// - piece -> piece to read
/// - tag   -> magic tag (MARK_MOVED, MARK_FLIPPED, TURN_LAST_MOVE, ...)
///
/// Return: the stored value, zero when the mark is absent
///
static int piece_mark(PieceInfo* piece, uintptr_t tag) {
    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        Effect* effect = &piece->piece->effects[slot];

        if (effect->func == eff_noop && effect->context &&
            (uintptr_t) effect->context->args[1] == tag) {
            return (int) (uintptr_t) effect->context->args[2];
        }
    }

    return 0;
}

/// piece_mark_set
///
/// Stores a value on a piece's magic counter mark for a tag, embedding the
/// noop mark on first use. battle_move and battle_flip stamp the counts and
/// last-turn marks through it.
///
/// Params:
/// - piece -> piece to stamp
/// - tag   -> magic tag
/// - value -> value to store
///
static void piece_mark_set(PieceInfo* piece, uintptr_t tag, int value) {
    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        Effect* effect = &piece->piece->effects[slot];

        if (effect->func == eff_noop && effect->context &&
            (uintptr_t) effect->context->args[1] == tag) {
            effect->context->args[2] = (void*) (uintptr_t) value;

            return;
        }
    }

    Effect mark = {
        .func      = eff_noop,
        .trigger   = ON_BATTLE_START,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* embedded = piece_embed_effect(piece, &mark);

    if (embedded) {
        embedded->context->args[1] = (void*) tag;
        embedded->context->args[2] = (void*) (uintptr_t) value;
    }
}

/// battle_piece_move_turn
///
/// Returns the turn on which a piece last moved, zero when it never moved,
/// from its TURN_LAST_MOVE mark. Consumers needing "moved this turn" or
/// "moved last turn" compare it to battle->turn.
///
/// Params:
/// - piece -> piece to query
///
/// Return: the piece's last-move turn, zero when it never moved
///
size_t battle_piece_move_turn(PieceInfo* piece) {
    return (size_t) piece_mark(piece, TURN_LAST_MOVE);
}

/// battle_piece_flip_turn
///
/// Returns the turn on which a piece last flipped, zero when it never
/// flipped, from its TURN_LAST_FLIP mark.
///
/// Params:
/// - piece -> piece to query
///
/// Return: the piece's last-flip turn, zero when it never flipped
///
size_t battle_piece_flip_turn(PieceInfo* piece) {
    return (size_t) piece_mark(piece, TURN_LAST_FLIP);
}

/// battle_piece_moves
///
/// Returns how many times a piece has moved this battle: the base is its
/// MARK_MOVED mark, then QUERY_PIECE_HAS_MOVED fires so effects may adjust.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to query
///
/// Return: the piece's move count
///
int battle_piece_moves(BattleState* battle, PieceInfo* piece) {
    PieceInfo* saved = SUBJECT;
    int        count = piece_mark(piece, MARK_MOVED);

    SUBJECT          = piece;
    effect_fire(battle, piece->side, QUERY_PIECE_HAS_MOVED, &count);
    SUBJECT = saved;

    return count;
}

/// battle_piece_flips
///
/// Returns how many times a piece has flipped this battle: the base is its
/// MARK_FLIPPED mark, then QUERY_PIECE_HAS_FLIPPED fires so effects may
/// adjust.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to query
///
/// Return: the piece's flip count
///
int battle_piece_flips(BattleState* battle, PieceInfo* piece) {
    PieceInfo* saved = SUBJECT;
    int        count = piece_mark(piece, MARK_FLIPPED);

    SUBJECT          = piece;
    effect_fire(battle, piece->side, QUERY_PIECE_HAS_FLIPPED, &count);
    SUBJECT = saved;

    return count;
}

/// battle_flip
///
/// Flips a piece to the opposing side, firing ON_PIECE_FLIP_PRE before
/// the toggle so effects may redirect or consume it, and ON_PIECE_FLIP
/// right after the toggle when it happened. ON_PIECE_FLIP_POST is fired
/// by the cascade once the step settles; callers flipping outside the
/// cascade fire it themselves.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to flip
///
void battle_flip(BattleState* battle, PieceInfo* piece) {
    PieceInfo* pick = piece;

    FLIPPED_PIECE   = nullptr;
    CURRENT_BATTLE  = battle;
    SUBJECT         = piece;

    effect_fire(battle, piece->side, ON_PIECE_FLIP_PRE, &pick);

    if (pick) {
        pick->side    = pick->side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

        FLIPPED_PIECE = pick;

        protocol_emit(
            "log flip piece=%s x=%d y=%d",
            pick->piece->name,
            pick->square.x,
            pick->square.y
        );

        piece_mark_set(pick, MARK_FLIPPED, piece_mark(pick, MARK_FLIPPED) + 1);
        piece_mark_set(pick, TURN_LAST_FLIP, (int) battle->turn);
        effect_fire(battle, pick->side, ON_PIECE_FLIP, pick);
    }

    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;
}

/// battle_swap
///
/// Exchanges the board squares of two pieces, the shared primitive behind
/// the king-swap moves (Shahzadeh, Royal Substitution). Both cells and
/// both stored squares are updated together.
///
/// Params:
/// - battle -> battle the pieces live in
/// - a      -> first piece
/// - b      -> second piece
///
void battle_swap(BattleState* battle, PieceInfo* a, PieceInfo* b) {
    Square square_a                                   = a->square;
    Square square_b                                   = b->square;

    battle->board.piece_board[square_index(square_a)] = b;
    battle->board.piece_board[square_index(square_b)] = a;

    a->square                                         = square_b;
    b->square                                         = square_a;
}

/// battle_remove
///
/// Removes a piece from the board by unlinking its cell and queuing it for
/// the next reap. Unlinking is immediate so meter, movement, and spawning
/// see the removal at once; the free is deferred so an effect may remove
/// its own piece while effect_fire is still walking the board.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to remove
///
void battle_remove(BattleState* battle, PieceInfo* piece) {
    battle->board.piece_board[square_index(piece->square)] = nullptr;

    if (REAP_COUNT < 2 * MAX_BOARD_SIZE) {
        REAP_LIST[REAP_COUNT] = piece;
        REAP_COUNT++;
    }
}

/*----------------------------------------------------------------------------*\
                                    ACTIONS
\*----------------------------------------------------------------------------*/

/// battle_move
///
/// Moves an acting-side piece if the destination is in its generated
/// move list and the player can pay the queried action cost. Fires
/// ON_PIECE_MOVE with the origin square in the move register.
///
/// Params:
/// - battle -> battle to act in
/// - from   -> square of the piece to move
/// - to     -> destination square
///
/// Return: true when the move was performed
///
bool battle_move(BattleState* battle, Square from, Square to) {
    PieceInfo* piece = battle_at(battle, from);

    if (!piece || piece->side != ACTING_SIDE) {
        return false;
    }

    Square* moves = battle_moves(battle, piece);
    bool    legal = false;

    for (size_t i = 0; !(moves[i].x == -1 && moves[i].y == -1); i++) {
        if (moves[i].x == to.x && moves[i].y == to.y) {
            legal = true;
            break;
        }
    }

    if (!legal) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);

    int          cost   = 1;

    CURRENT_BATTLE      = battle;
    SUBJECT             = piece;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_ACTION_COST_MOVE, &cost);

    if (player->actions < cost) {
        CURRENT_BATTLE = nullptr;
        SUBJECT        = nullptr;
        return false;
    }

    player->actions     -= cost;

    PieceInfo* occupant  = battle_at(battle, to);

    if (occupant && occupant->side == piece->side) {
        battle->board.piece_board[square_index(from)] = occupant;
        battle->board.piece_board[square_index(to)]   = piece;

        occupant->square                              = from;
        piece->square                                 = to;
    } else {
        battle->board.piece_board[square_index(from)] = nullptr;
        piece->square                                 = to;
        battle->board.piece_board[square_index(to)]   = piece;
    }

    MOVE_FROM = from;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_MOVE, piece);
    MOVE_FROM = SQUARE_END;

    piece_mark_set(piece, MARK_MOVED, piece_mark(piece, MARK_MOVED) + 1);
    piece_mark_set(piece, TURN_LAST_MOVE, (int) battle->turn);

    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;

    return true;
}

/// battle_price
///
/// Computes the base currency cost of a piece before the buy queries,
/// applying the region rule: pieces native to the battle's home kingdom
/// cost forty percent less, foreign pieces twenty percent more. The king
/// and kingdomless pieces price at face value. The foreign markup is a
/// QUERY_PIECE_CP_COST_BUY seam; the Trade Routes relic drops it as an
/// effect there, naming no item here.
///
/// Params:
/// - battle   -> battle whose node fixes the home kingdom
/// - template -> piece template being priced
///
/// Return: the region-adjusted base cost
///
static int battle_price(BattleState* battle, const Piece* template) {
    int cost = template->value;

    if (template->kingdom == KINGDOM_NONE || !battle->node ||
        !battle->node->kingdom) {
        return cost;
    }

    if (template->kingdom == battle->node->kingdom->id) {
        return cost * 60 / 100;
    }

    return cost * 120 / 100;
}

/// battle_buy
///
/// Buys a piece onto any empty square, applying the pricing and action
/// cost queries, then fires ON_PIECE_BUY. The king and combination
/// results are never in the shop; the human seat may only buy pieces
/// unlocked in the run.
///
/// Params:
/// - battle -> battle to act in
/// - id     -> piece to buy
/// - at     -> destination square
///
/// Return: true when the purchase was performed
///
/// battle_is_recipe_result
///
/// Reports whether a piece is a combination result, which are never sold
/// in the shop and are the pieces an Archive node reveals.
///
/// Params:
/// - id -> piece to test
///
/// Return: true when the piece appears as a recipe result
///
bool battle_is_recipe_result(PieceID id) {
    for (size_t i = 0; COMBO_RESULTS[i] != PIECE_NONE; i++) {
        if (COMBO_RESULTS[i] == id) {
            return true;
        }
    }

    return false;
}

/// battle_piece_unlocked
///
/// Reports whether the run has unlocked a piece identity, exposing the
/// battle.c-private run pointer so card effects can offer only unlocked
/// identities without naming the engine.
///
/// Params:
/// - id -> piece identity to test
///
/// Return: true when the run has unlocked the identity
///
bool battle_piece_unlocked(PieceID id) {
    return id < PIECE_COUNT && BATTLE_ENGINE->run->pieces[id];
}

bool battle_buy(BattleState* battle, PieceID id, Square at) {
    const Piece* template = PIECE_REGISTRY[id];

    if (!template || id == PIECE_KING) {
        return false;
    }

    if (battle_is_recipe_result(id)) {
        return false;
    }

    if (ACTING_SIDE == HUMAN_SIDE && !BATTLE_ENGINE->run->pieces[id]) {
        return false;
    }

    if (ACTING_SIDE == HUMAN_SIDE) {
        bool allowed   = true;

        CURRENT_BATTLE = battle;
        BUY_PIECE      = template;
        effect_fire(battle, HUMAN_SIDE, QUERY_PIECE_CAN_BUY, &allowed);
        CURRENT_BATTLE = nullptr;

        if (!allowed) {
            return false;
        }
    }

    PlayerState* player      = battle_player(battle, ACTING_SIDE);

    int          cost        = battle_price(battle, template);
    int          action_cost = 1;

    CURRENT_BATTLE           = battle;
    BUY_PIECE                = template;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_CP_COST_BUY, &cost);
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_ACTION_COST_BUY, &action_cost);
    BUY_PIECE      = nullptr;
    CURRENT_BATTLE = nullptr;

    if (player->cp < cost || player->actions < action_cost) {
        return false;
    }

    PieceInfo* piece = battle_spawn(battle, id, at, ACTING_SIDE);

    if (!piece) {
        return false;
    }

    player->cp      -= cost;
    player->actions -= action_cost;
    player->meter   += battle_value(battle, piece, nullptr);

    CURRENT_BATTLE   = battle;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_BUY, piece);
    CURRENT_BATTLE = nullptr;

    return true;
}

/// battle_combine
///
/// Combines the pieces on the two squares when they match an unlocked
/// recipe, removing both parents and spawning the result on the second
/// square, then fires ON_PIECE_COMBINE.
///
/// Params:
/// - battle -> battle to act in
/// - a      -> square of the first parent
/// - b      -> square of the second parent and result destination
///
/// Return: true when the combination was performed
///
bool battle_combine(BattleState* battle, Square a, Square b) {
    PieceInfo* first  = battle_at(battle, a);
    PieceInfo* second = battle_at(battle, b);

    if (!first || !second || first == second || first->side != ACTING_SIDE ||
        second->side != ACTING_SIDE) {
        return false;
    }

    PieceID result = PIECE_NONE;

    for (size_t i = 0; RECIPES[i][2] != PIECE_NONE; i++) {
        PieceID left  = RECIPES[i][0];
        PieceID right = RECIPES[i][1];

        if ((first->piece->id == left && second->piece->id == right) ||
            (first->piece->id == right && second->piece->id == left)) {
            result = RECIPES[i][2];
            break;
        }
    }

    if (result == PIECE_NONE) {
        return false;
    }

    if (ACTING_SIDE == HUMAN_SIDE && !BATTLE_ENGINE->run->pieces[result]) {
        return false;
    }

    PlayerState* player      = battle_player(battle, ACTING_SIDE);

    int          action_cost = 1;

    CURRENT_BATTLE           = battle;
    effect_fire(
        battle,
        ACTING_SIDE,
        QUERY_PIECE_ACTION_COST_COMBINE,
        &action_cost
    );
    CURRENT_BATTLE = nullptr;

    if (player->actions < action_cost) {
        return false;
    }

    int first_value  = battle_value(battle, first, nullptr);
    int second_value = battle_value(battle, second, nullptr);

    battle_remove(battle, first);
    battle_remove(battle, second);

    PieceInfo* piece = battle_spawn(battle, result, b, ACTING_SIDE);

    if (!piece) {
        return false;
    }

    player->actions -= action_cost;
    player->meter +=
        battle_value(battle, piece, nullptr) - first_value - second_value;

    CURRENT_BATTLE = battle;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_COMBINE, piece);
    CURRENT_BATTLE = nullptr;

    return true;
}

/// battle_card_step_count
///
/// Counts a card's QUERY_CARD_TARGETS effect slots, the number of target
/// selection steps the two-step play flow must gather before resolving.
///
/// Params:
/// - card -> card to test
///
/// Return: the number of target steps, zero when the card needs no target
///
static size_t battle_card_step_count(const Card* card) {
    size_t count = 0;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (card->effects[slot].func &&
            card->effects[slot].trigger == QUERY_CARD_TARGETS) {
            count++;
        }
    }

    return count;
}

/// battle_build_step
///
/// Fills PENDING_TARGETS with the legal targets for one selection step by
/// firing the step-th QUERY_CARD_TARGETS effect. That effect may read the
/// picks already gathered through battle_pending_picks.
///
/// Params:
/// - card -> targeting card
/// - step -> zero based ordinal among the QUERY_CARD_TARGETS slots
/// - side -> acting side, passed to the effect as args[0]
///
/// Return: the number of legal targets advertised for the step
///
static size_t battle_build_step(Card* card, size_t step, Side side) {
    PENDING_TARGETS[0].kind = TARGET_NONE;

    size_t ordinal          = 0;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        const Effect* effect = &card->effects[slot];

        if (!effect->func || effect->trigger != QUERY_CARD_TARGETS) {
            continue;
        }

        if (ordinal++ != step) {
            continue;
        }

        EffectContext ctx = {};

        ctx.args[0]       = (void*) (uintptr_t) side;

        effect->func(&ctx, PENDING_TARGETS);
        break;
    }

    size_t count = 0;

    while (PENDING_TARGETS[count].kind != TARGET_NONE) {
        count++;
    }

    return count;
}

/// battle_emit_targets
///
/// Emits the current PENDING_TARGETS list on the protocol so the caller can
/// pick one by index, shared by play and each further selection step.
///
/// Params:
/// - count -> number of live targets in PENDING_TARGETS
///
static void battle_emit_targets(size_t count) {
    for (size_t i = 0; i < count; i++) {
        protocol_emit(
            "target i=%zu kind=%d value=%d",
            i,
            PENDING_TARGETS[i].kind,
            PENDING_TARGETS[i].value
        );
    }
}

/// battle_pending_picks
///
/// Exposes the TARGET_NONE terminated list of targets chosen so far for the
/// parked card, so a later step's QUERY_CARD_TARGETS effect can exclude or
/// build on earlier picks.
///
/// Return: the pending picks list
///
const CardTarget* battle_pending_picks(void) {
    return PENDING_PICKS;
}

/// battle_card_can_play
///
/// Reports whether the hand card is playable now: it exists, its queried
/// play cost is affordable, no QUERY_CARD_CAN_PLAY effect vetoes it, and
/// every selection step it needs offers a target. Steps are dry-run in
/// order, greedily recording the first target of each so a later step that
/// builds on an earlier pick sees one; a step that offers nothing makes the
/// card unplayable, so the play never parks on a dead step. The pending
/// target state is saved and restored so an in-progress selection is
/// untouched. The protocol calls this to flag unplayable cards; battle_play
/// gates on it.
///
/// Params:
/// - battle -> battle to test in
/// - hand   -> hand index of the card
///
/// Return: true when the card can be played to completion
///
bool battle_card_can_play(BattleState* battle, size_t hand) {
    if (hand >= MAX_DRAWN_CARDS) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);
    Card*        card   = player->hand[hand];

    if (!card) {
        return false;
    }

    int cost       = card->play_cost;

    CURRENT_BATTLE = battle;
    SUBJECT_CARD   = card;
    effect_fire(battle, ACTING_SIDE, QUERY_CARD_PLAY_COST, &cost);

    bool   can   = player->cp >= cost;
    size_t steps = battle_card_step_count(card);

    if (can) {
        effect_fire(battle, ACTING_SIDE, QUERY_CARD_CAN_PLAY, &can);
    }

    if (can && steps > 0) {
        CardTarget saved_picks[MAX_EFFECT_COUNT + 1];
        size_t     saved_count = PENDING_TARGET_COUNT;

        for (size_t i = 0; i <= MAX_EFFECT_COUNT; i++) {
            saved_picks[i] = PENDING_PICKS[i];
        }

        PENDING_PICKS[0] = (CardTarget){TARGET_NONE, 0};

        for (size_t step = 0; can && step < steps; step++) {
            if (battle_build_step(card, step, ACTING_SIDE) == 0) {
                can = false;
                break;
            }

            PENDING_PICKS[step]     = PENDING_TARGETS[0];
            PENDING_PICKS[step + 1] = (CardTarget){TARGET_NONE, 0};
        }

        for (size_t i = 0; i <= MAX_EFFECT_COUNT; i++) {
            PENDING_PICKS[i] = saved_picks[i];
        }

        PENDING_TARGET_COUNT = saved_count;
    }

    SUBJECT_CARD   = nullptr;
    CURRENT_BATTLE = nullptr;

    return can;
}

/// battle_play
///
/// Plays a card from the hand, paying its queried play cost, running its
/// immediate effects and attaching its durational ones, then fires
/// ON_CARD_PLAY and advances the same-kingdom combo chain. A card with one
/// or more QUERY_CARD_TARGETS effects defers its resolution to
/// battle_card_target, parking the card and emitting the first step's
/// advertised targets.
///
/// Params:
/// - battle -> battle to act in
/// - hand   -> hand index of the card to play
///
/// Return: true when the card was played
///
bool battle_play(BattleState* battle, size_t hand) {
    if (!battle_card_can_play(battle, hand)) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);
    Card*        card   = player->hand[hand];

    int          cost   = card->play_cost;

    CURRENT_BATTLE      = battle;
    SUBJECT_CARD        = card;
    effect_fire(battle, ACTING_SIDE, QUERY_CARD_PLAY_COST, &cost);

    size_t steps = battle_card_step_count(card);

    if (steps > 0) {
        PENDING_TARGET_COUNT = battle_build_step(card, 0, ACTING_SIDE);
    }

    player->cp -= cost;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        const Effect* effect = &card->effects[slot];

        if (!effect->func) {
            continue;
        }

        if (effect->trigger == QUERY_CARD_TARGETS ||
            effect->trigger == ON_CARD_TARGET_SELECTED) {
            continue;
        }

        if (effect->trigger == ON_CARD_PLAY) {
            EffectContext ctx = {};

            ctx.args[0]       = (void*) (uintptr_t) ACTING_SIDE;

            if (effect->func(&ctx, card) && effect->name) {
                protocol_emit(
                    "log effect name=\"%s\" "
                    "trigger=ON_CARD_PLAY",
                    effect->name
                );
            }

            continue;
        }

        Effect* attached = effect_attach(&player->effects, effect);

        if (attached) {
            attached->context->args[0] = (void*) (uintptr_t) ACTING_SIDE;
        }
    }

    effect_fire(battle, ACTING_SIDE, ON_CARD_PLAY, card);

    if (card->kingdom < KINGDOM_COUNT) {
        KINGDOM_PLAYS[card->kingdom]++;

        KingdomID combo = card->kingdom;

        if (KINGDOM_PLAYS[card->kingdom] == 2) {
            protocol_emit("log combo kingdom=%d", (int) card->kingdom);
            effect_fire(battle, ACTING_SIDE, ON_COMBO_DOUBLE, &combo);
        } else if (KINGDOM_PLAYS[card->kingdom] == 3) {
            protocol_emit("log climax kingdom=%d", (int) card->kingdom);
            effect_fire(battle, ACTING_SIDE, ON_COMBO_CLIMAX, &combo);
        }
    }

    player->hand[hand] = nullptr;

    if (steps > 0) {
        PENDING_CARD          = card;
        PENDING_SIDE          = ACTING_SIDE;
        PENDING_STEP          = 0;
        PENDING_STEP_COUNT    = steps;
        PENDING_PICKS[0].kind = TARGET_NONE;

        battle_emit_targets(PENDING_TARGET_COUNT);
    }

    SUBJECT_CARD   = nullptr;
    CURRENT_BATTLE = nullptr;

    return true;
}

/// battle_card_target
///
/// Records one selection for a parked card. When more steps remain it builds
/// and emits the next step's targets; when the last pick lands it fires the
/// card's ON_CARD_TARGET_SELECTED effects with the whole TARGET_NONE
/// terminated pick list, logging any that apply. The protocol selects by
/// index into the list emitted for the current step.
///
/// Params:
/// - battle -> battle to act in
/// - index  -> index into the advertised target list of the current step
///
/// Return: true when the selection was accepted
///
bool battle_card_target(BattleState* battle, size_t index) {
    if (!PENDING_CARD || index >= PENDING_TARGET_COUNT) {
        return false;
    }

    Card* card                      = PENDING_CARD;
    Side  side                      = PENDING_SIDE;

    PENDING_PICKS[PENDING_STEP]     = PENDING_TARGETS[index];
    PENDING_PICKS[PENDING_STEP + 1] = (CardTarget){TARGET_NONE, 0};
    PENDING_STEP++;

    CURRENT_BATTLE = battle;
    SUBJECT_CARD   = card;

    while (PENDING_STEP < PENDING_STEP_COUNT) {
        PENDING_TARGET_COUNT = battle_build_step(card, PENDING_STEP, side);

        if (PENDING_TARGET_COUNT > 0) {
            battle_emit_targets(PENDING_TARGET_COUNT);
            SUBJECT_CARD   = nullptr;
            CURRENT_BATTLE = nullptr;
            return true;
        }

        PENDING_PICKS[PENDING_STEP]     = (CardTarget){TARGET_NONE, 0};
        PENDING_PICKS[PENDING_STEP + 1] = (CardTarget){TARGET_NONE, 0};
        PENDING_STEP++;
    }

    PENDING_CARD = nullptr;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        const Effect* effect = &card->effects[slot];

        if (!effect->func || effect->trigger != ON_CARD_TARGET_SELECTED) {
            continue;
        }

        EffectContext ctx = {};

        ctx.args[0]       = (void*) (uintptr_t) side;

        if (effect->func(&ctx, PENDING_PICKS) && effect->name) {
            protocol_emit(
                "log effect name=\"%s\" trigger=ON_CARD_TARGET_SELECTED",
                effect->name
            );
        }
    }

    SUBJECT_CARD   = nullptr;
    CURRENT_BATTLE = nullptr;

    return true;
}

/// battle_sell
///
/// Sells a card from the hand for its queried sell value, then fires
/// ON_CARD_SELL.
///
/// Params:
/// - battle -> battle to act in
/// - hand   -> hand index of the card to sell
///
/// Return: true when the card was sold
///
bool battle_sell(BattleState* battle, size_t hand) {
    if (hand >= MAX_DRAWN_CARDS) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);
    Card*        card   = player->hand[hand];

    if (!card) {
        return false;
    }

    int value      = card->sell_cost;

    CURRENT_BATTLE = battle;
    SUBJECT_CARD   = card;
    effect_fire(battle, ACTING_SIDE, QUERY_CARD_SELL_COST, &value);

    player->cp += value;

    effect_fire(battle, ACTING_SIDE, ON_CARD_SELL, card);

    SUBJECT_CARD       = nullptr;
    CURRENT_BATTLE     = nullptr;

    player->hand[hand] = nullptr;

    return true;
}

/// battle_reclaim
///
/// Reclaims a friendly piece from the board for its queried reclaim
/// cost, removing it and refunding its value.
///
/// Params:
/// - battle -> battle to act in
/// - at     -> square of the piece to reclaim
///
/// Return: true when the reclaim was performed
///
bool battle_reclaim(BattleState* battle, Square at) {
    PieceInfo* piece = battle_at(battle, at);

    if (!piece || piece->side == ACTING_SIDE ||
        piece->piece->id == PIECE_KING) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);

    int          cost   = 30;

    CURRENT_BATTLE      = battle;
    SUBJECT             = piece;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_CP_COST_RECLAIM, &cost);
    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;

    if (player->cp < cost) {
        return false;
    }

    player->cp -= cost;

    battle_flip(battle, piece);

    return true;
}

/*----------------------------------------------------------------------------*\
                              RESOLVE AND CASCADE
\*----------------------------------------------------------------------------*/

/// battle_resolve
///
/// Resolves the attacker side's coverage into total meter damage on the
/// receiver, filling the damager register with every piece that dealt
/// damage.
///
/// Params:
/// - battle   -> battle to resolve
/// - attacker -> side whose pieces attack
///
/// Return: total damage dealt to the receiving meter
///
static int battle_resolve(BattleState* battle, Side attacker) {
    int    total    = 0;
    size_t damagers = 0;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* piece = battle->board.piece_board[index];

        if (!piece || piece == &VOID_CELL || piece->side != attacker) {
            continue;
        }

        Square* coverage = battle_attacks(battle, piece);
        int     dealt    = 0;

        Square  targets[MAX_BOARD_SIZE + 1];
        size_t  count = 0;

        while (!(coverage[count].x == -1 && coverage[count].y == -1)) {
            targets[count] = coverage[count];
            count++;
        }

        for (size_t i = 0; i < count; i++) {
            PieceInfo* victim = battle_at(battle, targets[i]);

            if (!victim || victim->side == attacker ||
                victim->side == SIDE_NEUTRAL) {
                continue;
            }

            dealt += battle_value(battle, piece, victim);
        }

        if (dealt > 0) {
            total                  += dealt;
            DAMAGER_LIST[damagers]  = piece;
            damagers++;
        }
    }

    DAMAGER_LIST[damagers] = nullptr;

    return total;
}

/// battle_cascade
///
/// Runs the flip cascade on the receiving side after meter damage:
/// while the meter is empty a flip candidate is chosen and flipped, the
/// meter refills through QUERY_METER_REFILL, and any remaining deficit
/// carries over. The gaining side's meter grows by however much its
/// maximum actually changed, so redirected or consumed flips account
/// for themselves. A receiver reduced to its lone king loses the battle.
///
/// Params:
/// - battle   -> battle to cascade in
/// - receiver -> side whose meter was damaged
///
/// Return: true when the battle ended by a king flip
///
static bool battle_cascade(BattleState* battle, Side receiver) {
    PlayerState* player = battle_player(battle, receiver);
    int          low    = player->meter;
    bool         over   = false;

    while (player->meter <= 0) {
        int        deficit = -player->meter;

        PieceInfo* candidates[MAX_BOARD_SIZE];
        size_t     count     = 0;
        size_t     non_kings = 0;

        for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
            PieceInfo* cell = battle->board.piece_board[index];

            if (!cell || cell == &VOID_CELL || cell->side != receiver ||
                cell->piece->id == PIECE_KING) {
                continue;
            }

            non_kings++;

            bool can       = true;

            CURRENT_BATTLE = battle;
            SUBJECT        = cell;
            effect_fire(battle, receiver, QUERY_PIECE_CAN_FLIP, &can);
            CURRENT_BATTLE = nullptr;
            SUBJECT        = nullptr;

            if (can) {
                candidates[count] = cell;
                count++;
            }
        }

        if (count == 0) {
            if (non_kings == 0) {
                PieceInfo* king = battle_find_king(battle, receiver);

                if (king) {
                    battle_flip(battle, king);
                }

                over = true;
                break;
            }

            player->meter = 0;

            break;
        }

        Side gainer_side    = receiver == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

        PlayerState* gainer = battle_player(battle, gainer_side);
        int          max_before = battle_meter_max(battle, gainer_side);

        int          flips      = 1;

        CURRENT_BATTLE          = battle;
        effect_fire(battle, receiver, QUERY_FLIP_COUNT, &flips);
        CURRENT_BATTLE = nullptr;

        for (int f = 0; f < flips && count > 0; f++) {
            size_t pick = (size_t) rand_r(&BATTLE_RNG) % count;

            battle_flip(battle, candidates[pick]);

            candidates[pick] = candidates[count - 1];
            count--;
        }

        int refill     = battle_meter_max(battle, receiver);

        CURRENT_BATTLE = battle;
        effect_fire(battle, receiver, QUERY_METER_REFILL, &refill);
        CURRENT_BATTLE  = nullptr;

        player->meter   = refill - deficit;

        int gainer_max  = battle_meter_max(battle, gainer_side);

        gainer->meter  += gainer_max - max_before;

        if (gainer->meter > 2 * gainer_max) {
            gainer->meter = 2 * gainer_max;
        }
    }

    CURRENT_BATTLE = battle;
    effect_fire(battle, receiver, ON_CASCADE_END, &low);
    CURRENT_BATTLE = nullptr;

    return over;
}

/// battle_half_turn
///
/// Finishes one side's half of the round: fires ON_TURN_END, resolves
/// its attacks into the enemy meter, cascades flips, ticks its timed
/// effects, and zeroes its remaining actions.
///
/// Params:
/// - battle -> battle being played
/// - side   -> side whose half ends
///
/// Return: true when the battle ended during the cascade
///
static bool battle_half_turn(BattleState* battle, Side side) {
    Side         enemy_side = side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
    PlayerState* enemy      = battle_player(battle, enemy_side);
    PlayerState* actor      = battle_player(battle, side);

    CURRENT_BATTLE          = battle;

    for (size_t index = 0; index < MAX_DRAWN_CARDS; index++) {
        Card* card = actor->hand[index];

        if (!card) {
            continue;
        }

        int value    = card->sell_cost;

        SUBJECT_CARD = card;
        effect_fire(battle, side, QUERY_CARD_SELL_COST, &value);

        actor->cp += value;

        effect_fire(battle, side, ON_CARD_SELL, card);

        SUBJECT_CARD       = nullptr;
        actor->hand[index] = nullptr;

        protocol_emit("log autosell name=\"%s\" value=%d", card->name, value);
    }

    effect_fire(battle, side, ON_TURN_END, (void*) (uintptr_t) battle->turn);
    CURRENT_BATTLE = nullptr;

    DAMAGERS       = DAMAGER_LIST;
    int total      = battle_resolve(battle, side);

    CURRENT_BATTLE = battle;
    effect_fire(battle, enemy_side, QUERY_METER_DAMAGE_TAKEN, &total);
    CURRENT_BATTLE    = nullptr;

    int saved_origin  = CASCADE_ORIGIN;

    CASCADE_ORIGIN    = enemy->meter;
    enemy->meter     -= total;

    if (total > 0) {
        protocol_emit(
            "log damage side=%s amount=%d",
            enemy_side == SIDE_WHITE ? "white" : "black",
            total
        );
    }

    bool over                            = battle_cascade(battle, enemy_side);

    CASCADE_ORIGIN                       = saved_origin;
    DAMAGERS                             = nullptr;

    battle_player(battle, side)->actions = 0;

    battle_reap();

    return over;
}

/// battle_damage
///
/// Reduces a side's meter by an amount then runs the flip cascade at
/// once, so card and relic meter damage flips pieces the moment it lands.
/// Fires QUERY_METER_DAMAGE_TAKEN first so receiver-side interceptors act.
///
/// Params:
/// - battle -> battle to damage in
/// - side   -> side whose meter takes the damage
/// - amount -> meter damage to apply
///
void battle_damage(BattleState* battle, Side side, int amount) {
    if (amount <= 0) {
        return;
    }

    BattleState* saved = CURRENT_BATTLE;
    int          dmg   = amount;

    CURRENT_BATTLE     = battle;
    effect_fire(battle, side, QUERY_METER_DAMAGE_TAKEN, &dmg);

    int saved_origin                    = CASCADE_ORIGIN;

    CASCADE_ORIGIN                      = battle_player(battle, side)->meter;
    battle_player(battle, side)->meter -= dmg;

    if (dmg > 0) {
        protocol_emit(
            "log damage side=%s amount=%d",
            side == SIDE_WHITE ? "white" : "black",
            dmg
        );
    }

    battle_cascade(battle, side);

    CASCADE_ORIGIN = saved_origin;
    CURRENT_BATTLE = saved;
}

/// battle_lunge
///
/// Force-moves a piece to an empty in-bounds destination, firing
/// ON_PIECE_MOVE, then resolves that one piece's attack coverage into the
/// enemy meter and cascades. Powers multi-strike card choreography such
/// as Crusade.
///
/// Params:
/// - battle -> battle to act in
/// - piece  -> the striking piece
/// - to     -> square to move to before striking
///
/// Return: total damage dealt to the enemy meter
///
int battle_lunge(BattleState* battle, PieceInfo* piece, Square to) {
    if (!piece) {
        return 0;
    }

    BattleState* saved = CURRENT_BATTLE;

    if ((to.x != piece->square.x || to.y != piece->square.y) &&
        battle_in_bounds(battle, to) && !battle_at(battle, to)) {
        Square from                                   = piece->square;

        battle->board.piece_board[square_index(from)] = nullptr;
        piece->square                                 = to;
        battle->board.piece_board[square_index(to)]   = piece;

        CURRENT_BATTLE                                = battle;
        MOVE_FROM                                     = from;
        effect_fire(battle, piece->side, ON_PIECE_MOVE, piece);
        MOVE_FROM      = SQUARE_END;
        CURRENT_BATTLE = saved;
    }

    Square* coverage = battle_attacks(battle, piece);
    Square  targets[MAX_BOARD_SIZE + 1];
    size_t  count = 0;

    while (!(coverage[count].x == -1 && coverage[count].y == -1)) {
        targets[count] = coverage[count];
        count++;
    }

    Side enemy_side = battle_enemy(piece->side);
    int  dealt      = 0;

    for (size_t i = 0; i < count; i++) {
        PieceInfo* victim = battle_at(battle, targets[i]);

        if (!victim || victim->side == piece->side ||
            victim->side == SIDE_NEUTRAL) {
            continue;
        }

        dealt += battle_value(battle, piece, victim);
    }

    if (dealt > 0) {
        DAMAGER_LIST[0] = piece;
        DAMAGER_LIST[1] = nullptr;
        DAMAGERS        = DAMAGER_LIST;

        battle_damage(battle, enemy_side, dealt);

        DAMAGERS = nullptr;
    }

    return dealt;
}

/// battle_walk_run
///
/// Attaches the run's held relics to the human seat at battle start with
/// the human as their beneficiary in args[0]. Runs before the meters are
/// computed so value editing relics fold into the starting meter maxima.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_run(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run) {
        return;
    }

    for (int id = 0; id < RELIC_COUNT; id++) {
        if (!run->relics[id]) {
            continue;
        }

        const Relic* relic = &RELIC_REGISTRY[id];

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            if (!relic->effects[slot].func) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, HUMAN_SIDE)->effects,
                &relic->effects[slot]
            );

            if (attached) {
                attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;
            }
        }
    }
}

/// battle_walk_run_effects
///
/// Re-attaches each chosen event's run-persistent effects to the human seat
/// at battle start, rebuilt from the choices recorded in run->events so the
/// bonuses need no separate run-side effect list. An event effect's baked
/// context parameters copy onto the attached copy; args[0] marks the human
/// as beneficiary. Runs before the meters so value bonuses fold into them.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_run_effects(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run) {
        return;
    }

    for (size_t id = 0; id < EVENT_COUNT; id++) {
        EventChoice choice = run->events[id];

        if (choice == NO_CHOICE) {
            continue;
        }

        const EventOption* option =
            &EVENT_REGISTRY[id]->options[choice == CHOICE_A ? 0 : 1];

        bool consume = false;

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            const Effect* effect = &option->effects[slot];

            if (!effect->func || effect->trigger == ON_EVENT_CHOOSE ||
                effect->trigger == QUERY_EVENT_TARGETS ||
                effect->trigger == ON_EVENT_TARGET_SELECTED) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, HUMAN_SIDE)->effects,
                effect
            );

            if (!attached) {
                continue;
            }

            if (effect->context) {
                for (size_t arg = 1; arg < MAX_EFFECT_ARGS; arg++) {
                    attached->context->args[arg] = effect->context->args[arg];
                }
            }

            if ((intptr_t) attached->context->args[1] == -1 &&
                run->event_picks[id] >= 0) {
                attached->context->args[1] =
                    (void*) (uintptr_t) run->event_picks[id];
            }

            attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;

            if (effect->lasts_for == ONE_BATTLE) {
                consume = true;
            }
        }

        if (consume) {
            run->events[id] = NO_CHOICE;
        }
    }
}

static const PieceID KINGDOM_BASIC[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = PIECE_BING,
    [KINGDOM_KEWARANI]  = PIECE_MEDEQ,
    [KINGDOM_ZARQAN]    = PIECE_WAZIR,
    [KINGDOM_HARUSHIMA] = PIECE_FUHYO,
    [KINGDOM_CAELAN]    = PIECE_PAWN,
};

/// battle_free_army
///
/// Spawns count free pieces of one type on random empty squares within a
/// chosen half of the board: the enemy reinforcements that pressure,
/// chains, and elite nodes grant, or the Traitor's Gambit intrusion into
/// the human's half. Occupied and void squares are skipped; a bounded
/// retry keeps a crowded board from looping forever.
///
/// Params:
/// - battle -> battle to place pieces in
/// - owner  -> side the pieces belong to
/// - half   -> side whose half the pieces spawn in
/// - id     -> piece type to spawn
/// - count  -> number of pieces to place
///
static void battle_free_army(
    BattleState* battle,
    Side         owner,
    Side         half,
    PieceID      id,
    size_t       count
) {
    if (id == PIECE_NONE) {
        return;
    }

    int8_t height = battle->board.height;
    int8_t low    = half == SIDE_BLACK ? 0 : (int8_t) (height / 2);
    int8_t high   = half == SIDE_BLACK ? (int8_t) (height / 2) : height;

    for (size_t placed = 0; placed < count; placed++) {
        for (int attempt = 0; attempt < 64; attempt++) {
            int8_t x = (int8_t) (rand_r(&BATTLE_RNG) % battle->board.width);
            int8_t y = (int8_t) (low + rand_r(&BATTLE_RNG) % (high - low));

            if (battle_spawn(battle, id, (Square){x, y}, owner)) {
                break;
            }
        }
    }
}

/// battle_reinforce
///
/// Grants a side count free reinforcements of the region kingdom's basic
/// infantry, spawned across the chosen half of the board. Backs both the
/// enemy's starting army and the Traitor's Gambit intrusion, so difficulty
/// and challenge effects reinforce without the private basic-piece table.
///
/// Params:
/// - battle -> battle to place pieces in
/// - owner  -> side the pieces belong to
/// - half   -> side whose half the pieces spawn in
/// - count  -> number of pieces to place
///
void battle_reinforce(
    BattleState* battle,
    Side         owner,
    Side         half,
    size_t       count
) {
    if (!battle->node || !battle->node->kingdom) {
        return;
    }

    battle_free_army(
        battle,
        owner,
        half,
        KINGDOM_BASIC[battle->node->kingdom->id],
        count
    );
}

/// battle_setup_armies
///
/// Attaches the region's cumulative chain penalties to the human seat, then
/// grants the enemy its free starting reinforcements. The base count is
/// Vorath's pressure plus the elite and liberation node bonuses; chain and
/// difficulty army bonuses fold in through QUERY_ENEMY_ARMY_COUNT. The
/// pieces are the region kingdom's basic infantry. Runs before the meters
/// compute so the extra pieces fold into the enemy's starting maximum.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_setup_armies(BattleState* battle) {
    MapNode* node = battle->node;

    if (!node || !node->kingdom) {
        return;
    }

    KingdomID      kingdom = node->kingdom->id;
    ChainPenaltyID level =
        node->kingdom->chain
            ? (ChainPenaltyID) (node->kingdom->chain - CHAIN_REGISTRY)
            : CHAIN_NONE;

    for (ChainPenaltyID c = CHAIN_BRONZE; c <= level; c++) {
        const ChainPenalty* chain = &CHAIN_REGISTRY[c];

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            if (!chain->effects[slot].func) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, HUMAN_SIDE)->effects,
                &chain->effects[slot]
            );

            if (attached) {
                attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;
            }
        }
    }

    int count = (int) run_pressure(BATTLE_ENGINE->run, kingdom);

    if (node->type == MAP_NODE_ELITE) {
        count++;
    }

    if (node->type == MAP_NODE_LIBERATION) {
        count += 2;
    }

    CURRENT_BATTLE = battle;
    effect_fire(battle, HUMAN_SIDE, QUERY_ENEMY_ARMY_COUNT, &count);
    CURRENT_BATTLE = nullptr;

    Side enemy     = battle_enemy(HUMAN_SIDE);

    battle_reinforce(battle, enemy, enemy, (size_t) count);
}

/// battle_attach_power
///
/// Attaches a kingdom power's effects to the given seat, marking each with
/// the seat as beneficiary in args[0] and the mastery level in args[1] so a
/// scaling innate reads its strength from context rather than a call
/// argument. Shared by the synergy, innate, and climax walks.
///
/// Params:
/// - battle -> battle being set up
/// - side   -> seat receiving the power
/// - power  -> kingdom power whose effects attach
/// - level  -> mastery level marked on each attached effect
///
void battle_attach_power(
    BattleState*        battle,
    Side                side,
    const KingdomPower* power,
    MasteryLevel        level
) {
    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (!power->effects[slot].func) {
            continue;
        }

        Effect* attached = effect_attach(
            &battle_player(battle, side)->effects,
            &power->effects[slot]
        );

        if (attached) {
            attached->context->args[0] = (void*) (uintptr_t) side;
            attached->context->args[1] = (void*) (uintptr_t) level;
        }
    }
}

/// battle_walk_synergies
///
/// Attaches the human's overseer synergies that apply in this region: a
/// synergy earned by clearing a kingdom's overseer fires while fighting in
/// that kingdom's lore-adjacent neighbour.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_synergies(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run || !battle->node || !battle->node->kingdom) {
        return;
    }

    KingdomID region = battle->node->kingdom->id;

    for (size_t source = 0; source < KINGDOM_COUNT; source++) {
        if (!run->synergies[source] || KINGDOM_ADJACENT[source] != region) {
            continue;
        }

        battle_attach_power(
            battle,
            HUMAN_SIDE,
            &SYNERGY_REGISTRY[source],
            MASTERY_NONE
        );
    }
}

/// battle_walk_innates
///
/// Attaches each kingdom innate the human has unlocked to the human seat,
/// passing the kingdom's mastery so a scaling innate reads its strength
/// from context.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_innates(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run) {
        return;
    }

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        if (run_innate_ready(run, kingdom)) {
            battle_attach_power(
                battle,
                HUMAN_SIDE,
                INNATE_REGISTRY[kingdom],
                run->kingdoms[kingdom].mastery
            );
        }
    }
}

/// eff_combo_double
///
/// The universal combo refund: a second same-kingdom card play in a turn
/// returns fifteen currency to the acting side. Kingdom-agnostic, so it
/// ignores the played kingdom carried in x.
///
/// Params:
/// - context -> args[0] acting side
/// - x       -> KingdomID* of the doubled kingdom (unused)
///
/// Return: true, the refund always applies
///
static bool eff_combo_double(EffectContext* context, void* x) {
    (void) x;

    Side side = (Side) (uintptr_t) context->args[0];

    battle_player(battle_current(), side)->cp += 15;

    return true;
}

/// COMBO_REFUND
///
/// The universal combo refund template, one copy per seat.
///
static const Effect COMBO_REFUND = {
    .func      = eff_combo_double,
    .name      = "Combo Refund",
    .trigger   = ON_COMBO_DOUBLE,
    .lasts_for = ENTIRE_BATTLE,
};

/// battle_walk_combos
///
/// Attaches the combo-fired seat effects to both seats: the universal combo
/// refund and every kingdom's climax. Each lies dormant until a second or
/// third same-kingdom card play fires ON_COMBO_DOUBLE / ON_COMBO_CLIMAX for
/// the acting seat. Both seats hold every one so either player can chain a
/// combo; the refund is kingdom-agnostic, each climax self-filters on the
/// played kingdom.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_combos(BattleState* battle) {
    Side sides[2] = {SIDE_WHITE, SIDE_BLACK};

    for (size_t seat = 0; seat < 2; seat++) {
        Effect* refund = effect_attach(
            &battle_player(battle, sides[seat])->effects,
            &COMBO_REFUND
        );

        if (refund) {
            refund->context->args[0] = (void*) (uintptr_t) sides[seat];
        }

        for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
            battle_attach_power(
                battle,
                sides[seat],
                CLIMAX_REGISTRY[kingdom],
                MASTERY_NONE
            );
        }
    }
}

/// battle_walk_modifiers
///
/// Attaches the battle modifier's effects to both seats, one copy each,
/// so every copy fires only for its own side. Runs before the meters are
/// computed so a value-editing modifier folds into the starting maxima.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_modifiers(BattleState* battle) {
    if (!battle->modifier) {
        return;
    }

    Side sides[2] = {SIDE_WHITE, SIDE_BLACK};

    for (size_t seat = 0; seat < 2; seat++) {
        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            if (!battle->modifier->effects[slot].func) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, sides[seat])->effects,
                &battle->modifier->effects[slot]
            );

            if (attached) {
                attached->context->args[0] = (void*) (uintptr_t) sides[seat];
            }
        }
    }
}

/// battle_scatter_voids
///
/// Marks a percentage of the board as missing squares, skipping any cell
/// already holding a piece so placed pieces keep their squares. Backs the
/// Dense Terrain modifier and the Mirage and Island Chain traits.
///
/// Params:
/// - battle  -> battle whose board gains voids
/// - percent -> share of squares to void
///
void battle_scatter_voids(BattleState* battle, int percent) {
    int voids = battle->board.width * battle->board.height * percent / 100;

    for (int placed = 0; placed < voids; placed++) {
        int8_t x     = (int8_t) (rand_r(&BATTLE_RNG) % battle->board.width);
        int8_t y     = (int8_t) (rand_r(&BATTLE_RNG) % battle->board.height);

        size_t index = square_index((Square){x, y});

        if (battle->board.piece_board[index]) {
            continue;
        }

        battle->board.piece_board[index] = (PieceInfo*) &VOID_CELL;
    }
}

/// battle_board_view
///
/// Refills the board's visible read for the human: every cell starts
/// visible, then QUERY_BOARD_STATE lets the human's fog effects hide enemy
/// pieces. Fired from the human's seat so only the human-side copies of
/// battle-wide items act (Eagle Eye reveals and noops them if held).
///
/// Params:
/// - battle -> battle whose board view is refilled
///
void battle_board_view(BattleState* battle) {
    for (size_t cell = 0; cell < MAX_BOARD_SIZE; cell++) {
        battle->board.visible[cell] = true;
    }

    BattleState* saved = CURRENT_BATTLE;

    CURRENT_BATTLE     = battle;
    effect_fire(battle, HUMAN_SIDE, QUERY_BOARD_STATE, &battle->board);
    CURRENT_BATTLE = saved;
}

/// battle_hand_view
///
/// Refills a side's hand reveal read: every card starts visible, then
/// QUERY_HAND_STATE lets the side's blinder effects mask a card's identity.
///
/// Params:
/// - battle -> battle whose hand view is refilled
/// - side   -> side whose hand is read
///
void battle_hand_view(BattleState* battle, Side side) {
    PlayerState* player = battle_player(battle, side);

    for (size_t card = 0; card < MAX_DRAWN_CARDS; card++) {
        player->hand_visible[card] = true;
    }

    BattleState* saved = CURRENT_BATTLE;

    CURRENT_BATTLE     = battle;
    effect_fire(battle, side, QUERY_HAND_STATE, player);
    CURRENT_BATTLE = saved;
}

/// battle_walk_traits
///
/// Attaches the board trait's effects to both seats, one copy each, so
/// every copy fires only for its own side. Runs before the meters compute
/// so a value-editing trait folds into the starting maxima.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_traits(BattleState* battle) {
    if (!battle->board.trait) {
        return;
    }

    Side sides[2] = {SIDE_WHITE, SIDE_BLACK};

    for (size_t seat = 0; seat < 2; seat++) {
        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            if (!battle->board.trait->effects[slot].func) {
                continue;
            }

            Effect* attached = effect_attach(
                &battle_player(battle, sides[seat])->effects,
                &battle->board.trait->effects[slot]
            );

            if (attached) {
                attached->context->args[0] = (void*) (uintptr_t) sides[seat];
            }
        }
    }
}

/// battle_attach_rule
///
/// Attaches one rule item's effects to the human seat with the human as
/// their beneficiary in args[0]. Shared by the difficulty and challenge
/// walks.
///
/// Params:
/// - battle  -> battle being set up
/// - effects -> the rule item's effect array
///
static void battle_attach_rule(BattleState* battle, const Effect* effects) {
    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (!effects[slot].func) {
            continue;
        }

        Effect* attached = effect_attach(
            &battle_player(battle, HUMAN_SIDE)->effects,
            &effects[slot]
        );

        if (attached) {
            attached->context->args[0] = (void*) (uintptr_t) HUMAN_SIDE;
        }
    }
}

/// battle_walk_rules
///
/// Attaches the run's difficulty and challenge effect items to the human
/// seat so their penalties and constraints compose through the battle
/// triggers, naming no specific difficulty or challenge in the engine.
/// Difficulties are cumulative, so every level up to the run's is attached;
/// the challenge is a single choice. Runs before the meters compute so a
/// value-editing rule folds into the starting maxima.
///
/// Params:
/// - battle -> battle being set up
///
static void battle_walk_rules(BattleState* battle) {
    RunState* run = BATTLE_ENGINE ? BATTLE_ENGINE->run : nullptr;

    if (!run) {
        return;
    }

    for (Difficulty d = DIFFICULTY_FREE;
         d < DIFFICULTY_NONE && d <= run->difficulty;
         d++) {
        battle_attach_rule(battle, DIFFICULTY_REGISTRY[d].effects);
    }

    if (run->challenge < CHALLENGE_COUNT) {
        battle_attach_rule(battle, CHALLENGE_REGISTRY[run->challenge].effects);
    }
}

/// battle_begin
///
/// Sets up a battle on the given campaign node: the human's seat from
/// the battle parity (white on even battles), board size and voids,
/// kings, free enemy pieces, run-walk effect attachment, meters,
/// starting currency, and the opening turn. When the human sits black,
/// the AI's first white turn plays out before control returns. A null
/// node builds the synthetic town-sized board used before the campaign
/// exists. Stores the battle on the engine.
///
/// Params:
/// - engine -> engine owning the run entering battle
/// - node   -> campaign node the battle takes place on
///
void battle_begin(EngineState* engine, MapNode* node) {
    BattleState* battle = calloc(1, sizeof(BattleState));

    BATTLE_ENGINE       = engine;
    engine->battle      = battle;

    HUMAN_SIDE  = engine->run->battles_fought % 2 ? SIDE_BLACK : SIDE_WHITE;

    int8_t size = 10;

    if (node && node->map) {
        size = node->map->type == MAP_TOWN       ? 12
               : node->map->type == MAP_PROVINCE ? 14
                                                 : 16;
    }

    battle->node          = node;
    battle->modifier      = node ? node->modifier : nullptr;
    battle->board.trait   = node ? node->trait : nullptr;

    battle->board.width   = size;
    battle->board.height  = size;
    battle->turn          = 1;
    battle->white.effects = ll_init();
    battle->black.effects = ll_init();

    BATTLE_RNG            = (unsigned int)
        rng_mix(engine->run->seed, engine->run->battles_fought + 1);

    battle->white.cp = 20;
    battle->black.cp = 20;

    battle_walk_run(battle);
    battle_walk_run_effects(battle);
    battle_walk_modifiers(battle);
    battle_walk_traits(battle);
    battle_walk_rules(battle);

    Square dimension = {battle->board.width, battle->board.height};

    CURRENT_BATTLE   = battle;
    effect_fire(battle, SIDE_WHITE, QUERY_BOARD_DIMENSION, &dimension);

    battle->board.width  = dimension.x;
    battle->board.height = dimension.y;

    int8_t center        = (int8_t) (battle->board.width / 2);

    battle_spawn(
        battle,
        PIECE_KING,
        (Square){center, (int8_t) (battle->board.height - 1)},
        SIDE_WHITE
    );
    battle_spawn(battle, PIECE_KING, (Square){center, 0}, SIDE_BLACK);

    effect_fire(battle, SIDE_WHITE, ON_BOARD_BUILD, &battle->board);
    CURRENT_BATTLE = nullptr;

    battle_setup_armies(battle);
    battle_walk_innates(battle);
    battle_walk_synergies(battle);
    battle_walk_combos(battle);

    CURRENT_BATTLE = battle;
    effect_fire(battle, HUMAN_SIDE, ON_BATTLE_SETUP, node);
    CURRENT_BATTLE = nullptr;

    vorath_attach_capacity(battle, battle_enemy(HUMAN_SIDE));

    battle->white.meter = battle_meter_max(battle, SIDE_WHITE);
    battle->black.meter = battle_meter_max(battle, SIDE_BLACK);

    ACTING_SIDE         = SIDE_WHITE;

    CURRENT_BATTLE      = battle;
    effect_fire(battle, SIDE_WHITE, ON_BATTLE_START, node);
    effect_fire(battle, SIDE_BLACK, ON_BATTLE_START, node);
    CURRENT_BATTLE = nullptr;

    turn_start(battle, SIDE_WHITE);

    if (HUMAN_SIDE == SIDE_BLACK) {
        ai_take_turn(battle);

        if (battle_half_turn(battle, SIDE_WHITE)) {
            battle_finish(battle, SIDE_WHITE);
            return;
        }

        ACTING_SIDE = SIDE_BLACK;
        turn_start(battle, SIDE_BLACK);
    }
}

/// battle_territory_finish
///
/// Ends the battle by territory count after turn ten: the side holding
/// more squares wins, ties losing for the human.
///
/// Params:
/// - battle -> battle to score and finish
///
static void battle_territory_finish(BattleState* battle) {
    int white_squares = 0;
    int black_squares = 0;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            Side holder    = battle_territory(battle, (Square){x, y});

            white_squares += holder == SIDE_WHITE;
            black_squares += holder == SIDE_BLACK;
        }
    }

    protocol_emit(
        "log territory white=%d black=%d",
        white_squares,
        black_squares
    );

    Side winner;

    if (white_squares == black_squares) {
        winner = HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
    } else {
        winner = white_squares > black_squares ? SIDE_WHITE : SIDE_BLACK;
    }

    battle_finish(battle, winner);
}

/// battle_end_turn
///
/// Ends the human's turn and plays out the rest of the round in strict
/// white-first order. When the human sits white, the AI's black turn
/// follows before the next round starts; when the human sits black,
/// the round closes first and the AI's next white turn plays out before
/// control returns. After turn ten the battle ends by territory.
///
/// Params:
/// - battle -> battle whose human turn ends
///
void battle_end_turn(BattleState* battle) {
    Side ai_side = HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

    if (battle_half_turn(battle, HUMAN_SIDE)) {
        battle_finish(battle, HUMAN_SIDE);
        return;
    }

    if (HUMAN_SIDE == SIDE_WHITE) {
        ACTING_SIDE = ai_side;
        turn_start(battle, ai_side);
        ai_take_turn(battle);

        if (battle_half_turn(battle, ai_side)) {
            battle_finish(battle, ai_side);
            return;
        }
    }

    battle->turn++;

    if (battle->turn > 10) {
        battle_territory_finish(battle);
        return;
    }

    ACTING_SIDE = SIDE_WHITE;
    turn_start(battle, SIDE_WHITE);

    if (HUMAN_SIDE == SIDE_BLACK) {
        ai_take_turn(battle);

        if (battle_half_turn(battle, SIDE_WHITE)) {
            battle_finish(battle, SIDE_WHITE);
            return;
        }

        ACTING_SIDE = SIDE_BLACK;
        turn_start(battle, SIDE_BLACK);
    }
}

/*----------------------------------------------------------------------------*\
                                    QUERIES
\*----------------------------------------------------------------------------*/

/// battle_moves
///
/// Generates the piece's movement list: gates on QUERY_PIECE_CAN_MOVE,
/// runs the piece's mv, then lets QUERY_PIECE_MOVES effects edit the
/// list in place. The list lives in the shared scratch buffer.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to generate moves for
///
/// Return: SQUARE_END terminated move list
///
Square* battle_moves(BattleState* battle, PieceInfo* piece) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;

    CURRENT_BATTLE             = battle;
    SUBJECT                    = piece;

    mg_begin();

    bool can = true;
    effect_fire(battle, piece->side, QUERY_PIECE_CAN_MOVE, &can);

    Square* list = mg_end();

    if (can) {
        list = piece->piece->mv(battle, piece);
        effect_fire(battle, piece->side, QUERY_PIECE_MOVES, list);
    }

    CURRENT_BATTLE = saved_battle;
    SUBJECT        = saved_subject;

    return list;
}

/// battle_attacks
///
/// Generates the piece's attack coverage list: gates on
/// QUERY_PIECE_CAN_ATTACK, runs the piece's at, then lets
/// QUERY_PIECE_ATTACKS effects edit the list in place. Coverage includes
/// empty and enemy squares, never friendly ones.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to generate coverage for
///
/// Return: SQUARE_END terminated coverage list
///
Square* battle_attacks(BattleState* battle, PieceInfo* piece) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;

    CURRENT_BATTLE             = battle;
    SUBJECT                    = piece;

    mg_begin();

    bool can = true;
    effect_fire(battle, piece->side, QUERY_PIECE_CAN_ATTACK, &can);

    Square* list = mg_end();

    if (can) {
        list = piece->piece->at(battle, piece);
        effect_fire(battle, piece->side, QUERY_PIECE_ATTACKS, list);
    }

    CURRENT_BATTLE = saved_battle;
    SUBJECT        = saved_subject;

    return list;
}

/// battle_value
///
/// Computes the piece's damage against the given victim through the
/// damage queries. A nullptr victim performs an effective-value query
/// used for meters, pricing, and display; with a victim the defense
/// query runs afterwards with the victim as subject.
///
/// Params:
/// - battle -> battle the pieces live in
/// - piece  -> damage dealing piece
/// - victim -> damage receiving piece, nullptr for effective value
///
/// Return: queried damage or effective value
///
int battle_value(BattleState* battle, PieceInfo* piece, PieceInfo* victim) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;
    PieceInfo*   saved_victim  = VICTIM;

    int          damage        = piece->piece->value;

    CURRENT_BATTLE             = battle;
    SUBJECT                    = piece;
    VICTIM                     = victim;

    effect_fire(battle, piece->side, QUERY_PIECE_DAMAGE_DEALT, &damage);

    if (victim) {
        SUBJECT = victim;
        VICTIM  = piece;

        effect_fire(battle, victim->side, QUERY_PIECE_DAMAGE_TAKEN, &damage);
    }

    CURRENT_BATTLE = saved_battle;
    SUBJECT        = saved_subject;
    VICTIM         = saved_victim;

    return damage;
}

/// battle_meter_max
///
/// Computes a side's maximum meter as the sum of its pieces' effective
/// values.
///
/// Params:
/// - battle -> battle to measure
/// - side   -> side to measure
///
/// Return: the side's maximum meter
///
int battle_meter_max(BattleState* battle, Side side) {
    int total = 0;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side) {
            total += battle_value(battle, cell, nullptr);
        }
    }

    BattleState* saved = CURRENT_BATTLE;

    CURRENT_BATTLE     = battle;
    effect_fire(battle, side, QUERY_METER_AMOUNT, &total);
    CURRENT_BATTLE = saved;

    return total;
}

/// battle_territory
///
/// Determines which side holds the square by Chebyshev distance to the
/// nearest piece of each side (neutral pieces hold no territory), then
/// fires QUERY_SQUARE_OWNER so effects may adjust the holder.
///
/// Params:
/// - battle -> battle to measure
/// - square -> square to classify
///
/// Return: holding side, SIDE_NEUTRAL when contested
///
Side battle_territory(BattleState* battle, Square square) {
    int nearest_white = MAX_BOARD_SIZE;
    int nearest_black = MAX_BOARD_SIZE;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (!cell || cell == &VOID_CELL || cell->side == SIDE_NEUTRAL) {
            continue;
        }

        int dx       = abs(cell->square.x - square.x);
        int dy       = abs(cell->square.y - square.y);
        int distance = dx > dy ? dx : dy;

        if (cell->side == SIDE_WHITE && distance < nearest_white) {
            nearest_white = distance;
        }

        if (cell->side == SIDE_BLACK && distance < nearest_black) {
            nearest_black = distance;
        }
    }

    Side         owner = nearest_white < nearest_black   ? SIDE_WHITE
                         : nearest_black < nearest_white ? SIDE_BLACK
                                                         : SIDE_NEUTRAL;

    BattleState* saved = CURRENT_BATTLE;

    CURRENT_BATTLE     = battle;
    OWNER_SQUARE       = square;
    effect_fire(battle, SIDE_WHITE, QUERY_SQUARE_OWNER, &owner);
    OWNER_SQUARE   = SQUARE_END;
    CURRENT_BATTLE = saved;

    return owner;
}

/// battle_at
///
/// Returns the live piece occupying the square.
///
/// Params:
/// - battle -> battle providing the board
/// - square -> square to look up
///
/// Return: occupying piece, nullptr when empty or missing
///
PieceInfo* battle_at(BattleState* battle, Square square) {
    if (!battle_in_bounds(battle, square)) {
        return nullptr;
    }

    return battle->board.piece_board[square_index(square)];
}

/// battle_in_bounds
///
/// Checks whether the square lies inside the active board and is not a
/// void cell.
///
/// Params:
/// - battle -> battle providing the board
/// - square -> square to check
///
/// Return: true when the square exists on this board
///
bool battle_in_bounds(BattleState* battle, Square square) {
    if (square.x < 0 || square.x >= battle->board.width || square.y < 0 ||
        square.y >= battle->board.height) {
        return false;
    }

    return battle->board.piece_board[square_index(square)] != &VOID_CELL;
}
