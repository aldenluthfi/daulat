//! longwei.c
//!
//! Longwei kingdom data. Holds the kingdom's pieces, cards, and board
//! traits along with its Bulwark innate, climax, Iron Strategist
//! overseer setup, and narrative event handler.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                              MOVEMENT GENERATION
\*----------------------------------------------------------------------------*/

/// bing_gen
///
/// Generates the Bing's forward step, shared by movement and coverage.
/// The sideways step earned after first blood is added by an embedded
/// effect.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void bing_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(battle, self, (const Square[]) {{0, -1}, {0, 0}}, threat);
}

/// bing_mv
///
/// Generates the Bing's movement through bing_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* bing_mv(BattleState* battle, PieceInfo* self) {
    bing_gen(battle, self, false);

    return mg_end();
}

/// bing_at
///
/// Generates the Bing's attack coverage through bing_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* bing_at(BattleState* battle, PieceInfo* self) {
    bing_gen(battle, self, true);

    return mg_end();
}

/// xiang_gen
///
/// Generates the Xiang's two-square diagonal leaps, shared by movement
/// and coverage. The enemy-territory attack ban is an embedded effect.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void xiang_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {{-2, -2}, {2, -2}, {-2, 2}, {2, 2}, {0, 0}},
        threat
    );
}

/// xiang_mv
///
/// Generates the Xiang's movement through xiang_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* xiang_mv(BattleState* battle, PieceInfo* self) {
    xiang_gen(battle, self, false);

    return mg_end();
}

/// xiang_at
///
/// Generates the Xiang's attack coverage through xiang_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* xiang_at(BattleState* battle, PieceInfo* self) {
    xiang_gen(battle, self, true);

    return mg_end();
}

/// ma_gen
///
/// Generates the Ma's knight leaps, each blocked when a piece sits in the
/// orthogonal elbow square between the Ma and its landing square.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void ma_gen(BattleState* battle, PieceInfo* self, bool threat) {
    static const Square leaps[] = {
        {-1, -2}, {1, -2}, {-2, -1}, {2, -1},
        {-2, 1},  {2, 1},  {-1, 2},  {1, 2},
    };

    for (size_t i = 0; i < 8; i++) {
        Square elbow = abs(leaps[i].y) == 2
                           ? (Square) {self->square.x,
                                       (int8_t) (self->square.y +
                                                 (leaps[i].y > 0 ? 1 : -1))}
                           : (Square) {(int8_t) (self->square.x +
                                                 (leaps[i].x > 0 ? 1 : -1)),
                                       self->square.y};

        if (battle_at(battle, elbow)) {
            continue;
        }

        Square dest = {
            (int8_t) (self->square.x + leaps[i].x),
            (int8_t) (self->square.y + leaps[i].y),
        };

        if (!battle_in_bounds(battle, dest)) {
            continue;
        }

        PieceInfo* occupant = battle_at(battle, dest);

        if (!occupant || (threat && occupant->side != self->side)) {
            mg_push(dest);
        }
    }
}

/// ma_mv
///
/// Generates the Ma's movement through ma_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* ma_mv(BattleState* battle, PieceInfo* self) {
    ma_gen(battle, self, false);

    return mg_end();
}

/// ma_at
///
/// Generates the Ma's attack coverage through ma_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* ma_at(BattleState* battle, PieceInfo* self) {
    ma_gen(battle, self, true);

    return mg_end();
}

/// cannon_cover
///
/// Emits the cannon coverage along each direction: squares beyond exactly
/// one intervening screen piece, including the empties past the screen and
/// the first non-friendly occupant found after it.
///
/// Params:
/// - battle     -> battle providing the board
/// - self       -> generating piece
/// - directions -> zero vector terminated direction array
///
static void cannon_cover(
    BattleState*  battle,
    PieceInfo*    self,
    const Square* directions
) {
    for (size_t i = 0; directions[i].x != 0 || directions[i].y != 0; i++) {
        int8_t dx       = directions[i].x;
        int8_t dy       = directions[i].y;
        bool   screened = false;

        for (int step = 1; step < MAX_BOARD_SIZE; step++) {
            Square dest = {
                (int8_t) (self->square.x + dx * step),
                (int8_t) (self->square.y + dy * step),
            };

            if (!battle_in_bounds(battle, dest)) {
                break;
            }

            PieceInfo* occupant = battle_at(battle, dest);

            if (!screened) {
                if (occupant) {
                    screened = true;
                }

                continue;
            }

            if (!occupant) {
                mg_push(dest);
                continue;
            }

            if (occupant->side != self->side) {
                mg_push(dest);
            }

            break;
        }
    }
}

/// pao_mv
///
/// Generates the Pao's movement: unbounded orthogonal slides.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* pao_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, ORTHOGONAL_DIRECTIONS, 1, 127, false);

    return mg_end();
}

/// pao_at
///
/// Generates the Pao's attack coverage: the cannon screen rule along rows
/// and columns.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* pao_at(BattleState* battle, PieceInfo* self) {
    cannon_cover(battle, self, ORTHOGONAL_DIRECTIONS);

    return mg_end();
}

