//! card.c
//!
//! Card play target codec shared across the kingdom data files. Card
//! bodies receive their two targets as packed longs; these helpers pack a
//! square into the void pointer convention and decode one back into a
//! board square.
//!
//! Created: 13/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

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
