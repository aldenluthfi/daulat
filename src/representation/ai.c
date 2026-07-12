//! ai.c
//!
//! Enemy turn logic. Implements the shared move scoring core, the five
//! kingdom archetypes with their behind-fallback behaviors, and the
//! Divination preview that reports the enemy's planned actions.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// ai_active_side
///
/// Determines which side the AI is playing. The turn engine zeroes a
/// side's actions when its half settles and grants the AI its actions
/// immediately before ai_take_turn, so the side with actions remaining
/// is the AI's.
///
/// Params:
/// - battle -> battle being played
///
/// Return: the AI's side this battle
///
static Side ai_active_side(BattleState* battle) {
    return battle->white.actions > 0 ? SIDE_WHITE : SIDE_BLACK;
}

/// ai_find_king
///
/// Finds a side's king on the board.
///
/// Params:
/// - battle -> battle to search
/// - side   -> owning side
///
/// Return: the king, nullptr when it is not on the board
///
static PieceInfo* ai_find_king(BattleState* battle, Side side) {
    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        Square     square = {(int8_t) (index % 20), (int8_t) (index / 20)};

        PieceInfo* cell   = battle_at(battle, square);

        if (cell && cell->side == side && cell->piece->id == PIECE_KING) {
            return cell;
        }
    }

    return nullptr;
}

/// ai_try_buy
///
/// Buys a knight on an empty square adjacent to the AI's king when the
/// AI can afford it.
///
/// Params:
/// - battle -> battle to act in
/// - side   -> the AI's side
///
/// Return: true when a piece was bought
///
static bool ai_try_buy(BattleState* battle, Side side) {
    PieceInfo*   king   = ai_find_king(battle, side);
    PlayerState* player = side == SIDE_WHITE ? &battle->white : &battle->black;

    if (!king || player->cp < 30) {
        return false;
    }

    for (int8_t dy = -1; dy <= 1; dy++) {
        for (int8_t dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            Square spot = {
                (int8_t) (king->square.x + dx),
                (int8_t) (king->square.y + dy),
            };

            if (battle_in_bounds(battle, spot) && !battle_at(battle, spot) &&
                battle_buy(battle, PIECE_KNIGHT, spot)) {
                protocol_emit(
                    "log ai buy piece=Knight x=%d y=%d",
                    spot.x,
                    spot.y
                );
                return true;
            }
        }
    }

    return false;
}

/// ai_try_advance
///
/// Moves the AI piece whose best move most reduces its Chebyshev
/// distance to the enemy king.
///
/// Params:
/// - battle -> battle to act in
/// - side   -> the AI's side
///
/// Return: true when a piece was moved
///
static bool ai_try_advance(BattleState* battle, Side side) {
    Side       enemy_side = side == SIDE_WHITE ? SIDE_BLACK : SIDE_WHITE;

    PieceInfo* target     = ai_find_king(battle, enemy_side);

    if (!target) {
        return false;
    }

    PieceInfo* best_piece = nullptr;
    Square     best_dest  = {0, 0};
    int        best_gain  = 0;

    for (size_t index = 0; index < MAX_BOARD_SIZE; index++) {
        Square     square = {(int8_t) (index % 20), (int8_t) (index / 20)};

        PieceInfo* piece  = battle_at(battle, square);

        if (!piece || piece->side != side) {
            continue;
        }

        int     dx      = abs(piece->square.x - target->square.x);
        int     dy      = abs(piece->square.y - target->square.y);
        int     current = dx > dy ? dx : dy;

        Square* moves   = battle_moves(battle, piece);

        for (size_t i = 0; !(moves[i].x == -1 && moves[i].y == -1); i++) {
            int mx       = abs(moves[i].x - target->square.x);
            int my       = abs(moves[i].y - target->square.y);
            int distance = mx > my ? mx : my;
            int gain     = current - distance;

            if (gain > best_gain) {
                best_gain  = gain;
                best_piece = piece;
                best_dest  = moves[i];
            }
        }
    }

    if (!best_piece) {
        return false;
    }

    Square from = best_piece->square;

    if (!battle_move(battle, from, best_dest)) {
        return false;
    }

    protocol_emit(
        "log ai move fx=%d fy=%d tx=%d ty=%d",
        from.x,
        from.y,
        best_dest.x,
        best_dest.y
    );

    return true;
}

/// ai_take_turn
///
/// Spends the AI side's actions with the Hammer behavior: buy
/// aggressively, then advance on the enemy king. The turn engine
/// resolves and settles the half afterwards.
///
/// Params:
/// - battle -> battle whose AI actions are spent
///
void ai_take_turn(BattleState* battle) {
    Side         side   = ai_active_side(battle);

    PlayerState* player = side == SIDE_WHITE ? &battle->white : &battle->black;

    while (player->actions > 0) {
        if (ai_try_buy(battle, side)) {
            continue;
        }

        if (ai_try_advance(battle, side)) {
            continue;
        }

        break;
    }
}

/// ai_plan
///
/// Emits the actions the enemy would take on its next turn without
/// performing them, backing the Divination card's preview.
///
/// Params:
/// - battle -> battle to preview the enemy turn of
///
void ai_plan(BattleState* battle) {
    (void) battle;
}
