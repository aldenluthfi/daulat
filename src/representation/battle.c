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

const PieceInfo VOID_CELL = {};

static BattleState* CURRENT_BATTLE;
static PieceInfo*   SUBJECT;
static PieceInfo*   VICTIM;
static Card*        SUBJECT_CARD;
static Square       MOVE_FROM = { -1, -1 };
static PieceInfo**  DAMAGERS;

static EngineState* BATTLE_ENGINE;
static Side         HUMAN_SIDE;
static Side         ACTING_SIDE;
static unsigned int BATTLE_RNG;
static PieceInfo*   FLIPPED_PIECE;
static PieceInfo*   DAMAGER_LIST[MAX_BOARD_SIZE + 1];

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
    PIECE_CHANCELLOR,
    PIECE_SOVEREIGN_BANNER,
    PIECE_NONE,
};

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

/*----------------------------------------------------------------------------*\
                                  MOVEGEN KIT
\*----------------------------------------------------------------------------*/

const Square ORTHOGONAL_DIRECTIONS[] = {
    { 0, -1 }, { -1, 0 }, { 1, 0 }, { 0, 1 },
    { 0,  0 },
};

const Square DIAGONAL_DIRECTIONS[] = {
    { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 },
    {  0,  0 },
};

const Square ALL_DIRECTIONS[] = {
    { -1, -1 }, { 0, -1 }, { 1, -1 },
    { -1,  0 },            { 1,  0 },
    { -1,  1 }, { 0,  1 }, { 1,  1 },
    {  0,  0 },
};

static Square SCRATCH[MAX_BOARD_SIZE + 1];
static size_t SCRATCH_CURSOR;

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

/// mg_begin
///
/// Resets the shared static scratch buffer that movement generation
/// writes into. See the header for the full scratch buffer convention.
///
void mg_begin(void) {
    SCRATCH_CURSOR = 0;
}

/// mg_push
///
/// Appends one square to the scratch buffer. Squares past the buffer
/// capacity are dropped.
///
/// Params:
/// - square -> square to append
///
void mg_push(Square square) {
    if (SCRATCH_CURSOR < MAX_BOARD_SIZE) {
        SCRATCH[SCRATCH_CURSOR] = square;
        SCRATCH_CURSOR++;
    }
}

/// mg_end
///
/// Terminates the scratch buffer with SQUARE_END and returns it. The
/// sentinel is overwritten by the next push, so sequential generator
/// calls concatenate their output.
///
/// Return: the scratch buffer, SQUARE_END terminated
///
Square* mg_end(void) {
    SCRATCH[SCRATCH_CURSOR] = SQUARE_END;

    return SCRATCH;
}

/// mg_emit
///
/// Pushes the destination when it satisfies the generation semantics:
/// movement wants empty squares only, threat wants every square not
/// held by a friendly piece.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - dest   -> candidate destination
/// - threat -> true for at (coverage), false for mv (movement)
///
static void mg_emit(BattleState* battle, PieceInfo* self, Square dest,
                    bool threat) {
    PieceInfo* occupant = battle_at(battle, dest);

    if (!occupant || (threat && occupant->side != self->side)) {
        mg_push(dest);
    }
}

/// mg_leap
///
/// Emits one destination per offset from self's square. Offsets are
/// mirrored vertically for black. Movement emits only empty squares;
/// threat emits every covered square not holding a friendly piece.
///
/// Params:
/// - battle  -> battle providing the board
/// - self    -> piece generating from its square and side
/// - offsets -> zero vector terminated offset array, white perspective
/// - threat  -> true for at (coverage), false for mv (movement)
///
void mg_leap(BattleState* battle, PieceInfo* self,
             const Square* offsets, bool threat) {
    for (size_t i = 0; offsets[i].x != 0 || offsets[i].y != 0; i++) {
        int8_t dy = self->side == SIDE_BLACK
                  ? (int8_t) -offsets[i].y
                  : offsets[i].y;

        Square dest = {
            (int8_t) (self->square.x + offsets[i].x),
            (int8_t) (self->square.y + dy),
        };

        if (battle_in_bounds(battle, dest)) {
            mg_emit(battle, self, dest, threat);
        }
    }
}

