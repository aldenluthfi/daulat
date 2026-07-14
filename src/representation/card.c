//! card.c
//!
//! Card play targeting helpers shared across the kingdom data files. A
//! targeting card advertises its legal targets in a CardTarget list through
//! QUERY_CARD_TARGETS and resolves against the chosen one in
//! ON_CARD_TARGET_SELECTED; these primitives build and read that list. The
//! square pack/unpack pair also serves effects that stash a square in their
//! own context.
//!
//! Created: 13/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// card_target_count
///
/// Counts the targets a list holds before its TARGET_NONE terminator.
///
/// Params:
/// - list -> TARGET_NONE terminated target list
///
/// Return: the number of live targets
///
size_t card_target_count(const CardTarget* list) {
    size_t count = 0;

    while (list[count].kind != TARGET_NONE) {
        count++;
    }

    return count;
}

/// card_target_push
///
/// Appends one target before the list terminator and re-terminates, the
/// CardTarget analogue of mg_push.
///
/// Params:
/// - list  -> TARGET_NONE terminated target list to extend
/// - kind  -> kind of the appended target
/// - value -> value read per kind
///
void card_target_push(CardTarget* list, TargetKind kind, int value) {
    size_t count = card_target_count(list);

    list[count].kind      = kind;
    list[count].value     = value;
    list[count + 1].kind  = TARGET_NONE;
}

/// card_target_at
///
/// Decodes a TARGET_SQUARE value into a board square at the fixed stride of
/// twenty cells per row.
///
/// Params:
/// - value -> square target value, y times twenty plus x
///
/// Return: the decoded board square
///
Square card_target_at(int value) {
    return (Square) {(int8_t) (value % 20), (int8_t) (value / 20)};
}

/// card_targets_piece
///
/// Pushes every board piece the card's block accepts onto the target list as
/// a TARGET_PIECE, the single enumerator behind every card that acts on a
/// chosen piece. The block, written inline at the card, decides side, king,
/// type, or flipped status, so no filter is named or hoisted here.
///
/// Params:
/// - list   -> target list to extend
/// - battle -> battle providing the board
/// - match  -> block selecting acceptable pieces
///
void card_targets_piece(CardTarget* list, BattleState* battle,
                        bool (^match)(const PieceInfo* piece)) {
    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* cell = battle_at(battle, (Square) {x, y});

            if (cell && cell->piece && match(cell)) {
                card_target_push(list, TARGET_PIECE, y * 20 + x);
            }
        }
    }
}

/// card_targets_square
///
/// Pushes every board square the card's block accepts onto the target list
/// as a TARGET_SQUARE, the single enumerator behind cards that act on a
/// location such as a drop, teleport, or file.
///
/// Params:
/// - list   -> target list to extend
/// - battle -> battle providing the board
/// - match  -> block selecting acceptable squares
///
void card_targets_square(CardTarget* list, BattleState* battle,
                         bool (^match)(Square square)) {
    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            Square square = {x, y};

            if (battle_in_bounds(battle, square) && match(square)) {
                card_target_push(list, TARGET_SQUARE, y * 20 + x);
            }
        }
    }
}

/// card_targets_piece_type
///
/// Pushes every registered piece identity the card's block accepts onto the
/// target list as a TARGET_PIECE_TYPE valued by its identity, the single
/// enumerator behind cards that pick a kind of piece.
///
/// Params:
/// - list  -> target list to extend
/// - match -> block selecting acceptable identities
///
void card_targets_piece_type(CardTarget* list,
                             bool (^match)(const Piece* piece)) {
    for (int id = 0; id < PIECE_COUNT; id++) {
        const Piece* piece = PIECE_REGISTRY[id];

        if (piece && match(piece)) {
            card_target_push(list, TARGET_PIECE_TYPE, id);
        }
    }
}

/// card_square
///
/// Decodes a packed target back into a board square using the fixed
/// stride of twenty cells per row.
///
/// Params:
/// - packed -> target encoded as y times twenty plus x, in a void pointer
///
/// Return: the decoded board square
///
Square card_square(void* packed) {
    long value = (long) (uintptr_t) packed;

    return (Square) {(int8_t) (value % 20), (int8_t) (value / 20)};
}

/// card_pack
///
/// Packs a board square into the target pointer convention, offset by one
/// so a packed zero square is distinguishable from an absent target.
///
/// Params:
/// - square -> board square to pack
///
/// Return: the packed target as a pointer sized integer
///
uintptr_t card_pack(Square square) {
    return (uintptr_t) (square.y * 20 + square.x) + 1;
}