/// liubo_mv
///
/// Generates the Liubo Diviner's teleport destinations: every empty
/// square currently covered by any enemy piece, deduplicated. The enemy
/// coverage lists share the movegen scratch, so they are copied out
/// before the union is rebuilt.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* liubo_mv(BattleState* battle, PieceInfo* self) {
    Square found[MAX_BOARD_SIZE];
    size_t count = 0;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x = 0; x < battle->board.width; x++) {
            PieceInfo* enemy = battle_at(battle, (Square) {x, y});

            if (!enemy || enemy->side == self->side) {
                continue;
            }

            Square* cover = battle_attacks(battle, enemy);

            for (size_t i = 0; !(cover[i].x == -1 && cover[i].y == -1); i++) {
                if (battle_at(battle, cover[i])) {
                    continue;
                }

                bool duplicate = false;

                for (size_t k = 0; k < count && !duplicate; k++) {
                    duplicate = found[k].x == cover[i].x &&
                                found[k].y == cover[i].y;
                }

                if (!duplicate && count < MAX_BOARD_SIZE) {
                    found[count] = cover[i];
                    count++;
                }
            }
        }
    }

    mg_begin();

    for (size_t i = 0; i < count; i++) {
        mg_push(found[i]);
    }

    return mg_end();
}

/// liubo_at
///
/// Generates the Liubo Diviner's coverage: the eight king-adjacent
/// squares, an interpretation since the design gives no threat pattern.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* liubo_at(BattleState* battle, PieceInfo* self) {
    mg_leap(battle, self, ALL_DIRECTIONS, true);

    return mg_end();
}

/// sang_gen
///
/// Generates the Sang's Janggi-elephant path: one orthogonal step then two
/// diagonal steps outward, blockable at either intermediate square.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void sang_gen(BattleState* battle, PieceInfo* self, bool threat) {
    for (size_t o = 0; ORTHOGONAL_DIRECTIONS[o].x != 0 ||
                       ORTHOGONAL_DIRECTIONS[o].y != 0;
         o++) {
        int8_t ox    = ORTHOGONAL_DIRECTIONS[o].x;
        int8_t oy    = ORTHOGONAL_DIRECTIONS[o].y;

        Square inter = {
            (int8_t) (self->square.x + ox),
            (int8_t) (self->square.y + oy),
        };

        if (!battle_in_bounds(battle, inter) || battle_at(battle, inter)) {
            continue;
        }

        for (size_t d = 0; DIAGONAL_DIRECTIONS[d].x != 0 ||
                           DIAGONAL_DIRECTIONS[d].y != 0;
             d++) {
            int8_t dx = DIAGONAL_DIRECTIONS[d].x;
            int8_t dy = DIAGONAL_DIRECTIONS[d].y;

            if ((ox != 0 && dx != ox) || (oy != 0 && dy != oy)) {
                continue;
            }

            Square step2 = {
                (int8_t) (inter.x + dx),
                (int8_t) (inter.y + dy),
            };

            if (!battle_in_bounds(battle, step2) || battle_at(battle, step2)) {
                continue;
            }

            Square dest = {
                (int8_t) (step2.x + dx),
                (int8_t) (step2.y + dy),
            };

            if (!battle_in_bounds(battle, dest)) {
                continue;
            }

            PieceInfo* occupant = battle_at(battle, dest);

            if (!occupant || (threat && occupant->side != self->side)) {
                mg_push(dest);
            }
        }
    }
}

/// sang_mv
///
/// Generates the Sang's movement through sang_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* sang_mv(BattleState* battle, PieceInfo* self) {
    sang_gen(battle, self, false);

    return mg_end();
}

/// sang_at
///
/// Generates the Sang's attack coverage through sang_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* sang_at(BattleState* battle, PieceInfo* self) {
    sang_gen(battle, self, true);

    return mg_end();
}

/// cavalry_gen
///
/// Generates the Northern Cavalry's knight leaps, shared by movement and
/// coverage.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
/// - threat -> true for at (coverage), false for mv (movement)
///
static void cavalry_gen(BattleState* battle, PieceInfo* self, bool threat) {
    mg_leap(
        battle,
        self,
        (const Square[]) {
            {-1, -2}, {1, -2}, {-2, -1}, {2, -1},
            {-2, 1},  {2, 1},  {-1, 2},  {1, 2},
            {0, 0},
        },
        threat
    );
}

/// cavalry_mv
///
/// Generates the Northern Cavalry's movement through cavalry_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* cavalry_mv(BattleState* battle, PieceInfo* self) {
    cavalry_gen(battle, self, false);

    return mg_end();
}

/// cavalry_at
///
/// Generates the Northern Cavalry's attack coverage through cavalry_gen.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* cavalry_at(BattleState* battle, PieceInfo* self) {
    cavalry_gen(battle, self, true);

    return mg_end();
}

/// hwacha_mv
///
/// Generates the Hwacha's movement: unbounded slides in all eight
/// directions.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated move list
///
static Square* hwacha_mv(BattleState* battle, PieceInfo* self) {
    mg_slide(battle, self, ALL_DIRECTIONS, 1, 127, false);

    return mg_end();
}

/// hwacha_at
///
/// Generates the Hwacha's attack coverage: the cannon screen rule applied
/// to all eight directions.
///
/// Params:
/// - battle -> battle providing the board
/// - self   -> generating piece
///
/// Return: SQUARE_END terminated coverage list
///
static Square* hwacha_at(BattleState* battle, PieceInfo* self) {
    cannon_cover(battle, self, ALL_DIRECTIONS);

    return mg_end();
}