/// mg_slide
///
/// Walks each direction from self's square emitting squares between min
/// and max steps. Blockers stop the walk regardless of min. Movement
/// emits empty squares; threat also emits the first non-friendly
/// occupant reached.
///
/// Params:
/// - battle     -> battle providing the board
/// - self       -> piece generating from its square and side
/// - directions -> zero vector terminated direction array
/// - min        -> minimum step distance to emit
/// - max        -> maximum step distance, 127 for unbounded
/// - threat     -> true for at (coverage), false for mv (movement)
///
void mg_slide(BattleState* battle, PieceInfo* self,
              const Square* directions, int8_t min, int8_t max,
              bool threat) {
    for (size_t i = 0; directions[i].x != 0 || directions[i].y != 0;
         i++) {
        int8_t dx = directions[i].x;
        int8_t dy = self->side == SIDE_BLACK
                  ? (int8_t) -directions[i].y
                  : directions[i].y;

        for (int step = 1; step <= max; step++) {
            Square dest = {
                (int8_t) (self->square.x + dx * step),
                (int8_t) (self->square.y + dy * step),
            };

            if (!battle_in_bounds(battle, dest)) {
                break;
            }

            PieceInfo* occupant = battle_at(battle, dest);

            if (occupant) {
                if (threat && step >= min
                    && occupant->side != self->side) {
                    mg_push(dest);
                }

                break;
            }

            if (step >= min) {
                mg_push(dest);
            }
        }
    }
}

/// mg_compound
///
/// Emits the union of other pieces' patterns generated from self's own
/// square and side, matching mv or at according to threat. Parts with
/// no registry entry are skipped.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> piece generating from its square and side
/// - parts  -> PIECE_NONE terminated array of pieces to copy
/// - threat -> true for at (coverage), false for mv (movement)
///
void mg_compound(BattleState* battle, PieceInfo* self,
                 const PieceID* parts, bool threat) {
    for (size_t i = 0; parts[i] != PIECE_NONE; i++) {
        const Piece* part = PIECE_REGISTRY[parts[i]];

        if (!part) {
            continue;
        }

        if (threat) {
            part->at(battle, self);
        } else {
            part->mv(battle, self);
        }
    }
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
static PlayerState* battle_player(BattleState* battle, Side side) {
    return side == SIDE_WHITE ? &battle->white : &battle->black;
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
static PieceInfo* battle_find_king(BattleState* battle, Side side) {
    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL && cell->side == side
            && cell->piece->id == PIECE_KING) {
            return cell;
        }
    }

    return nullptr;
}

/// turn_start
///
/// Starts a side's turn: restores actions, applies queried income, and
/// fires ON_TURN_START. Card drawing joins this pipeline once hands are
/// implemented.
///
/// Params:
/// - battle -> battle whose turn starts
/// - side   -> side beginning its turn
///
static void turn_start(BattleState* battle, Side side) {
    PlayerState* player = battle_player(battle, side);

    player->actions = 3;

    int income = 10;

    CURRENT_BATTLE = battle;
    effect_fire(battle, side, QUERY_CP_INCOME, &income);

    player->cp += income;

    effect_fire(battle, side, ON_TURN_START,
                (void*) (uintptr_t) battle->turn);
    CURRENT_BATTLE = nullptr;
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
    effect_fire(battle, winner, ON_BATTLE_END,
                (void*) (uintptr_t) winner);
    CURRENT_BATTLE = nullptr;

    protocol_emit("result won=%d", winner == HUMAN_SIDE ? 1 : 0);

    battle_free(battle);
    free(battle);

    BATTLE_ENGINE->battle = nullptr;
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
    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        PieceInfo* cell = battle->board.piece_board[index];

        if (cell && cell != &VOID_CELL) {
            battle_remove(battle, cell);
        }
    }

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
PieceInfo* battle_spawn(BattleState* battle, PieceID id, Square at,
                        Side side) {
    const Piece* template = PIECE_REGISTRY[id];

    if (!template || !battle_in_bounds(battle, at)
        || battle_at(battle, at)) {
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

    info->piece  = copy;
    info->square = at;
    info->side   = side;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (!copy->effects[slot].func) {
            continue;
        }

        copy->effects[slot].context = calloc(1, sizeof(EffectContext));
        copy->effects[slot].context->args[0] = info;
    }

    battle->board.piece_board[square_index(at)] = info;

    return info;
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

    FLIPPED_PIECE  = nullptr;
    CURRENT_BATTLE = battle;
    SUBJECT        = piece;

    effect_fire(battle, piece->side, ON_PIECE_FLIP_PRE, &pick);

    if (pick) {
        pick->side = pick->side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

        FLIPPED_PIECE = pick;

        protocol_emit("log flip piece=%s x=%d y=%d",
                      pick->piece->name, pick->square.x, pick->square.y);

        effect_fire(battle, pick->side, ON_PIECE_FLIP, pick);
    }

    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;
}

