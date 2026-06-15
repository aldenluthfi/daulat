//! map.c
//!
//! Overworld map generator + campaign-loop driver. Maps are linear
//! 9-node sequences seeded by `(kingdom, tier, seed)`; node types
//! follow the GDD § 6 distribution. Battle / event / archive
//! resolution is dispatched by the screen layer through map_advance.
//!
//! Created: 2026-06-14
//! Author : Alden Luthfi

#include "prelude.h"

/*--------------------------------------------------------------------------*\
                              GENERATOR HELPERS
\*--------------------------------------------------------------------------*/

/// Default node layout per (tier) — 9 nodes per map.
/// Town / Province: 5 battles + 1 elite + 1 offering + 2 events
/// Country:         4 battles + 1 elite + 1 offering + 2 events + 1 overseer
static void layout_for_tier(Tier tier, NodeType* out, uint8_t* count_out) {
    static const NodeType TOWN_PROVINCE[] = {
        NODE_BATTLE, NODE_BATTLE, NODE_EVENT,
        NODE_ELITE,  NODE_BATTLE, NODE_OFFERING,
        NODE_EVENT,  NODE_BATTLE, NODE_BATTLE
    };
    static const NodeType COUNTRY[] = {
        NODE_BATTLE, NODE_BATTLE, NODE_EVENT,
        NODE_ELITE,  NODE_BATTLE, NODE_OFFERING,
        NODE_EVENT,  NODE_BATTLE, NODE_OVERSEER
    };
    const NodeType* src = (tier == TIER_COUNTRY) ? COUNTRY : TOWN_PROVINCE;
    size_t          n   = (tier == TIER_COUNTRY)
                       ? sizeof(COUNTRY) / sizeof(COUNTRY[0])
                       : sizeof(TOWN_PROVINCE) / sizeof(TOWN_PROVINCE[0]);
    for (size_t i = 0; i < n; i++)
        out[i] = src[i];
    *count_out = (uint8_t)n;
}

static uint16_t roll_event_payload(Rng* rng, Kingdom k) {
    static const EventId UNIVERSAL[] = {
        EVENT_UNIVERSAL_WANDERER,
        EVENT_UNIVERSAL_MARKET,
        EVENT_UNIVERSAL_AMBUSH
    };
    static const EventId LONGWEI[]   = {
        EVENT_LONGWEI_RIVER_FESTIVAL, EVENT_LONGWEI_SKY_LADDER
    };
    static const EventId HARUSHIMA[] = {
        EVENT_HARUSHIMA_HONOR_TRIAL, EVENT_HARUSHIMA_TAKENOKO_SWARM
    };
    static const EventId KEWARANI[]  = {
        EVENT_KEWARANI_PILGRIMAGE_SEASON, EVENT_KEWARANI_BAZAAR_RUMOR
    };
    static const EventId ZARQAN[]    = {
        EVENT_ZARQAN_SANDSTORM, EVENT_ZARQAN_OASIS_DISCOVERY
    };
    static const EventId CAELAN[]    = {
        EVENT_CAELAN_TOURNAMENT, EVENT_CAELAN_CASTLE_FESTIVAL
    };

    if ((rng_next(rng) & 1u) == 0u)
        return UNIVERSAL[rng_range(rng, 3)];
    switch (k) {
        case KINGDOM_LONGWEI:   return LONGWEI[rng_range(rng, 2)];
        case KINGDOM_HARUSHIMA: return HARUSHIMA[rng_range(rng, 2)];
        case KINGDOM_KEWARANI:  return KEWARANI[rng_range(rng, 2)];
        case KINGDOM_ZARQAN:    return ZARQAN[rng_range(rng, 2)];
        case KINGDOM_CAELAN:    return CAELAN[rng_range(rng, 2)];
        default:                return UNIVERSAL[0];
    }
}

static uint16_t roll_overseer_for(Kingdom k) {
    switch (k) {
        case KINGDOM_LONGWEI:   return OVERSEER_IRON_STRATEGIST;
        case KINGDOM_HARUSHIMA: return OVERSEER_ETERNAL_RECURSION;
        case KINGDOM_KEWARANI:  return OVERSEER_CARAVAN_OF_CONQUEST;
        case KINGDOM_ZARQAN:    return OVERSEER_MANY_FACED_KING;
        case KINGDOM_CAELAN:    return OVERSEER_CROWNED_HERETIC;
        default:                return OVERSEER_IRON_STRATEGIST;
    }
}

/*--------------------------------------------------------------------------*\
                              GENERATOR
\*--------------------------------------------------------------------------*/

void map_generate(MapState* map, Kingdom k, Tier tier, uint64_t seed) {
    Rng rng;
    rng_init(&rng, seed);

    NodeType layout[MAX_MAP_NODES];
    uint8_t  layout_count;
    layout_for_tier(tier, layout, &layout_count);

    map->kingdom         = k;
    map->map_tier        = tier;
    map->seed            = seed;
    map->node_count      = layout_count;
    map->entry_node_id   = 0;
    map->current_node_id = MAP_NODE_INVALID;

    for (uint8_t i = 0; i < layout_count; i++) {
        MapNode* node      = &map->nodes[i];
        node->id           = i;
        node->type         = (uint8_t)layout[i];
        node->kingdom      = (uint8_t)k;
        node->visited      = false;
        node->revealed     = false;
        node->edge_count   = (uint8_t)((i + 1 < layout_count) ? 1 : 0);
        node->edges[0]     = (uint16_t)(i + 1);
        node->modifier_id  = (uint16_t)(rng_range(&rng, MODIFIER_COUNT));
        node->trait_id     = (uint16_t)(rng_range(&rng, TRAIT_COUNT));

        switch (layout[i]) {
            case NODE_EVENT:
                node->payload_id = roll_event_payload(&rng, k);
                break;
            case NODE_OVERSEER:
                node->payload_id = roll_overseer_for(k);
                break;
            case NODE_ARCHIVE:
                node->payload_id = (uint16_t)rng_range(&rng, MAX_RECIPES);
                break;
            default:
                node->payload_id = 0;
                break;
        }
    }
}