/*----------------------------------------------------------------------------*\
                                PIECE EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_bing_blood
///
/// Marks the Bing as blooded the first time it deals damage to an enemy,
/// unlocking its sideways step.
///
/// Params:
/// - context -> Bing's context, args[0] self, args[1] blooded flag
/// - x       -> damage value, left unchanged
///
/// Return: true the moment the Bing becomes blooded
///
static bool eff_bing_blood(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self || !battle_victim() || context->args[1]) {
        return false;
    }

    context->args[1] = (void*) 1;

    return true;
}

/// eff_bing_sideways
///
/// Adds the Bing's sideways step to its move list once it is blooded.
///
/// Params:
/// - context -> Bing's context, args[0] self, args[1] blooded flag
/// - x       -> move list, extended through the shared scratch
///
/// Return: true when a sideways square was added
///
static bool eff_bing_sideways(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self || !context->args[1]) {
        return false;
    }

    BattleState* battle = battle_current();
    size_t       pushed = 0;

    for (int8_t dx = -1; dx <= 1; dx += 2) {
        Square side = {(int8_t) (self->square.x + dx), self->square.y};

        if (battle_in_bounds(battle, side) && !battle_at(battle, side)) {
            mg_push(side);
            pushed++;
        }
    }

    mg_end();

    return pushed > 0;
}

/// eff_bing_sideways_at
///
/// Adds the Bing's sideways coverage once it is blooded.
///
/// Params:
/// - context -> Bing's context, args[0] self, args[1] blooded flag
/// - x       -> coverage list, extended through the shared scratch
///
/// Return: true when a sideways square was added
///
static bool eff_bing_sideways_at(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self || !context->args[1]) {
        return false;
    }

    BattleState* battle = battle_current();
    size_t       pushed = 0;

    for (int8_t dx = -1; dx <= 1; dx += 2) {
        Square      side     = {(int8_t) (self->square.x + dx), self->square.y};
        PieceInfo*  occupant = battle_at(battle, side);

        if (battle_in_bounds(battle, side) &&
            (!occupant || occupant->side != self->side)) {
            mg_push(side);
            pushed++;
        }
    }

    mg_end();

    return pushed > 0;
}

/// eff_xiang_territory
///
/// Bans the Xiang from attacking while it stands in enemy territory.
///
/// Params:
/// - context -> Xiang's context, args[0] self
/// - x       -> can-attack flag to clear
///
/// Return: true when the ban applied
///
static bool eff_xiang_territory(EffectContext* context, void* x) {
    PieceInfo* self = context->args[0];

    if (battle_subject() != self) {
        return false;
    }

    BattleState* battle = battle_current();

    if (battle_territory(battle, self->square) != battle_enemy(self->side)) {
        return false;
    }

    *(bool*) x = false;

    return true;
}

/// eff_cavalry_bulwark
///
/// Halves damage taken by allies orthogonally adjacent to the Northern
/// Cavalry.
///
/// Params:
/// - context -> Cavalry's context, args[0] self
/// - x       -> incoming damage value
///
/// Return: true when the reduction applied
///
static bool eff_cavalry_bulwark(EffectContext* context, void* x) {
    PieceInfo* self   = context->args[0];
    PieceInfo* victim = battle_subject();

    if (!victim || victim == self || victim->side != self->side) {
        return false;
    }

    int dx = abs(victim->square.x - self->square.x);
    int dy = abs(victim->square.y - self->square.y);

    if (dx + dy != 1) {
        return false;
    }

    int* damage = x;
    *damage     = *damage * 50 / 100;

    return true;
}

/// eff_sideways_grant
///
/// Grants a permanent sideways step to the piece it is attached to, used
/// by River Wade.
///
/// Params:
/// - context -> granted context, args[0] holding the target piece
/// - x       -> move list, extended through the shared scratch
///
/// Return: true when a sideways square was added
///
static bool eff_sideways_grant(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* self = context->args[0];

    if (battle_subject() != self) {
        return false;
    }

    BattleState* battle = battle_current();
    size_t       pushed = 0;

    for (int8_t dx = -1; dx <= 1; dx += 2) {
        Square side = {(int8_t) (self->square.x + dx), self->square.y};

        if (battle_in_bounds(battle, side) && !battle_at(battle, side)) {
            mg_push(side);
            pushed++;
        }
    }

    mg_end();

    return pushed > 0;
}

/*----------------------------------------------------------------------------*\
                                 CARD EFFECTS
\*----------------------------------------------------------------------------*/

/// eff_river_wade_targets
///
/// River Wade targeting: advertises every friendly pawn grade piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one pawn was offered
///
static bool eff_river_wade_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side && piece_is_pawn(p->piece->id);
    });

    return card_target_count(x) > 0;
}

/// eff_river_wade_pick
///
/// River Wade resolution: grants the chosen pawn a permanent sideways step
/// for the rest of the battle.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the sideways step was granted
///
static bool eff_river_wade_pick(EffectContext* context, void* x) {
    (void) context;

    CardTarget*  target = x;
    BattleState* battle = battle_current();

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || !piece_is_pawn(piece->piece->id)) {
        return false;
    }

    static const Effect grant = {
        .func      = eff_sideways_grant,
        .name      = "River Wade",
        .trigger   = QUERY_PIECE_MOVES,
        .lasts_for = ENTIRE_BATTLE,
    };

    return piece_embed_effect(piece, &grant) != nullptr;
}