/// battle_remove
///
/// Removes a piece from the board entirely, freeing its effect contexts,
/// its heap piece copy, and the piece info itself.
///
/// Params:
/// - battle -> battle the piece lives in
/// - piece  -> piece to remove and free
///
void battle_remove(BattleState* battle, PieceInfo* piece) {
    battle->board.piece_board[square_index(piece->square)] = nullptr;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        free(piece->piece->effects[slot].context);
    }

    free(piece->piece);
    free(piece);
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

    int cost = 1;

    CURRENT_BATTLE = battle;
    SUBJECT        = piece;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_ACTION_COST_MOVE,
                &cost);

    if (player->actions < cost) {
        CURRENT_BATTLE = nullptr;
        SUBJECT        = nullptr;
        return false;
    }

    player->actions -= cost;

    battle->board.piece_board[square_index(from)] = nullptr;
    piece->square = to;
    battle->board.piece_board[square_index(to)]   = piece;

    MOVE_FROM = from;
    effect_fire(battle, ACTING_SIDE, ON_PIECE_MOVE, piece);
    MOVE_FROM = SQUARE_END;

    CURRENT_BATTLE = nullptr;
    SUBJECT        = nullptr;

    return true;
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
bool battle_buy(BattleState* battle, PieceID id, Square at) {
    const Piece* template = PIECE_REGISTRY[id];

    if (!template || id == PIECE_KING) {
        return false;
    }

    for (size_t i = 0; COMBO_RESULTS[i] != PIECE_NONE; i++) {
        if (COMBO_RESULTS[i] == id) {
            return false;
        }
    }

    if (ACTING_SIDE == HUMAN_SIDE
        && !BATTLE_ENGINE->run->pieces[id]) {
        return false;
    }

    PlayerState* player = battle_player(battle, ACTING_SIDE);

    int cost        = template->value;
    int action_cost = 1;

    CURRENT_BATTLE = battle;
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_CP_COST_BUY, &cost);
    effect_fire(battle, ACTING_SIDE, QUERY_PIECE_ACTION_COST_BUY,
                &action_cost);
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

    CURRENT_BATTLE = battle;
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
    (void) battle;
    (void) a;
    (void) b;

    return false;
}