/*--------------------------------------------------------------------------*\
                              NAVIGATION
\*--------------------------------------------------------------------------*/

const MapNode* map_current_node(const MapState* map) {
    if (map->current_node_id >= map->node_count)
        return NULL;
    return &map->nodes[map->current_node_id];
}

bool map_can_advance(const MapState* map, uint16_t to_id) {
    if (to_id >= map->node_count)
        return false;
    if (map->current_node_id == MAP_NODE_INVALID) {
        /* From the virtual entry, only the entry node is reachable. */
        return to_id == map->entry_node_id;
    }
    const MapNode* current = &map->nodes[map->current_node_id];
    for (uint8_t i = 0; i < current->edge_count; i++)
        if (current->edges[i] == to_id)
            return true;
    return false;
}

/// Push the appropriate screen for the resolved node type. Battle /
/// event / overseer / liberation push their own screens; archive /
/// offering resolve immediately on the map.
static void enter_node(EngineState* engine, MapNode* node) {
    switch ((NodeType)node->type) {
        case NODE_BATTLE:
        case NODE_ELITE:
        case NODE_OVERSEER:
        case NODE_LIBERATION_TRIAL:
            screen_goto(engine, SCREEN_BATTLE);
            break;
        case NODE_EVENT:
            screen_goto(engine, SCREEN_EVENT);
            break;
        case NODE_ARCHIVE:
            if (engine->run != NULL && node->payload_id < MAX_RECIPES)
                engine->run->revealed_recipes |= (1ULL << node->payload_id);
            node->visited = true;
            break;
        case NODE_OFFERING:
            /* The in-map remove-card prompt is surfaced here. */
            node->visited = true;
            break;
        default:
            break;
    }
}

bool map_advance(EngineState* engine, uint16_t to_id) {
    if (engine == NULL || engine->run == NULL)
        return false;
    MapState* map = &engine->run->current_map;
    if (!map_can_advance(map, to_id))
        return false;
    map->current_node_id = to_id;
    enter_node(engine, &map->nodes[to_id]);
    run_save(engine->run);
    return true;
}

/*--------------------------------------------------------------------------*\
                              BATTLE CALLBACKS
\*--------------------------------------------------------------------------*/

static void clear_kingdom_if_overseer(EngineState* engine, MapNode* node) {
    if (engine->run == NULL)
        return;
    if (node->type != NODE_OVERSEER)
        return;
    Kingdom k = (Kingdom)node->kingdom;
    if ((unsigned)k >= KINGDOM_COUNT)
        return;
    engine->run->cleared_kingdoms[k] = true;
    for (size_t i = 0; i < KINGDOM_COUNT; i++) {
        if (i == (size_t)k)
            continue;
        if (engine->run->vorath_pressure < 255)
            engine->run->vorath_pressure++;
    }
}

void map_on_battle_won(EngineState* engine) {
    if (engine == NULL || engine->run == NULL)
        return;
    MapState* map = &engine->run->current_map;
    if (map->current_node_id >= map->node_count)
        return;
    MapNode* node = &map->nodes[map->current_node_id];
    node->visited = true;

    Kingdom k = map->kingdom;
    if ((unsigned)k < KINGDOM_COUNT
        && engine->run->chain_levels[k] > 0)
        engine->run->chain_levels[k]--;

    clear_kingdom_if_overseer(engine, node);
    run_save(engine->run);
}

static void seed_liberation_trial(RunState* run, Kingdom subjugated_k) {
    MapState* map = &run->current_map;
    for (uint8_t i = 0; i < map->node_count; i++) {
        MapNode* node = &map->nodes[i];
        if (node->type == NODE_EVENT && !node->visited) {
            node->type       = NODE_LIBERATION_TRIAL;
            node->kingdom    = (uint8_t)subjugated_k;
            node->payload_id = 0;
            return;
        }
    }
}

void map_on_battle_lost(EngineState* engine) {
    if (engine == NULL || engine->run == NULL)
        return;
    RunState* run = engine->run;
    Kingdom   k   = run->current_map.kingdom;
    if ((unsigned)k >= KINGDOM_COUNT)
        return;

    if (run->chain_levels[k] < 3)
        run->chain_levels[k]++;
    run->vorath_counter++;
    run->mastery_disqualified[k] = true;
    if (run->chain_levels[k] >= 3 && !run->subjugated[k]) {
        run->subjugated[k] = true;
        for (Kingdom other = 0; other < KINGDOM_COUNT; other++) {
            if (other == k || run->subjugated[other])
                continue;
            seed_liberation_trial(run, k);
            break;
        }
    }
    run_save(run);
}