/// eff_charge
///
/// Lets the charged slider pass one square beyond a single blocker along
/// any line on its next move.
///
/// Params:
/// - context -> target piece in args[1]
/// - x       -> move list, extended through the shared scratch
///
/// Return: true when a jump square was added
///
static bool eff_charge(EffectContext* context, void* x) {
    (void) x;

    PieceInfo* target = context->args[1];

    if (battle_subject() != target) {
        return false;
    }

    BattleState* battle = battle_current();
    size_t       pushed = 0;

    for (size_t i = 0; ALL_DIRECTIONS[i].x != 0 || ALL_DIRECTIONS[i].y != 0;
         i++) {
        int8_t dx = ALL_DIRECTIONS[i].x;
        int8_t dy = ALL_DIRECTIONS[i].y;

        for (int step = 1; step < MAX_BOARD_SIZE; step++) {
            Square block = {
                (int8_t) (target->square.x + dx * step),
                (int8_t) (target->square.y + dy * step),
            };

            if (!battle_in_bounds(battle, block)) {
                break;
            }

            if (battle_at(battle, block)) {
                Square beyond = {
                    (int8_t) (block.x + dx),
                    (int8_t) (block.y + dy),
                };

                if (battle_in_bounds(battle, beyond) &&
                    !battle_at(battle, beyond)) {
                    mg_push(beyond);
                    pushed++;
                }

                break;
            }
        }
    }

    mg_end();

    return pushed > 0;
}

/// eff_charge_targets
///
/// Charge targeting: advertises every friendly slider, the only pieces the
/// jump helps.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one slider was offered
///
static bool eff_charge_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side && p->piece->class == MOVE_SLIDER;
    });

    return card_target_count(x) > 0;
}

/// eff_charge_pick
///
/// Charge resolution: attaches the Charge jump to the chosen piece for its
/// next move this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the charge was attached
///
static bool eff_charge_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    Side         side   = (Side) (uintptr_t) context->args[0];
    BattleState* battle = battle_current();

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side ||
        piece->piece->class != MOVE_SLIDER) {
        return false;
    }

    static const Effect charge = {
        .func      = eff_charge,
        .name      = "Charge",
        .trigger   = QUERY_PIECE_MOVES,
        .lasts_for = TURNS_1,
    };

    Effect* attached = effect_attach(&battle_player(battle, side)->effects,
                                     &charge);

    if (attached) {
        attached->context->args[1] = piece;
    }

    return attached != nullptr;
}

/// eff_formation
///
/// Adds half again to the damage of a piece standing in a contiguous line
/// of at least three friendly pieces.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> damage value to raise
///
/// Return: true when the boost applied
///
static bool eff_formation(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (!battle_victim() || !subject || subject->side != side) {
        return false;
    }

    static const Square lines[] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    BattleState* battle = battle_current();
    bool         formed = false;

    for (size_t i = 0; i < 4 && !formed; i++) {
        int count = 1;

        for (int sign = -1; sign <= 1; sign += 2) {
            for (int step = 1; step < MAX_BOARD_SIZE; step++) {
                Square spot = {
                    (int8_t) (subject->square.x + lines[i].x * step * sign),
                    (int8_t) (subject->square.y + lines[i].y * step * sign),
                };

                PieceInfo* cell = battle_in_bounds(battle, spot)
                                      ? battle_at(battle, spot)
                                      : nullptr;

                if (!cell || cell->side != side) {
                    break;
                }

                count++;
            }
        }

        formed = count >= 3;
    }

    if (!formed) {
        return false;
    }

    *(int*) x += *(int*) x / 2;

    return true;
}

/// eff_divination
///
/// Immediate play effect storing the enemy plan for preview.
///
/// Params:
/// - context -> unused
/// - x       -> played card, unused
///
/// Return: true, the preview always resolves
///
static bool eff_divination(EffectContext* context, void* x) {
    (void) context;
    (void) x;

    ai_plan(battle_current());
    protocol_emit("log divination");

    return true;
}

/// eff_cannon_volley_targets
///
/// Cannon Volley targeting: advertises every friendly Pao as a square
/// target.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one Pao was offered
///
static bool eff_cannon_volley_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side && p->piece->id == PIECE_PAO;
    });

    return card_target_count(x) > 0;
}

/// eff_cannon_volley_pick
///
/// Cannon Volley resolution: deals the chosen Pao's value to every enemy on
/// its row and column, ignoring screens.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the volley resolved
///
static bool eff_cannon_volley_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* pao = battle_at(battle, card_target_at(target->value));

    if (!pao || pao->side != side || pao->piece->id != PIECE_PAO) {
        return false;
    }

    int value = battle_value(battle, pao, nullptr);
    int total = 0;

    for (int8_t y = 0; y < battle->board.height; y++) {
        for (int8_t x2 = 0; x2 < battle->board.width; x2++) {
            PieceInfo* cell = battle_at(battle, (Square) {x2, y});

            if (cell && cell->side != side && cell->side != SIDE_NEUTRAL &&
                (x2 == pao->square.x || y == pao->square.y)) {
                total += value;
            }
        }
    }

    battle_damage(battle, battle_enemy(side), total);

    return true;
}

