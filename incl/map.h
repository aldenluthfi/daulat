//! map.h
//!
//! Overworld map: nodes, edges, deterministic generator, and
//! navigation. A `MapState` covers a single (kingdom, tier) pair
//! (Town / Province / Country). The campaign keeps the player on
//! one map at a time; map advancement triggers the appropriate
//! battle / event / archive resolution path.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#ifndef MAP_H
#define MAP_H

#include "defs.h"

struct App;
struct RunState;

/*--------------------------------------------------------------------------*\
                              SIZES
\*--------------------------------------------------------------------------*/

#define MAX_MAP_NODES    24
#define MAX_NODE_EDGES   4
#define MAP_NODE_INVALID 0xFFFFu

/*--------------------------------------------------------------------------*\
                              NODE TYPES
\*--------------------------------------------------------------------------*/

/// NodeType
///
/// Classification of every overworld node. The campaign loop reads
/// this to decide which downstream screen to push (battle, event,
/// etc.).
///
typedef enum {
    NODE_BATTLE,
    NODE_ELITE,
    NODE_ARCHIVE,
    NODE_OFFERING,
    NODE_EVENT,
    NODE_OVERSEER,
    NODE_LIBERATION_TRIAL,
    NODE_TYPE_COUNT
} NodeType;

/*--------------------------------------------------------------------------*\
                              MAP NODE
\*--------------------------------------------------------------------------*/

/// MapNode
///
/// A single overworld node. `payload_id` carries the type-specific
/// content (event id, overseer id, recipe id...). `modifier_id` /
/// `trait_id` are pre-rolled per battle so the player can see them
/// in the projection panel before entering.
///
typedef struct {
    uint16_t id;
    uint8_t  type;     /* NodeType */
    uint8_t  kingdom;  /* Kingdom */
    uint16_t payload_id;
    uint16_t modifier_id;
    uint16_t trait_id;
    bool     visited;
    bool     revealed;
    uint8_t  edge_count;
    uint16_t edges[MAX_NODE_EDGES];
} MapNode;

/*--------------------------------------------------------------------------*\
                              MAP STATE
\*--------------------------------------------------------------------------*/

/// MapState
///
/// The current overworld map. Nodes are dense in `nodes[0..node_count)`;
/// `entry_node_id` is the player's starting node and
/// `current_node_id` advances on navigation.
///
typedef struct {
    Kingdom  kingdom;
    Tier     map_tier;
    uint64_t seed;
    MapNode  nodes[MAX_MAP_NODES];
    uint8_t  node_count;
    uint16_t entry_node_id;
    uint16_t current_node_id;
} MapState;

/*--------------------------------------------------------------------------*\
                              API
\*--------------------------------------------------------------------------*/

/// map_generate
///
/// Procedurally build a map for the given (kingdom, tier) pair.
/// The result is deterministic in `seed`. Node counts and types
/// follow the GDD § 6 layout (5 battles + 1 elite + 1 offering +
/// 2 events per Town/Province map; Country swaps the last battle
/// for an Overseer).
///
/// Params:
/// - MapState* map   -> destination
/// - Kingdom   k     -> kingdom of the map
/// - Tier      tier  -> tier (DISTRICT/TOWN/PROVINCE/COUNTRY)
/// - uint64_t  seed  -> RNG seed
///
void map_generate(MapState* map, Kingdom k, Tier tier, uint64_t seed);

/// map_can_advance
///
/// Is `to_id` a legal next step from the current node?
///
/// Params:
/// - const MapState* map  -> current map
/// - uint16_t       to_id -> candidate destination
///
/// Return:
/// bool -> true if `to_id` is reachable from current_node_id
///
bool map_can_advance(const MapState* map, uint16_t to_id);

/// map_advance
///
/// Take the step to `to_id`: mark visited, switch the screen stack
/// to the destination's resolution screen (battle / event / etc.),
/// and autosave the run. Returns false if the step is illegal.
///
/// Params:
/// - struct App* app    -> app holding the active run
/// - uint16_t    to_id  -> destination node id
///
/// Return:
/// bool -> true on success
///
bool map_advance(struct App* app, uint16_t to_id);

/// map_on_battle_won
///
/// Called from the battle screen after a victory. Marks the
/// current node visited, decrements the kingdom's chain by one,
/// applies Overseer-specific bookkeeping (kingdom cleared,
/// synergy unlock, vorath pressure), and autosaves.
///
/// Params:
/// - struct App* app -> app holding the active run
///
void map_on_battle_won(struct App* app);

/// map_on_battle_lost
///
/// Called from the battle screen after a defeat. Increments the
/// kingdom's chain, bumps the Vorath counter, may flag the kingdom
/// subjugated and seed a Liberation Trial elsewhere, and autosaves.
///
/// Params:
/// - struct App* app -> app holding the active run
///
void map_on_battle_lost(struct App* app);

/// map_current_node
///
/// Pointer to the current node, or NULL on an invalid id.
///
/// Params:
/// - const MapState* map -> map to query
///
/// Return:
/// const MapNode* -> current node or NULL
///
const MapNode* map_current_node(const MapState* map);

#endif /* MAP_H */