/// battle_play
///
/// Plays a card from the hand, paying its queried play cost, running its
/// immediate effects and attaching its durational ones, then fires
/// ON_CARD_PLAY and advances the same-kingdom combo chain. The a and b
/// targets are encoded per the card's documentation.
///
/// Params:
/// - battle -> battle to act in
/// - hand   -> hand index of the card to play
/// - a      -> first encoded target, card specific
/// - b      -> second encoded target, card specific
///
/// Return: true when the card was played
///
bool battle_play(BattleState* battle, size_t hand, long a, long b) {
    (void) battle;
    (void) hand;
    (void) a;
    (void) b;

    return false;
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
    (void) battle;
    (void) hand;

    return false;
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
    (void) battle;
    (void) at;

    return false;
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

        Square targets[MAX_BOARD_SIZE + 1];
        size_t count = 0;

        while (!(coverage[count].x == -1 && coverage[count].y == -1)) {
            targets[count] = coverage[count];
            count++;
        }

        for (size_t i = 0; i < count; i++) {
            PieceInfo* victim = battle_at(battle, targets[i]);

            if (!victim || victim->side == attacker
                || victim->side == SIDE_NEUTRAL) {
                continue;
            }

            dealt += battle_value(battle, piece, victim);
        }

        if (dealt > 0) {
            total += dealt;
            DAMAGER_LIST[damagers] = piece;
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

    while (player->meter <= 0) {
        int deficit = -player->meter;

        PieceInfo* candidates[MAX_BOARD_SIZE];
        size_t     count      = 0;
        size_t     non_kings  = 0;

        for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
            PieceInfo* cell = battle->board.piece_board[index];

            if (!cell || cell == &VOID_CELL || cell->side != receiver
                || cell->piece->id == PIECE_KING) {
                continue;
            }

            non_kings++;

            bool can = true;

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

                return true;
            }

            player->meter = 0;

            return false;
        }

        size_t pick = (size_t) rand_r(&BATTLE_RNG) % count;

        Side gainer_side = receiver == SIDE_WHITE
                         ? SIDE_BLACK
                         : SIDE_WHITE;

        PlayerState* gainer     = battle_player(battle, gainer_side);
        int          max_before = battle_meter_max(battle, gainer_side);

        battle_flip(battle, candidates[pick]);

        int refill = battle_meter_max(battle, receiver);

        CURRENT_BATTLE = battle;
        effect_fire(battle, receiver, QUERY_METER_REFILL, &refill);
        CURRENT_BATTLE = nullptr;

        player->meter = refill - deficit;

        int gainer_max = battle_meter_max(battle, gainer_side);

        gainer->meter += gainer_max - max_before;

        if (gainer->meter > 2 * gainer_max) {
            gainer->meter = 2 * gainer_max;
        }

        if (FLIPPED_PIECE) {
            CURRENT_BATTLE = battle;
            SUBJECT        = FLIPPED_PIECE;

            effect_fire(battle, FLIPPED_PIECE->side, ON_PIECE_FLIP_POST,
                        FLIPPED_PIECE);

            CURRENT_BATTLE = nullptr;
            SUBJECT        = nullptr;
        }
    }

    return false;
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
    Side         enemy_side = side == SIDE_WHITE
                            ? SIDE_BLACK
                            : SIDE_WHITE;
    PlayerState* enemy      = battle_player(battle, enemy_side);

    CURRENT_BATTLE = battle;
    effect_fire(battle, side, ON_TURN_END,
                (void*) (uintptr_t) battle->turn);
    CURRENT_BATTLE = nullptr;

    DAMAGERS  = DAMAGER_LIST;
    int total = battle_resolve(battle, side);

    CURRENT_BATTLE = battle;
    effect_fire(battle, enemy_side, QUERY_METER_DAMAGE_TAKEN, &total);
    CURRENT_BATTLE = nullptr;

    enemy->meter -= total;

    if (total > 0) {
        protocol_emit("log damage side=%s amount=%d",
                      enemy_side == SIDE_WHITE ? "white" : "black",
                      total);
    }

    bool over = battle_cascade(battle, enemy_side);

    DAMAGERS = nullptr;

    effect_tick(&battle_player(battle, side)->effects);

    battle_player(battle, side)->actions = 0;

    return over;
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

    BATTLE_ENGINE  = engine;
    engine->battle = battle;

    HUMAN_SIDE = engine->run->battles_fought % 2
               ? SIDE_BLACK
               : SIDE_WHITE;

    battle->node          = node;
    battle->board.width   = 10;
    battle->board.height  = 10;
    battle->turn          = 1;
    battle->white.effects = ll_init();
    battle->black.effects = ll_init();

    BATTLE_RNG = (unsigned int) rng_mix(engine->run->seed,
                                        engine->run->battles_fought + 1);

    int8_t center = (int8_t) (battle->board.width / 2);

    battle_spawn(battle, PIECE_KING,
                 (Square) { center, (int8_t) (battle->board.height - 1) },
                 SIDE_WHITE);
    battle_spawn(battle, PIECE_KING, (Square) { center, 0 }, SIDE_BLACK);

    battle->white.cp    = 20;
    battle->black.cp    = 20;
    battle->white.meter = battle_meter_max(battle, SIDE_WHITE);
    battle->black.meter = battle_meter_max(battle, SIDE_BLACK);

    ACTING_SIDE = SIDE_WHITE;

    CURRENT_BATTLE = battle;
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
            Side holder = battle_territory(battle, (Square) { x, y });

            white_squares += holder == SIDE_WHITE;
            black_squares += holder == SIDE_BLACK;
        }
    }

    protocol_emit("log territory white=%d black=%d",
                  white_squares, black_squares);

    Side winner;

    if (white_squares == black_squares) {
        winner = HUMAN_SIDE == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;
    } else {
        winner = white_squares > black_squares
               ? SIDE_WHITE
               : SIDE_BLACK;
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

    CURRENT_BATTLE = battle;
    SUBJECT        = piece;

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

    CURRENT_BATTLE = battle;
    SUBJECT        = piece;

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
int battle_value(BattleState* battle, PieceInfo* piece,
                 PieceInfo* victim) {
    BattleState* saved_battle  = CURRENT_BATTLE;
    PieceInfo*   saved_subject = SUBJECT;
    PieceInfo*   saved_victim  = VICTIM;

    int damage = piece->piece->value;

    CURRENT_BATTLE = battle;
    SUBJECT        = piece;
    VICTIM         = victim;

    effect_fire(battle, piece->side, QUERY_PIECE_DAMAGE_DEALT, &damage);

    if (victim) {
        SUBJECT = victim;
        VICTIM  = piece;

        effect_fire(battle, victim->side, QUERY_PIECE_DAMAGE_TAKEN,
                    &damage);
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

    return total;
}

/// battle_territory
///
/// Determines which side holds the square by Chebyshev distance to the
/// nearest piece of each side. Neutral pieces hold no territory.
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

        int dx = abs(cell->square.x - square.x);
        int dy = abs(cell->square.y - square.y);
        int distance = dx > dy ? dx : dy;

        if (cell->side == SIDE_WHITE && distance < nearest_white) {
            nearest_white = distance;
        }

        if (cell->side == SIDE_BLACK && distance < nearest_black) {
            nearest_black = distance;
        }
    }

    if (nearest_white < nearest_black) {
        return SIDE_WHITE;
    }

    if (nearest_black < nearest_white) {
        return SIDE_BLACK;
    }

    return SIDE_NEUTRAL;
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
    if (square.x < 0 || square.x >= battle->board.width
        || square.y < 0 || square.y >= battle->board.height) {
        return false;
    }

    return battle->board.piece_board[square_index(square)] != &VOID_CELL;
}