/// eff_palace_zone
///
/// Confines the enemy king to a three-by-three zone around the square it
/// occupied when Palace Decree resolved.
///
/// Params:
/// - context -> packed zone center in args[1]
/// - x       -> the king's move list, filtered in place
///
/// Return: true when the confinement applied
///
static bool eff_palace_zone(EffectContext* context, void* x) {
    PieceInfo* subject = battle_subject();

    if (!subject || subject->piece->id != PIECE_KING) {
        return false;
    }

    Square  center = card_square(context->args[1]);
    Square* list   = x;
    size_t  write  = 0;

    for (size_t read = 0; !(list[read].x == -1 && list[read].y == -1); read++) {
        if (abs(list[read].x - center.x) <= 1 &&
            abs(list[read].y - center.y) <= 1) {
            list[write] = list[read];
            write++;
        }
    }

    list[write] = SQUARE_END;

    return true;
}

/// eff_palace_decree
///
/// Immediate play effect attaching the enemy-king confinement to the
/// enemy side's list for two turns.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> played card, unused
///
/// Return: true when the decree was attached
///
static bool eff_palace_decree(EffectContext* context, void* x) {
    (void) x;

    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];
    Side         enemy  = battle_enemy(side);
    PieceInfo*   king   = battle_find_king(battle, enemy);

    if (!king) {
        return false;
    }

    static const Effect zone = {
        .func      = eff_palace_zone,
        .name      = "Palace Decree",
        .trigger   = QUERY_PIECE_MOVES,
        .lasts_for = TURNS_2,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, enemy)->effects, &zone);

    if (attached) {
        attached->context->args[1] =
            (void*) (uintptr_t) (king->square.y * 20 + king->square.x);
    }

    return attached != nullptr;
}

/// eff_mingzhu_seal
///
/// The seal itself: while attached to the target's own side list it vetoes
/// that one piece's movement.
///
/// Params:
/// - context -> args[1] the sealed piece pointer
/// - x       -> bool* move eligibility to veto
///
/// Return: true when the sealed piece was blocked
///
static bool eff_mingzhu_seal(EffectContext* context, void* x) {
    if (battle_subject() != context->args[1]) {
        return false;
    }

    *(bool*) x = false;

    return true;
}

/// eff_mingzhu_targets
///
/// Mingzhu's Seal targeting: advertises every enemy piece, the king
/// included, per the GDD "target 1 enemy piece".
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one enemy was offered
///
static bool eff_mingzhu_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side != side && p->side != SIDE_NEUTRAL;
    });

    return card_target_count(x) > 0;
}

/// eff_mingzhu_pick
///
/// Mingzhu's Seal resolution: one enemy piece cannot move for three turns.
/// Attaches the seal to the target's own side list keyed on the piece.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when a piece was sealed
///
static bool eff_mingzhu_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side == side) {
        return false;
    }

    static const Effect seal = {
        .func      = eff_mingzhu_seal,
        .name      = "Mingzhu's Seal",
        .trigger   = QUERY_PIECE_CAN_MOVE,
        .lasts_for = TURNS_3,
    };

    Effect* attached = effect_attach(
        &battle_player(battle, piece->side)->effects,
        &seal
    );

    if (attached) {
        attached->context->args[1] = piece;
    }

    return attached != nullptr;
}

/// eff_mandate_targets
///
/// Mandate targeting: advertises every friendly non-king piece as a square
/// target.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* list to append to
///
/// Return: true when at least one target was offered
///
static bool eff_mandate_targets(EffectContext* context, void* x) {
    Side side = (Side) (uintptr_t) context->args[0];

    card_targets_piece(x, battle_current(), ^bool(const PieceInfo* p) {
        return p->side == side && p->piece->id != PIECE_KING;
    });

    return card_target_count(x) > 0;
}

/// eff_mandate_pick
///
/// Mandate resolution: removes the chosen friendly piece and deals three
/// times its value to the enemy meter.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> CardTarget* chosen square
///
/// Return: true when the piece was spent
///
static bool eff_mandate_pick(EffectContext* context, void* x) {
    CardTarget*  target = x;
    BattleState* battle = battle_current();
    Side         side   = (Side) (uintptr_t) context->args[0];

    if (target->kind != TARGET_PIECE) {
        return false;
    }

    PieceInfo* piece = battle_at(battle, card_target_at(target->value));

    if (!piece || piece->side != side || piece->piece->id == PIECE_KING) {
        return false;
    }

    int dmg = battle_value(battle, piece, nullptr) * 3;

    battle_remove(battle, piece);
    battle_damage(battle, battle_enemy(side), dmg);

    return true;
}

/// eff_longwei_climax
///
/// Halves the damage taken by every friendly piece for the Longwei combo
/// climax this turn.
///
/// Params:
/// - context -> beneficiary side in args[0]
/// - x       -> incoming damage value
///
/// Return: true when the reduction applied
///
static bool eff_longwei_climax(EffectContext* context, void* x) {
    Side       side    = (Side) (uintptr_t) context->args[0];
    PieceInfo* subject = battle_subject();

    if (!subject || subject->side != side) {
        return false;
    }

    int* damage = x;
    *damage     = *damage * 50 / 100;

    return true;
}

/*----------------------------------------------------------------------------*\
                                 KINGDOM DATA
\*----------------------------------------------------------------------------*/

const Piece LONGWEI_PIECES[] = {
    {
        .at      = bing_at,
        .mv      = bing_mv,
        .effects =
            {
                {
                    .func      = eff_bing_blood,
                    .name      = "First Blood",
                    .trigger   = QUERY_PIECE_DAMAGE_DEALT,
                    .lasts_for = ENTIRE_BATTLE,
                },
                {
                    .func      = eff_bing_sideways,
                    .name      = "Sidestep",
                    .trigger   = QUERY_PIECE_MOVES,
                    .lasts_for = ENTIRE_BATTLE,
                },
                {
                    .func      = eff_bing_sideways_at,
                    .name      = "Sidestep",
                    .trigger   = QUERY_PIECE_ATTACKS,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Bing",
        .desc    = "Forward 1. Gains a sideways step after first damaging "
                   "an enemy piece.",
        .id      = PIECE_BING,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 10,
    },
    {
        .at      = xiang_at,
        .mv      = xiang_mv,
        .effects =
            {
                {
                    .func      = eff_xiang_territory,
                    .name      = "River Ban",
                    .trigger   = QUERY_PIECE_CAN_ATTACK,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Xiang",
        .desc    = "2 diagonal. Cannot attack in enemy territory.",
        .id      = PIECE_XIANG,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_DISTRICT,
        .class   = MOVE_LEAPER,
        .value   = 20,
    },
    {
        .at      = ma_at,
        .mv      = ma_mv,
        .name    = "Ma",
        .desc    = "Knight L. Blocked if any piece sits in the elbow "
                   "square.",
        .id      = PIECE_MA,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_TOWN,
        .class   = MOVE_SPECIAL,
        .value   = 30,
    },
    {
        .at      = pao_at,
        .mv      = pao_mv,
        .name    = "Pao",
        .desc    = "Rook-line movement. Attacks only across exactly one "
                   "screen piece.",
        .id      = PIECE_PAO,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SLIDER,
        .value   = 50,
    },
    {
        .at      = liubo_at,
        .mv      = liubo_mv,
        .name    = "Liubo Diviner",
        .desc    = "Teleports to any square currently threatened by an "
                   "enemy piece.",
        .id      = PIECE_LIUBO_DIVINER,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_COUNTRY,
        .class   = MOVE_SPECIAL,
        .value   = 100,
    },
    {
        .at      = sang_at,
        .mv      = sang_mv,
        .name    = "Sang",
        .desc    = "1 orthogonal then 2 diagonal. Blockable at either "
                   "intermediate square.",
        .id      = PIECE_SANG,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_TOWN,
        .class   = MOVE_SPECIAL,
        .value   = 30,
    },
    {
        .at      = cavalry_at,
        .mv      = cavalry_mv,
        .effects =
            {
                {
                    .func      = eff_cavalry_bulwark,
                    .name      = "Cavalry Bulwark",
                    .trigger   = QUERY_PIECE_DAMAGE_TAKEN,
                    .lasts_for = ENTIRE_BATTLE,
                },
            },
        .name    = "Northern Cavalry",
        .desc    = "Knight L. Grants Bulwark to orthogonally adjacent "
                   "allies.",
        .id      = PIECE_NORTHERN_CAVALRY,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_LEAPER,
        .value   = 40,
    },
    {
        .at      = hwacha_at,
        .mv      = hwacha_mv,
        .name    = "Hwacha",
        .desc    = "Cannon screen rule applied to rows, columns, and "
                   "diagonals.",
        .id      = PIECE_HWACHA,
        .kingdom = KINGDOM_LONGWEI,
        .tier    = TIER_PROVINCE,
        .class   = MOVE_SLIDER,
        .value   = 60,
    },
};

const Card LONGWEI_CARDS[] = {
    {
        .effects =
            {{.func      = eff_river_wade_targets,
              .name      = "River Wade",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_river_wade_pick,
              .name      = "River Wade",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "River Wade",
        .desc      = "Target pawn permanently gains a sideways step this "
                     "battle.",
        .id        = CARD_RIVER_WADE,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_charge_targets,
              .name      = "Charge",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_charge_pick,
              .name      = "Charge",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Charge",
        .desc      = "Target slider may pass through one occupied square "
                     "on its next move.",
        .id        = CARD_CHARGE,
        .tier      = TIER_DISTRICT,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 0,
        .sell_cost = 15,
    },
    {
        .effects =
            {{.func      = eff_formation,
              .name      = "Formation",
              .trigger   = QUERY_PIECE_DAMAGE_DEALT,
              .lasts_for = TURNS_1}},
        .name      = "Formation",
        .desc      = "3 of your pieces in a straight line each deal +50% "
                     "damage this turn.",
        .id        = CARD_FORMATION,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 0,
        .sell_cost = 30,
    },
    {
        .effects =
            {{.func      = eff_divination,
              .name      = "Divination",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Divination",
        .desc      = "Reveal the enemy's intended moves and cards for next "
                     "turn.",
        .id        = CARD_DIVINATION,
        .tier      = TIER_TOWN,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 0,
        .sell_cost = 25,
    },
    {
        .effects =
            {{.func      = eff_cannon_volley_targets,
              .name      = "Cannon Volley",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_cannon_volley_pick,
              .name      = "Cannon Volley",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Cannon Volley",
        .desc      = "Target Pao attacks every enemy on its row and column "
                     "this turn, ignoring screens.",
        .id        = CARD_CANNON_VOLLEY,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 30,
        .sell_cost = 45,
    },
    {
        .effects =
            {{.func      = eff_palace_decree,
              .name      = "Palace Decree",
              .trigger   = ON_CARD_PLAY,
              .lasts_for = TURNS_1}},
        .name      = "Palace Decree",
        .desc      = "Enemy king restricted to a 3x3 zone for 2 turns.",
        .id        = CARD_PALACE_DECREE,
        .tier      = TIER_PROVINCE,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 0,
        .sell_cost = 50,
    },
    {
        .effects =
            {{.func      = eff_mandate_targets,
              .name      = "Mandate",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = TURNS_1},
             {.func      = eff_mandate_pick,
              .name      = "Mandate",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = TURNS_1}},
        .name      = "Mandate",
        .desc      = "Remove one of your pieces. Deal damage to enemy "
                     "meter by its value x3.",
        .id        = CARD_MANDATE,
        .tier      = TIER_COUNTRY,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 0,
        .sell_cost = 75,
    },
    {
        .effects =
            {{.func      = eff_mingzhu_targets,
              .name      = "Mingzhu's Seal",
              .trigger   = QUERY_CARD_TARGETS,
              .lasts_for = ENTIRE_BATTLE},
             {.func      = eff_mingzhu_pick,
              .name      = "Mingzhu's Seal",
              .trigger   = ON_CARD_TARGET_SELECTED,
              .lasts_for = ENTIRE_BATTLE}},
        .name      = "Mingzhu's Seal",
        .desc      = "Target 1 enemy piece. It cannot move for 3 turns.",
        .id        = CARD_MINGZHUS_SEAL,
        .tier      = TIER_MASTERY,
        .kingdom   = KINGDOM_LONGWEI,
        .play_cost = 30,
        .sell_cost = 50,
    },
};

/// eff_palace
///
/// The Palace: the king cannot leave a three-by-three zone. The zone
/// centres on the king's starting square, captured lazily on the first
/// fire; move entries outside the box are dropped from the list.
///
/// Params:
/// - context -> args[1] captured center x + 1, args[2] center y + 1
/// - x       -> Square* king move list to prune
///
/// Return: true when the list was pruned
///
static bool eff_palace(EffectContext* context, void* x) {
    PieceInfo* subject = battle_subject();

    if (!subject || subject->piece->id != PIECE_KING) {
        return false;
    }

    if (!context->args[1]) {
        context->args[1] = (void*) (uintptr_t) (subject->square.x + 1);
        context->args[2] = (void*) (uintptr_t) (subject->square.y + 1);
    }

    int cx = (int) (uintptr_t) context->args[1] - 1;
    int cy = (int) (uintptr_t) context->args[2] - 1;

    Square* moves  = x;
    size_t  write  = 0;
    bool    pruned = false;

    for (size_t read = 0; !(moves[read].x == -1 && moves[read].y == -1);
         read++) {
        int dx = moves[read].x - cx;
        int dy = moves[read].y - cy;

        dx     = dx < 0 ? -dx : dx;
        dy     = dy < 0 ? -dy : dy;

        if (dx > 1 || dy > 1) {
            pruned = true;
            continue;
        }

        moves[write] = moves[read];
        write++;
    }

    moves[write] = SQUARE_END;

    return pruned;
}

/// eff_river_crossing
///
/// River Crossing: a Xiang cannot cross the river that bisects the board.
/// Move entries carrying it to the far half are dropped from the list.
///
/// Params:
/// - context -> unused
/// - x       -> Square* Xiang move list to prune
///
/// Return: true when the list was pruned
///
static bool eff_river_crossing(EffectContext* context, void* x) {
    (void) context;

    PieceInfo* subject = battle_subject();

    if (!subject || subject->piece->id != PIECE_XIANG) {
        return false;
    }

    int8_t  river  = (int8_t) (battle_current()->board.height / 2);
    bool    near   = subject->square.y >= river;
    Square* moves  = x;
    size_t  write  = 0;
    bool    pruned = false;

    for (size_t read = 0; !(moves[read].x == -1 && moves[read].y == -1);
         read++) {
        if ((moves[read].y >= river) != near) {
            pruned = true;
            continue;
        }

        moves[write] = moves[read];
        write++;
    }

    moves[write] = SQUARE_END;

    return pruned;
}

/// eff_river_bing
///
/// River Crossing: a Bing that steps across the river earns a permanent
/// sideways step, the same grant River Wade gives, once.
///
/// Params:
/// - context -> unused
/// - x       -> PieceInfo* the piece that just moved
///
/// Return: true when the sideways step was granted
///
static bool eff_river_bing(EffectContext* context, void* x) {
    (void) context;

    PieceInfo* piece = x;

    if (piece->piece->id != PIECE_BING) {
        return false;
    }

    int8_t river = (int8_t) (battle_current()->board.height / 2);
    Square from  = battle_move_from();

    if ((from.y >= river) == (piece->square.y >= river)) {
        return false;
    }

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (piece->piece->effects[slot].func == eff_sideways_grant) {
            return false;
        }
    }

    static const Effect grant = {
        .func      = eff_sideways_grant,
        .name      = "River Crossing",
        .trigger   = QUERY_PIECE_MOVES,
        .lasts_for = ENTIRE_BATTLE,
    };

    return piece_embed_effect(piece, &grant) != nullptr;
}

const BoardTrait LONGWEI_TRAITS[] = {
    {
        .name    = "River Crossing",
        .desc    = "A river bisects the board; Xiang cannot cross, a Bing "
                   "gains a sideways step on crossing.",
        .id      = BOARD_TRAIT_RIVER_CROSSING,
        .effects = {
            {
                .func      = eff_river_crossing,
                .name      = "River Crossing",
                .trigger   = QUERY_PIECE_MOVES,
                .lasts_for = ENTIRE_BATTLE,
            },
            {
                .func      = eff_river_bing,
                .name      = "River Crossing",
                .trigger   = ON_PIECE_MOVE,
                .lasts_for = ENTIRE_BATTLE,
            },
        },
    },
    {
        .name      = "The Palace",
        .desc      = "A 3x3 zone near the king. The king cannot leave it.",
        .id        = BOARD_TRAIT_THE_PALACE,
        .effects   = {{
            .func      = eff_palace,
            .name      = "The Palace",
            .trigger   = QUERY_PIECE_MOVES,
            .lasts_for = ENTIRE_BATTLE,
        }},
    },
};

/*----------------------------------------------------------------------------*\
                              KINGDOM MECHANICS
\*----------------------------------------------------------------------------*/

/// eff_bulwark
///
/// Bulwark: a Longwei piece with an orthogonally adjacent friendly piece
/// takes reduced damage — half normally, sixty percent less at mastery
/// three.
///
/// Params:
/// - context -> args[0] owning side, args[1] mastery level
/// - x       -> int* incoming damage to reduce
///
/// Return: true when the reduction applied
///
static bool eff_bulwark(EffectContext* context, void* x) {
    Side         side  = (Side) (uintptr_t) context->args[0];
    MasteryLevel level = (MasteryLevel) (uintptr_t) context->args[1];
    PieceInfo*   piece = battle_subject();

    if (!piece || piece->side != side ||
        piece->piece->kingdom != KINGDOM_LONGWEI) {
        return false;
    }

    BattleState* battle  = battle_current();
    bool         guarded = false;

    for (size_t o = 0; ORTHOGONAL_DIRECTIONS[o].x != 0 ||
                       ORTHOGONAL_DIRECTIONS[o].y != 0;
         o++) {
        Square adjacent = {
            (int8_t) (piece->square.x + ORTHOGONAL_DIRECTIONS[o].x),
            (int8_t) (piece->square.y + ORTHOGONAL_DIRECTIONS[o].y),
        };

        PieceInfo* neighbour = battle_at(battle, adjacent);

        if (neighbour && neighbour != &VOID_CELL && neighbour->side == side) {
            guarded = true;
            break;
        }
    }

    if (!guarded) {
        return false;
    }

    int reduction = level >= MASTERY_LEVEL_3 ? 60 : 50;

    *(int*) x = *(int*) x * (100 - reduction) / 100;

    return true;
}

/// longwei_innate
///
/// Attaches the Bulwark innate to the given side at the given mastery
/// level.
///
/// Params:
/// - battle -> battle to attach into
/// - side   -> side receiving the innate
/// - level  -> mastery level scaling the innate
///
void longwei_innate(BattleState* battle, Side side, MasteryLevel level) {
    Effect bulwark = {
        .func      = eff_bulwark,
        .name      = "Bulwark",
        .trigger   = QUERY_PIECE_DAMAGE_TAKEN,
        .lasts_for = ENTIRE_BATTLE,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &bulwark);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
        attached->context->args[1] = (void*) (uintptr_t) level;
    }
}

/// longwei_climax
///
/// Fires the Longwei combo climax for the given side, granting every
/// friendly piece Bulwark this turn.
///
/// Params:
/// - battle -> battle the climax fires in
/// - side   -> side that completed the combo chain
///
void longwei_climax(BattleState* battle, Side side) {
    static const Effect climax = {
        .func      = eff_longwei_climax,
        .name      = "Longwei Climax",
        .trigger   = QUERY_PIECE_DAMAGE_TAKEN,
        .lasts_for = TURNS_1,
    };

    Effect* attached =
        effect_attach(&battle_player(battle, side)->effects, &climax);

    if (attached) {
        attached->context->args[0] = (void*) (uintptr_t) side;
    }
}

/// longwei_overseer
///
/// Sets up the Iron Strategist overseer battle with its bespoke army
/// and boss rules.
///
/// Params:
/// - battle -> battle to set up
///
void longwei_overseer(BattleState* battle) {
    (void) battle;
}

/// longwei_event
///
/// Resolves a Longwei narrative event with the player's choice.
///
/// Params:
/// - engine -> engine owning the run
/// - id     -> event being resolved
/// - choice -> choice taken by the player
///
void longwei_event(EngineState* engine, EventID id, EventChoice choice) {
    (void) engine;
    (void) id;
    (void) choice;
}
