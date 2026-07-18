//! run.c
//!
//! Run-scale campaign logic. Owns the static map layout tables, map
//! generation, node selection, unlock schedules, chain bookkeeping, the
//! Vorath counter, events, offerings, and relic picks.
//!
//! Created: 12/07/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/*----------------------------------------------------------------------------*\
                                 LAYOUT TABLES
\*----------------------------------------------------------------------------*/

static const MapNode LONGWEI_TOWN[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Copper Gate", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Western Market", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The River Quarter", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Silk Exchange", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Second Wall", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Prefecture Garrison",
     .content = RELIC_CHOICE(RELIC_SOUL_SHARD, RELIC_MINTED_COIN)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Hall of Records",
     .content = PIECE_SANG},
    {.type = MAP_NODE_OFFERING, .name = "The River Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Governor's Courtyard",
     .content = EVENT_DRAGON_COURT_TRIBUTE},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Janggi Elder's School",
     .content = EVENT_JANGGI_ELDER},
};

static const MapNode LONGWEI_PROVINCE[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Dragon Bridge", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Eastern Checkpoint", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Iron Mines", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Great Canal", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Silk Road Station", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Border Fortress",
     .content = RELIC_CHOICE(RELIC_TACTICIANS_SCROLL, RELIC_EAGLE_EYE)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Imperial Library",
     .content = PIECE_HWACHA},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Battle Codex",
     .content = PIECE_NORTHERN_CAVALRY},
    {.type = MAP_NODE_OFFERING, .name = "The War Memorial", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Cannon Works",
     .content = EVENT_CANNON_SALUTE},
    {.type    = MAP_NODE_EVENT,
     .name    = "The River Crossing Post",
     .content = EVENT_DEFECTOR},
};

static const MapNode LONGWEI_COUNTRY[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Palace Approach", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Outer City", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Dragon Gate", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Throne Antechamber", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Palace Guard",
     .content = RELIC_CHOICE(RELIC_PHILOSOPHERS_STONE, RELIC_IRON_KING)},
    {.type = MAP_NODE_OFFERING, .name = "The Dragon Altar", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Scholar's Garden",
     .content = EVENT_SCHOLARS_OFFER},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Imperial Court",
     .content = EVENT_VORATHS_DECREE},
    {.type    = MAP_NODE_OVERSEER,
     .name    = "The Hall of Ten Thousand Victories",
     .content = OVERSEER_IRON_STRATEGIST},
};

static const MapNode KEWARANI_TOWN[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Market of Addis", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Well Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Spice Quarter", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Caravan Gate", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Outpost Road", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Market Garrison",
     .content = RELIC_CHOICE(RELIC_MERCHANTS_LEDGER, RELIC_BULK_DISCOUNT)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Salt Road Archive",
     .content = PIECE_MEDEQ_SQUAD},
    {.type = MAP_NODE_OFFERING, .name = "The Road Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Salt Road",
     .content = EVENT_SALT_ROAD_MERCHANT},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Camel Traders' Camp",
     .content = EVENT_CAMEL_CARAVAN},
};

static const MapNode KEWARANI_PROVINCE[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Highland Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Ancient Ford", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Thorn Pass", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Lowland Camp", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Plateau Fort", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Highland Garrison",
     .content = RELIC_CHOICE(RELIC_WAR_CHEST, RELIC_FORWARD_COMMAND)},
    {.type = MAP_NODE_OFFERING, .name = "The Rain Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Court House",
     .content = EVENT_MANSAS_COURT},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Festival Grounds",
     .content = EVENT_FEAST_OF_YOD_ABEGA},
};

static const MapNode KEWARANI_COUNTRY[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Palace Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The City Gates", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Inner Court", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Throne Hall Approach", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Royal Guard",
     .content = RELIC_CHOICE(RELIC_DEEP_HAND, RELIC_LAST_BREATH)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Negus Guard Archive",
     .content = PIECE_SULTANS_LEVY},
    {.type = MAP_NODE_OFFERING, .name = "The Sacred Spring", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Prison Tower",
     .content = EVENT_STOLEN_GUARD},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Road's End",
     .content = EVENT_WANDERING_PIECE},
    {.type    = MAP_NODE_OVERSEER,
     .name    = "The Throne of the Negesta",
     .content = OVERSEER_CARAVAN_OF_CONQUEST},
};

static const MapNode ZARQAN_TOWN[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Oasis Market", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Caravanserai", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Date Merchants", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Copper Bazaar", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Southern Gate", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Garrison of the Dunes",
     .content = RELIC_CHOICE(RELIC_TAX_STAMP, RELIC_FORTIFIED_LINE)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Wazir's Library",
     .content = PIECE_CATAPHRACT},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Rukh Records",
     .content = PIECE_ROOK},
    {.type = MAP_NODE_OFFERING, .name = "The Desert Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Desert Arena",
     .content = EVENT_WARLORDS_CHALLENGE},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Grand Bazaar",
     .content = EVENT_BAZAAR_OF_SAMARKAND},
};

static const MapNode ZARQAN_PROVINCE[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Steppe Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Dune Fortress", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Salt Flats", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Oasis Outpost", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Cliffside Fort", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Steppe Garrison",
     .content = RELIC_CHOICE(RELIC_COUNTRY_SEAL, RELIC_WARLORDS_BANNER)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Traveller's Archive",
     .content = PIECE_OLD_KING},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The War Records",
     .content = PIECE_WAR_ELEPHANT},
    {.type = MAP_NODE_OFFERING, .name = "The Wind Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Mirage Outpost",
     .content = EVENT_MIRAGE},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Spy's Tent",
     .content = EVENT_SPY_REPORT},
};

static const MapNode ZARQAN_COUNTRY[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Sultan's Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Citadel Gate", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Inner Fortress", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Throne Hall", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Sultan's Guard",
     .content = RELIC_CHOICE(RELIC_ALCHEMISTS_KIT, RELIC_SURVEYORS_MAP)},
    {.type = MAP_NODE_OFFERING, .name = "The Flame Altar", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Crossing Point",
     .content = EVENT_DESERT_CROSSING},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Ancient Ruin",
     .content = EVENT_ARCHIVE},
    {.type    = MAP_NODE_OVERSEER,
     .name    = "The Hall of Many Faces",
     .content = OVERSEER_MANY_FACED_KING},
};

static const MapNode HARUSHIMA_TOWN[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Harbor District", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Fish Market", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Merchant's Ward", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Eastern Docks", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Bridge Quarter", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Coastal Garrison",
     .content = RELIC_CHOICE(RELIC_DEAD_MANS_PACT, RELIC_GILDED_ARCHIVE)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Dojo of Records",
     .content = PIECE_HONORABLE_HORSE},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Sailor's Almanac",
     .content = PIECE_PROMOTED_BISHOP},
    {.type = MAP_NODE_OFFERING, .name = "The Sea Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Wandering Blade Inn",
     .content = EVENT_RONIN},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Dockmaster's Office",
     .content = EVENT_SPY_NETWORK},
};

static const MapNode HARUSHIMA_PROVINCE[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Rice Fields", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Mountain Pass", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Castle Outskirts", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Inland Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Valley Fort", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Mountain Garrison",
     .content = RELIC_CHOICE(RELIC_BLOODTHIRST, RELIC_VETERANS_BOND)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Castle Archive",
     .content = PIECE_DAIMYO},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Field Commander's Notes",
     .content = PIECE_DRAGON},
    {.type = MAP_NODE_OFFERING, .name = "The Ancestor Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Merchant Ward",
     .content = EVENT_BURNING_PORT},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Smithy Quarter",
     .content = EVENT_FORGE_MASTER},
};

static const MapNode HARUSHIMA_COUNTRY[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Castle Town", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Inner Gate", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Shogun's Garden", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Keep Approach", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Castle Guard",
     .content = RELIC_CHOICE(RELIC_MASTERS_NOTES, RELIC_TRADE_ROUTES)},
    {.type = MAP_NODE_OFFERING, .name = "The Offering Stone", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Lance Regiment's Post",
     .content = EVENT_VETERAN_LANCE},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Emperor's Courier",
     .content = EVENT_DESERTER},
    {.type    = MAP_NODE_OVERSEER,
     .name    = "The Hall of Eternal Recursion",
     .content = OVERSEER_ETERNAL_RECURSION},
};

static const MapNode CAELAN_TOWN[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Market Square", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Guild District", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The East Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Mill Quarter", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Town Gate", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Town Garrison",
     .content = RELIC_CHOICE(RELIC_LIBRARIANS_NOTES, RELIC_MINTED_COIN)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Scribe's Hall",
     .content = PIECE_CHANCELLOR},
    {.type = MAP_NODE_OFFERING, .name = "The Town Chapel", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Tournament Grounds",
     .content = EVENT_TOURNAMENT},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Cathedral",
     .content = EVENT_CHURCH_BLESSING},
};

static const MapNode CAELAN_PROVINCE[] = {
    {.type = MAP_NODE_BATTLE, .name = "The King's Road", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Manor Grounds", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Border Fort", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Old Bridge", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Forest Road", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Border Garrison",
     .content = RELIC_CHOICE(RELIC_SOUL_SHARD, RELIC_IRON_KING)},
    {.type    = MAP_NODE_ARCHIVE,
     .name    = "The Royal Archive",
     .content = PIECE_SOVEREIGN_BANNER},
    {.type = MAP_NODE_OFFERING, .name = "The Wayside Shrine", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Engineer's Workshop",
     .content = EVENT_SIEGE_ENGINEER},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Pretender's Manor",
     .content = EVENT_PRETENDER},
};

static const MapNode CAELAN_COUNTRY[] = {
    {.type = MAP_NODE_BATTLE, .name = "The Capital Gates", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The King's Quarter", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Palace Ward", .content = 0},
    {.type = MAP_NODE_BATTLE, .name = "The Throne Approach", .content = 0},
    {.type    = MAP_NODE_ELITE,
     .name    = "The Royal Guard",
     .content = RELIC_CHOICE(RELIC_INHERITED_POWER, RELIC_WARLORDS_BANNER)},
    {.type = MAP_NODE_OFFERING, .name = "The Royal Altar", .content = 0},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Royal Chamber",
     .content = EVENT_ROYAL_DECREE},
    {.type    = MAP_NODE_EVENT,
     .name    = "The Queen's Garden",
     .content = EVENT_QUEENS_FAVOR},
    {.type    = MAP_NODE_OVERSEER,
     .name    = "The Crowned Throne",
     .content = OVERSEER_CROWNED_HERETIC},
};

static const MapNode* const LAYOUTS[KINGDOM_COUNT][3] = {
    [KINGDOM_LONGWEI]  = {LONGWEI_TOWN, LONGWEI_PROVINCE, LONGWEI_COUNTRY},
    [KINGDOM_KEWARANI] = {KEWARANI_TOWN, KEWARANI_PROVINCE, KEWARANI_COUNTRY},
    [KINGDOM_ZARQAN]   = {ZARQAN_TOWN, ZARQAN_PROVINCE, ZARQAN_COUNTRY},
    [KINGDOM_HARUSHIMA] =
        {HARUSHIMA_TOWN, HARUSHIMA_PROVINCE, HARUSHIMA_COUNTRY},
    [KINGDOM_CAELAN] = {CAELAN_TOWN, CAELAN_PROVINCE, CAELAN_COUNTRY},
};

static const size_t LAYOUT_SIZE[KINGDOM_COUNT][3] = {
    [KINGDOM_LONGWEI] =
        {sizeof(LONGWEI_TOWN) / sizeof(MapNode),
         sizeof(LONGWEI_PROVINCE) / sizeof(MapNode),
         sizeof(LONGWEI_COUNTRY) / sizeof(MapNode)},
    [KINGDOM_KEWARANI] =
        {sizeof(KEWARANI_TOWN) / sizeof(MapNode),
         sizeof(KEWARANI_PROVINCE) / sizeof(MapNode),
         sizeof(KEWARANI_COUNTRY) / sizeof(MapNode)},
    [KINGDOM_ZARQAN] =
        {sizeof(ZARQAN_TOWN) / sizeof(MapNode),
         sizeof(ZARQAN_PROVINCE) / sizeof(MapNode),
         sizeof(ZARQAN_COUNTRY) / sizeof(MapNode)},
    [KINGDOM_HARUSHIMA] =
        {sizeof(HARUSHIMA_TOWN) / sizeof(MapNode),
         sizeof(HARUSHIMA_PROVINCE) / sizeof(MapNode),
         sizeof(HARUSHIMA_COUNTRY) / sizeof(MapNode)},
    [KINGDOM_CAELAN] =
        {sizeof(CAELAN_TOWN) / sizeof(MapNode),
         sizeof(CAELAN_PROVINCE) / sizeof(MapNode),
         sizeof(CAELAN_COUNTRY) / sizeof(MapNode)},
};

static const char* const KINGDOM_NAME[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = "Longwei Empire",
    [KINGDOM_KEWARANI]  = "Kewarani Negusate",
    [KINGDOM_ZARQAN]    = "Zarqan Sultanate",
    [KINGDOM_HARUSHIMA] = "Harushima Shogunate",
    [KINGDOM_CAELAN]    = "Caelan Kingdom",
};

static const PieceID CAPSTONE[KINGDOM_COUNT] = {
    [KINGDOM_LONGWEI]   = PIECE_LIUBO_DIVINER,
    [KINGDOM_KEWARANI]  = PIECE_NEGUS_GUARD,
    [KINGDOM_ZARQAN]    = PIECE_SHAHZADEH,
    [KINGDOM_HARUSHIMA] = PIECE_SHISHI,
    [KINGDOM_CAELAN]    = PIECE_GRYPHON,
};

/*----------------------------------------------------------------------------*\
                              NAVIGATION STATE
\*----------------------------------------------------------------------------*/

static EngineState*       RUN_ENGINE;
static KingdomID          ENTERED_KINGDOM;
static MapNode*           PENDING_NODE;
static const EventOption* PENDING_OPTION;
static RelicID            PENDING_RELICS[2];
static bool               RELIC_OFFER;
static MapNode            VORATH_NODE;
static int                EVENT_REWARD;
static bool               EVENT_REWARD_ACTIVE;

static CardTarget         RUN_TARGETS[MAX_BOARD_SIZE + 1];
static size_t             RUN_TARGET_COUNT;
static CardTarget         RUN_PICKS[MAX_EFFECT_COUNT + 1];
static size_t             RUN_STEP;
static size_t             RUN_STEP_COUNT;

/// rng_mix
///
/// Mixes a seed with a salt into a new deterministic seed using a
/// splitmix-style hash, so every campaign subsystem draws from its own
/// independent stream.
///
/// Params:
/// - seed -> base seed to mix
/// - salt -> stream identifier to mix in
///
/// Return: mixed seed
///
size_t rng_mix(size_t seed, size_t salt) {
    size_t mixed = seed + salt * 0x9E3779B97F4A7C15;

    mixed        = (mixed ^ (mixed >> 30)) * 0xBF58476D1CE4E5B9;
    mixed        = (mixed ^ (mixed >> 27)) * 0x94D049BB133111EB;

    return mixed ^ (mixed >> 31);
}

/*----------------------------------------------------------------------------*\
                               MAP GENERATION
\*----------------------------------------------------------------------------*/

/// map_node
///
/// Returns the MapNode stored at a graph index.
///
/// Params:
/// - map   -> map to look into
/// - index -> node index
///
/// Return: the node's MapNode
///
static MapNode* map_node(MapState* map, size_t index) {
    return map->nodes.nodes[index].data;
}

/// graph_add_edge
///
/// Adds a directed edge if it is not already present, keeping the graph
/// acyclic since it is only ever called with from < to.
///
/// Params:
/// - graph -> graph to edit
/// - from  -> source node
/// - to    -> destination node
///
static void graph_add_edge(DirectedGraph* graph, size_t from, size_t to) {
    for (DGEdge* edge = graph->nodes[from].edges; edge; edge = edge->next) {
        if (edge->to == to) {
            return;
        }
    }

    DGEdge* edge             = malloc(sizeof(DGEdge));

    edge->to                 = to;
    edge->next               = graph->nodes[from].edges;

    graph->nodes[from].edges = edge;
    graph->edges_count++;
}

/// reveal_successors
///
/// Marks every direct successor of a node revealed, so clearing or
/// revealing a node uncovers the content of the nodes it leads to.
///
/// Params:
/// - map   -> map holding the node
/// - index -> node whose successors are revealed
///
static void reveal_successors(MapState* map, size_t index) {
    for (DGEdge* edge = map->nodes.nodes[index].edges; edge;
         edge         = edge->next) {
        map_node(map, edge->to)->revealed = true;
    }
}

/// reveal_from
///
/// Reveals the successors of a node located by identity, for the clear
/// sites that hold a node pointer rather than its map index.
///
/// Params:
/// - node -> cleared or revealing node whose successors uncover
///
static void reveal_from(MapNode* node) {
    MapState* map = node->map;

    if (!map) {
        return;
    }

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        if (map_node(map, i) == node) {
            reveal_successors(map, i);
            return;
        }
    }
}

/// map_generate
///
/// Builds one map: a random DAG patched so every node has an incoming
/// edge from its predecessor, with a heap MapNode per vertex resolved
/// from the layout table and a deterministic modifier on battle nodes.
/// The entry node and its direct successors start revealed; deeper nodes
/// stay hidden until a predecessor is cleared or an event reveals them.
///
/// Params:
/// - map     -> map to populate
/// - kingdom -> owning kingdom
/// - tier    -> map tier
/// - seed    -> per-map deterministic seed
///
static void
map_generate(MapState* map, KingdomID kingdom, MapTypeID tier, size_t seed) {
    size_t count = LAYOUT_SIZE[kingdom][tier];

    map->type    = tier;
    map->nodes   = dag_init();

    dag_rand(&map->nodes, count, count + count / 2, seed);

    for (size_t i = 1; i < count; i++) {
        graph_add_edge(&map->nodes, i - 1, i);
    }

    for (size_t i = 0; i < count; i++) {
        MapNode* node  = malloc(sizeof(MapNode));

        *node          = LAYOUTS[kingdom][tier][i];
        node->map      = map;
        node->kingdom  = map->kingdom;
        node->revealed          = i == 0;
        node->modifier_revealed = false;
        node->cleared           = false;

        if (node->type == MAP_NODE_BATTLE || node->type == MAP_NODE_ELITE ||
            node->type == MAP_NODE_OVERSEER) {
            size_t pick    = rng_mix(seed, i) % MODIFIER_COUNT;

            node->modifier = (BattleModifier*) &MODIFIER_REGISTRY[pick];
        }

        if (node->type == MAP_NODE_BATTLE && rng_mix(seed, i + 100) % 2 == 0) {
            size_t which = (size_t) kingdom * 2 + rng_mix(seed, i + 200) % 2;

            node->trait  = (BoardTrait*) TRAIT_REGISTRY[which];
        }

        map->nodes.nodes[i].data = node;
    }

    reveal_successors(map, 0);
}

/// map_complete
///
/// Reports whether every node of a map has been cleared.
///
/// Params:
/// - map -> map to test
///
/// Return: true when all nodes are cleared
///
static bool map_complete(MapState* map) {
    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        MapNode* node = map_node(map, i);

        if (node->type == MAP_NODE_LIBERATION) {
            continue;
        }

        if (!node->cleared) {
            return false;
        }
    }

    return true;
}

/// active_tier
///
/// Returns the lowest uncleared tier of a kingdom, the tier the player is
/// currently working through.
///
/// Params:
/// - run     -> run holding the kingdom maps
/// - kingdom -> kingdom to inspect
///
/// Return: the active MapTypeID, MAP_COUNTRY when all are cleared
///
static MapTypeID active_tier(RunState* run, KingdomID kingdom) {
    KingdomState* state   = &run->kingdoms[kingdom];
    MapState*     maps[3] = {
        state->town_map,
        state->province_map,
        state->country_map};

    for (int tier = 0; tier < 2; tier++) {
        if (!map_complete(maps[tier])) {
            return (MapTypeID) tier;
        }
    }

    return MAP_COUNTRY;
}

/// active_map
///
/// Returns the map of a kingdom's active tier.
///
/// Params:
/// - run     -> run holding the kingdom maps
/// - kingdom -> kingdom to inspect
///
/// Return: the active MapState
///
static MapState* active_map(RunState* run, KingdomID kingdom) {
    KingdomState* state = &run->kingdoms[kingdom];

    switch (active_tier(run, kingdom)) {
    case MAP_TOWN:
        return state->town_map;
    case MAP_PROVINCE:
        return state->province_map;
    default:
        return state->country_map;
    }
}

/// node_selectable
///
/// Reports whether a node can be selected now: uncleared and either the
/// entry node or a successor of an already cleared node.
///
/// Params:
/// - map   -> map the node belongs to
/// - index -> node index
///
/// Return: true when the node is selectable
///
static bool node_selectable(MapState* map, size_t index) {
    MapNode* self = map_node(map, index);

    if (self->cleared) {
        return false;
    }

    if (self->type == MAP_NODE_LIBERATION) {
        RunState* run = RUN_ENGINE->run;

        return run->battles_fought >= run->liberation_at[self->content];
    }

    if (index == 0) {
        return true;
    }

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        if (!map_node(map, i)->cleared) {
            continue;
        }

        for (DGEdge* edge = map->nodes.nodes[i].edges; edge;
             edge         = edge->next) {
            if (edge->to == index) {
                return true;
            }
        }
    }

    return false;
}

/*----------------------------------------------------------------------------*\
                                   LIFECYCLE
\*----------------------------------------------------------------------------*/

/// unlock_tier
///
/// Applies the idempotent unlock schedule for entering a kingdom's tier:
/// that tier's pieces (town and province only, capstones stay locked
/// until the overseer falls) and cards, plus universal cards of the tier.
///
/// Params:
/// - run     -> run to unlock into
/// - kingdom -> kingdom whose tier is entered
/// - tier    -> map tier entered
///
static void unlock_tier(RunState* run, KingdomID kingdom, MapTypeID tier) {
    UnlockTier unlock = tier == MAP_TOWN       ? TIER_TOWN
                        : tier == MAP_PROVINCE ? TIER_PROVINCE
                                               : TIER_COUNTRY;

    for (PieceID id = 0; id < PIECE_COUNT; id++) {
        const Piece* piece = PIECE_REGISTRY[id];

        if (piece && piece->kingdom == kingdom && piece->tier == unlock &&
            unlock != TIER_COUNTRY) {
            run->pieces[id] = true;
        }
    }

    for (CardID id = 0; id < CARD_COUNT; id++) {
        const Card* card = CARD_REGISTRY[id];

        if (!card) {
            continue;
        }

        if ((card->kingdom == kingdom || card->kingdom == KINGDOM_NONE) &&
            card->tier == unlock) {
            run->cards[id] = true;
        }
    }
}

/// run_new
///
/// Allocates and initializes a fresh run on the engine: seeds, kingdom
/// states, all fifteen campaign maps, starting district unlocks,
/// difficulty, and the optional challenge.
///
/// Params:
/// - engine     -> engine to attach the run to
/// - seed       -> run seed for all deterministic draws
/// - difficulty -> difficulty of the run
/// - challenge  -> challenge modifier, CHALLENGE_NONE for none
///
void run_new(
    EngineState*   engine,
    size_t         seed,
    Difficulty     difficulty,
    ChallengeRunID challenge
) {
    if (engine->run) {
        run_free(engine->run);
    }

    RunState* run   = calloc(1, sizeof(RunState));

    run->seed       = seed ? seed : RNG_SEED;
    run->difficulty = difficulty;
    run->challenge  = challenge;

    for (size_t event = 0; event < EVENT_COUNT; event++) {
        run->event_picks[event] = -1;
    }

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        KingdomState* state = &run->kingdoms[kingdom];

        state->id           = (KingdomID) kingdom;
        state->mastery      = engine->masteries[kingdom];

        MapState** maps[3]  = {
            &state->town_map,
            &state->province_map,
            &state->country_map};

        for (int tier = 0; tier < 3; tier++) {
            *maps[tier]            = calloc(1, sizeof(MapState));
            (*maps[tier])->kingdom = state;

            size_t map_seed        = rng_mix(run->seed, kingdom * 3 + tier);

            map_generate(
                *maps[tier],
                (KingdomID) kingdom,
                (MapTypeID) tier,
                map_seed
            );
        }
    }

    for (PieceID id = 0; id < PIECE_COUNT; id++) {
        const Piece* piece = PIECE_REGISTRY[id];

        if (piece && id != PIECE_KING && piece->tier == TIER_DISTRICT) {
            run->pieces[id] = true;
        }
    }

    for (CardID id = 0; id < CARD_COUNT; id++) {
        const Card* card = CARD_REGISTRY[id];

        if (card && card->tier == TIER_DISTRICT) {
            run->cards[id] = true;
        }
    }

    static const CardID MASTERY_CARDS[KINGDOM_COUNT] = {
        [KINGDOM_LONGWEI]   = CARD_MINGZHUS_SEAL,
        [KINGDOM_KEWARANI]  = CARD_SELASSIES_MARCH,
        [KINGDOM_ZARQAN]    = CARD_TIMURS_CONQUEST,
        [KINGDOM_HARUSHIMA] = CARD_TOMOHITOS_PATIENCE,
        [KINGDOM_CAELAN]    = CARD_ISABELLAS_CORONATION,
    };

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        if (run->kingdoms[kingdom].mastery >= MASTERY_LEVEL_2) {
            run->cards[MASTERY_CARDS[kingdom]] = true;
        }
    }

    engine->run     = run;
    RUN_ENGINE      = engine;
    ENTERED_KINGDOM = KINGDOM_LONGWEI;
    PENDING_NODE    = nullptr;
    RELIC_OFFER     = false;
}

/// run_free
///
/// Frees everything the run owns including all campaign maps and their
/// nodes, then the run itself.
///
/// Params:
/// - run -> run to deallocate
///
void run_free(RunState* run) {
    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        KingdomState* state   = &run->kingdoms[kingdom];
        MapState*     maps[3] = {
            state->town_map,
            state->province_map,
            state->country_map};

        for (int tier = 0; tier < 3; tier++) {
            if (maps[tier]) {
                dag_free(&maps[tier]->nodes);
                free(maps[tier]);
            }
        }
    }

    free(run);
}

/// run_fire_relics
///
/// Generic run-side relic dispatch: walks the run's held relics and invokes
/// every relic effect matching the trigger through the shared invoke-and-log
/// path, the run-side analogue of battle_walk_run. x is the value the trigger
/// computes; relic identities are never named here.
///
/// Params:
/// - engine  -> engine owning the run
/// - trigger -> trigger to match relic effects against
/// - x       -> queried value, type defined per trigger
///
static void
run_fire_relics(EngineState* engine, EffectTrigger trigger, void* x) {
    RunState* run = engine->run;

    for (int id = 0; id < RELIC_COUNT; id++) {
        if (!run->relics[id]) {
            continue;
        }

        const Relic* relic = &RELIC_REGISTRY[id];

        for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
            const Effect* effect = &relic->effects[slot];

            if (!effect->func || effect->trigger != trigger) {
                continue;
            }

            EffectContext ctx =
                effect->context ? *effect->context : (EffectContext){};

            if (effect->func(&ctx, x) && effect->name) {
                protocol_emit(
                    "log effect name=\"%s\" trigger=%s",
                    effect->name,
                    effect_trigger_name(trigger)
                );
            }
        }
    }
}

/// run_enter_map
///
/// Enters the given kingdom's active map, applying the idempotent unlock
/// schedule for every tier up to and including the active one, then fires
/// ON_MAP_ENTER so held run items may pre-reveal map information.
///
/// Params:
/// - engine  -> engine owning the run
/// - kingdom -> kingdom whose active map is entered
///
void run_enter_map(EngineState* engine, KingdomID kingdom) {
    ENTERED_KINGDOM = kingdom;

    MapTypeID tier  = active_tier(engine->run, kingdom);

    for (int t = 0; t <= (int) tier; t++) {
        unlock_tier(engine->run, kingdom, (MapTypeID) t);
    }

    run_fire_relics(engine, ON_MAP_ENTER, engine);
}

/*----------------------------------------------------------------------------*\
                                  EMISSION
\*----------------------------------------------------------------------------*/

/// run_emit_kingdoms
///
/// Emits one line per kingdom for the campaign screen: id, name, mastery
/// level, chain level, and active tier.
///
/// Params:
/// - engine -> engine owning the run
///
void run_emit_kingdoms(EngineState* engine) {
    RunState* run = engine->run;

    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        KingdomState* state = &run->kingdoms[kingdom];

        int           chain =
            state->chain ? (int) (state->chain - CHAIN_REGISTRY) : CHAIN_NONE;

        protocol_emit(
            "kingdom id=%zu name=\"%s\" mastery=%d chain=%d tier=%d",
            kingdom,
            KINGDOM_NAME[kingdom],
            state->mastery,
            chain,
            active_tier(run, (KingdomID) kingdom)
        );
    }
}

/// liberation_inject
///
/// Appends a Liberation Trial node to a host map, linked from the entry node
/// so it becomes selectable once the entry is cleared. The node carries the
/// subjugated kingdom in its content field. Appending at the highest index
/// preserves the graph's low-to-high acyclicity.
///
/// Params:
/// - map        -> host map to grow
/// - subjugated -> kingdom whose figurehead the trial liberates
///
static void liberation_inject(MapState* map, KingdomID subjugated) {
    size_t  index = map->nodes.vertices_count;
    DGNode* grown = realloc(map->nodes.nodes, (index + 1) * sizeof(DGNode));

    if (!grown) {
        return;
    }

    map->nodes.nodes                 = grown;
    map->nodes.nodes[index].edges    = nullptr;

    MapNode* node                    = malloc(sizeof(MapNode));

    *node                            = (MapNode){};
    node->type                       = MAP_NODE_LIBERATION;
    node->name                       = "The Liberation Trial";
    node->content                    = (int) subjugated;
    node->map                        = map;
    node->kingdom                    = map->kingdom;
    node->revealed                   = true;

    map->nodes.nodes[index].data     = node;
    map->nodes.vertices_count        = index + 1;

    graph_add_edge(&map->nodes, 0, index);
}

/// liberation_sync
///
/// Ensures every Subjugated kingdom has one Liberation Trial node on its
/// lore-adjacent kingdom's active map, injecting one when missing. Re-homes
/// a pending trial after the host tier advances, since the active map then
/// differs; a stale node left in a completed tier is never emitted. Derives
/// the trial purely from persisted chain and liberation state, so a reloaded
/// run reconstructs it without saving the node.
///
/// Params:
/// - run -> run whose maps are reconciled
///
static void liberation_sync(RunState* run) {
    for (KingdomID k = 0; k < KINGDOM_COUNT; k++) {
        if (run->kingdoms[k].chain != &CHAIN_REGISTRY[CHAIN_GOLD]) {
            continue;
        }

        MapState* host    = active_map(run, KINGDOM_ADJACENT[k]);
        bool      present = false;

        for (size_t i = 0; i < host->nodes.vertices_count; i++) {
            MapNode* node = map_node(host, i);

            if (node->type == MAP_NODE_LIBERATION && node->content == (int) k) {
                present = true;
                break;
            }
        }

        if (!present) {
            liberation_inject(host, k);
        }
    }
}

/// run_emit_map
///
/// Emits one line per node of the entered kingdom's active map: index,
/// type, name, revealed, cleared, selectable, and modifier index.
///
/// Params:
/// - engine -> engine owning the run
///
void run_emit_map(EngineState* engine) {
    liberation_sync(engine->run);

    MapState* map = active_map(engine->run, ENTERED_KINGDOM);

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        MapNode* node = map_node(map, i);

        int      modifier =
            node->modifier ? (int) (node->modifier - MODIFIER_REGISTRY) : -1;

        protocol_emit(
            "node i=%zu type=%d name=\"%s\" revealed=%d cleared=%d "
            "selectable=%d modifier=%d",
            i,
            node->revealed ? (int) node->type : -1,
            node->revealed ? node->name : "?",
            node->revealed,
            node->cleared,
            node_selectable(map, i),
            node->modifier_revealed ? modifier : -1
        );
    }
}

/*----------------------------------------------------------------------------*\
                                  SELECTION
\*----------------------------------------------------------------------------*/

/// run_select_node
///
/// Selects a reachable node on the entered kingdom's active map and
/// dispatches per type: battle-likes begin a battle, archives reveal a
/// recipe immediately, and offerings and events set up a pending choice
/// resolved on the map screen.
///
/// Params:
/// - engine -> engine owning the run
/// - index  -> node index on the active map
///
/// Return: true when the node was selectable and handled
///
bool run_select_node(EngineState* engine, size_t index) {
    RunState* run = engine->run;

    liberation_sync(run);

    if (run->kingdoms[ENTERED_KINGDOM].chain == &CHAIN_REGISTRY[CHAIN_GOLD]) {
        return false;
    }

    MapState* map = active_map(run, ENTERED_KINGDOM);

    if (index >= map->nodes.vertices_count || !node_selectable(map, index)) {
        return false;
    }

    MapNode* node = map_node(map, index);

    PENDING_NODE  = node;

    switch (node->type) {
    case MAP_NODE_ARCHIVE: {
        int reveal_count = 1;

        run_fire_relics(engine, QUERY_ARCHIVE_REVEAL_COUNT, &reveal_count);

        int revealed = 0;

        if (!run->pieces[node->content]) {
            run->pieces[node->content] = true;

            protocol_emit("log recipe result=%d", node->content);
            revealed++;
        }

        for (PieceID id = 0; id < PIECE_COUNT && revealed < reveal_count;
             id++) {
            if (battle_is_recipe_result(id) && !run->pieces[id]) {
                run->pieces[id] = true;

                protocol_emit("log recipe result=%d", id);
                revealed++;
            }
        }

        node->cleared = true;
        PENDING_NODE  = nullptr;

        reveal_successors(map, index);
        run_emit_map(engine);
        break;
    }

    case MAP_NODE_OFFERING:
        protocol_emit("log offering name=\"%s\"", node->name);
        break;

    case MAP_NODE_EVENT:
        protocol_emit("event id=%d name=\"%s\"", node->content, node->name);
        break;

    default:
        battle_begin(engine, node);
        break;
    }

    return true;
}

/*----------------------------------------------------------------------------*\
                                 RESOLUTION
\*----------------------------------------------------------------------------*/

/// chain_level
///
/// Returns a kingdom's current chain level from its chain pointer.
///
/// Params:
/// - state -> kingdom state to read
///
/// Return: the ChainPenaltyID level, CHAIN_NONE when unchained
///
static ChainPenaltyID chain_level(KingdomState* state) {
    return state->chain ? (ChainPenaltyID) (state->chain - CHAIN_REGISTRY)
                        : CHAIN_NONE;
}

/// chain_add
///
/// Advances a kingdom one chain level toward Gold on a loss.
///
/// Params:
/// - state -> kingdom state to chain
///
static void chain_add(KingdomState* state) {
    ChainPenaltyID level = chain_level(state);

    if (level < CHAIN_GOLD) {
        level = (ChainPenaltyID) (level + 1);
    }

    state->chain        = (ChainPenalty*) &CHAIN_REGISTRY[level];
    state->ever_chained = true;
}

/// chain_remove
///
/// Removes a kingdom's most recent chain level on a win.
///
/// Params:
/// - state -> kingdom state to unchain a step
///
static void chain_remove(KingdomState* state) {
    ChainPenaltyID level = chain_level(state);

    state->chain         = level <= CHAIN_BRONZE
                               ? nullptr
                               : (ChainPenalty*) &CHAIN_REGISTRY[level - 1];
}

/// overseer_reward
///
/// Grants a defeated overseer's rewards: the kingdom's capstone piece, a
/// cleared chain, and the run-wide synergy that also marks the overseer
/// dead for Vorath eligibility.
///
/// Params:
/// - run     -> run to reward
/// - kingdom -> kingdom whose overseer fell
///
static void overseer_reward(RunState* run, KingdomID kingdom) {
    run->pieces[CAPSTONE[kingdom]] = true;
    run->synergies[kingdom]        = true;
    run->kingdoms[kingdom].chain   = nullptr;
}

/// vorath_ready
///
/// Reports whether every overseer has been defeated, enabling Vorath.
///
/// Params:
/// - run -> run to test
///
/// Return: true when all five synergies are held
///
static bool vorath_ready(RunState* run) {
    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        if (!run->synergies[kingdom]) {
            return false;
        }
    }

    return true;
}

/// run_battle_result
///
/// Applies a finished battle to the run: node clearing, chain
/// bookkeeping, the Vorath loss counter, relic offers, overseer rewards,
/// run completion on a Vorath win, and the return to the map or title
/// screen. A battle with no pending node was the synthetic slice and only
/// returns to the title.
///
/// Params:
/// - engine -> engine owning the run
/// - won    -> whether the player won the battle
///
void run_battle_result(EngineState* engine, bool won) {
    MapNode* node = PENDING_NODE;

    PENDING_NODE  = nullptr;

    if (node == &VORATH_NODE) {
        engine_finalize_run(engine, won);
        screen_goto(engine, SCREEN_TITLE);
        return;
    }

    if (!node) {
        screen_goto(engine, SCREEN_TITLE);
        return;
    }

    RunState*     run   = engine->run;
    KingdomID     king  = node->kingdom->id;
    KingdomState* state = &run->kingdoms[king];

    run->battles_fought++;

    if (node->type == MAP_NODE_LIBERATION) {
        KingdomID subjugated = (KingdomID) node->content;

        if (won) {
            node->cleared                   = true;

            reveal_from(node);
            run->kingdoms[subjugated].chain = nullptr;
            run->liberation_at[subjugated]  = 0;
        } else {
            run->liberation_at[subjugated] = run->battles_fought + 3;

            run->vorath_counter++;

            if (run->vorath_counter % 4 == 0) {
                vorath_forbid_recipe(run);
            }
        }
    } else if (won) {
        node->cleared = true;

        reveal_from(node);
        chain_remove(state);

        if (node->type == MAP_NODE_OVERSEER) {
            overseer_reward(run, king);
        }
    } else {
        chain_add(state);

        run->vorath_counter++;

        if (run->vorath_counter % 4 == 0) {
            vorath_forbid_recipe(run);
        }
    }

    screen_goto(engine, SCREEN_MAP);
    run_emit_map(engine);

    if (won && node->type == MAP_NODE_ELITE) {
        int content       = node->content;

        PENDING_RELICS[0] = (RelicID) ((content >> 8) & 0xFF);
        PENDING_RELICS[1] = (RelicID) (content & 0xFF);
        RELIC_OFFER       = true;

        protocol_emit(
            "offer relic_a=%d relic_b=%d",
            PENDING_RELICS[0],
            PENDING_RELICS[1]
        );
    }

    if (EVENT_REWARD_ACTIVE) {
        EVENT_REWARD_ACTIVE = false;

        if (won && EVENT_REWARD == 1) {
            run_remove_chain(run);
        } else if (won) {
            RELIC_OFFER = true;

            protocol_emit(
                "offer relic_a=%d relic_b=%d",
                PENDING_RELICS[0],
                PENDING_RELICS[1]
            );
        }
    }
}

/// run_engine
///
/// Exposes the engine driving the current run so an event's target and
/// resolver effect bodies can enumerate held cards, unlocked pieces,
/// chained figureheads, and map nodes without threading it through x.
///
/// Return: the engine owning the active run
///
EngineState* run_engine(void) {
    return RUN_ENGINE;
}

/// run_pending_picks
///
/// Exposes the TARGET_NONE terminated targets chosen so far for the parked
/// event, so a later step's QUERY_EVENT_TARGETS effect can exclude earlier
/// picks, the run analogue of battle_pending_picks.
///
/// Return: the pending picks list
///
const CardTarget* run_pending_picks(void) {
    return RUN_PICKS;
}

/// run_event_step_count
///
/// Counts an option's QUERY_EVENT_TARGETS slots, the number of target
/// selection steps a targeting event gathers before it resolves.
///
/// Params:
/// - option -> chosen event option
///
/// Return: the number of target steps, zero when the option needs none
///
static size_t run_event_step_count(const EventOption* option) {
    size_t count = 0;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        if (option->effects[slot].func &&
            option->effects[slot].trigger == QUERY_EVENT_TARGETS) {
            count++;
        }
    }

    return count;
}

/// run_build_step
///
/// Fills RUN_TARGETS with the legal targets for one selection step by
/// firing the step-th QUERY_EVENT_TARGETS effect of the pending option.
/// That effect enumerates candidates through run_engine and may read the
/// picks already gathered through run_pending_picks.
///
/// Params:
/// - step -> zero based ordinal among the QUERY_EVENT_TARGETS slots
///
/// Return: the number of legal targets advertised for the step
///
static size_t run_build_step(size_t step) {
    RUN_TARGETS[0].kind = TARGET_NONE;

    size_t ordinal      = 0;

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        const Effect* effect = &PENDING_OPTION->effects[slot];

        if (!effect->func || effect->trigger != QUERY_EVENT_TARGETS) {
            continue;
        }

        if (ordinal++ != step) {
            continue;
        }

        EffectContext ctx =
            effect->context ? *effect->context : (EffectContext){};

        effect->func(&ctx, RUN_TARGETS);
        break;
    }

    return card_target_count(RUN_TARGETS);
}

/// run_emit_targets
///
/// Emits the current RUN_TARGETS list on the protocol so the caller can
/// pick one by index, shared by the choice and each further step.
///
/// Params:
/// - count -> number of live targets in RUN_TARGETS
///
static void run_emit_targets(size_t count) {
    for (size_t i = 0; i < count; i++) {
        protocol_emit(
            "target i=%zu kind=%d value=%d",
            i,
            RUN_TARGETS[i].kind,
            RUN_TARGETS[i].value
        );
    }
}

/// run_fire
///
/// Generic run-side effect dispatch: walks the pending event option's
/// effects and invokes every one matching the trigger through the shared
/// invoke-and-log path, the non-battle analogue of effect_fire. x is the
/// value the trigger computes; run operation metadata lives in the file
/// static pending state, never in x.
///
/// Params:
/// - engine  -> engine owning the run (unused, kept for call symmetry)
/// - trigger -> trigger to match option effects against
/// - x       -> queried value, type defined per trigger
///
void run_fire(EngineState* engine, EffectTrigger trigger, void* x) {
    (void) engine;

    if (!PENDING_OPTION) {
        return;
    }

    for (size_t slot = 0; slot < MAX_EFFECT_COUNT; slot++) {
        const Effect* effect = &PENDING_OPTION->effects[slot];

        if (!effect->func || effect->trigger != trigger) {
            continue;
        }

        EffectContext ctx =
            effect->context ? *effect->context : (EffectContext){};

        if (effect->func(&ctx, x) && effect->name) {
            protocol_emit(
                "log effect name=\"%s\" trigger=%s",
                effect->name,
                effect_trigger_name(trigger)
            );
        }
    }
}

/// run_interaction_advance
///
/// Advances the parked event interaction: builds and emits the next
/// non-empty target step, or when no steps remain resolves the gathered
/// picks through run_fire and completes the node. This is the single
/// completion path for every event that does not suspend into a battle or
/// a relic offer.
///
/// Params:
/// - engine -> engine owning the run
///
static void run_interaction_advance(EngineState* engine) {
    while (RUN_STEP < RUN_STEP_COUNT) {
        RUN_TARGET_COUNT = run_build_step(RUN_STEP);

        if (RUN_TARGET_COUNT > 0) {
            run_emit_targets(RUN_TARGET_COUNT);
            return;
        }

        RUN_PICKS[RUN_STEP].kind     = TARGET_NONE;
        RUN_PICKS[RUN_STEP + 1].kind = TARGET_NONE;
        RUN_STEP++;
    }

    run_fire(engine, ON_EVENT_TARGET_SELECTED, RUN_PICKS);

    MapNode* node = PENDING_NODE;

    if (node) {
        node->cleared = true;

        reveal_from(node);
    }

    PENDING_NODE   = nullptr;
    PENDING_OPTION = nullptr;

    run_emit_map(engine);
}

/// run_event_choose
///
/// Resolves the pending narrative event with the player's choice,
/// recording it, firing its immediate effects through run_fire, then either
/// parking a typed target interaction or completing through the single
/// completion path.
///
/// Params:
/// - engine -> engine owning the run
/// - choice -> choice taken by the player
///
void run_event_choose(EngineState* engine, EventChoice choice) {
    MapNode* node = PENDING_NODE;

    if (!node || node->type != MAP_NODE_EVENT) {
        return;
    }

    RunState* run      = engine->run;
    EventID   event    = (EventID) node->content;

    run->events[event] = choice;
    PENDING_OPTION =
        &EVENT_REGISTRY[event]->options[choice == CHOICE_A ? 0 : 1];

    run_fire(engine, ON_EVENT_CHOOSE, engine);

    protocol_emit("log event id=%d choice=%d", event, choice);

    if (engine->battle || RELIC_OFFER) {
        return;
    }

    RUN_STEP          = 0;
    RUN_STEP_COUNT    = run_event_step_count(PENDING_OPTION);
    RUN_PICKS[0].kind = TARGET_NONE;

    run_interaction_advance(engine);
}

/// run_event_target
///
/// Records one selection for the parked event and advances the interaction,
/// the run analogue of battle_card_target. The protocol selects by index
/// into the list emitted for the current step.
///
/// Params:
/// - engine -> engine owning the run
/// - index  -> index into the current step's advertised target list
///
/// Return: true when the selection was accepted
///
bool run_event_target(EngineState* engine, size_t index) {
    if (!PENDING_OPTION || RUN_STEP >= RUN_STEP_COUNT ||
        index >= RUN_TARGET_COUNT) {
        return false;
    }

    RUN_PICKS[RUN_STEP]          = RUN_TARGETS[index];
    RUN_PICKS[RUN_STEP + 1].kind = TARGET_NONE;
    RUN_STEP++;

    run_interaction_advance(engine);

    return true;
}

/// run_targets_battle_nodes
///
/// Pushes every currently selectable, uncleared battle node of the active
/// map onto the target list as a TARGET_NODE, the candidate set a skip
/// event chooses among.
///
/// Params:
/// - list -> target list to extend
///
void run_targets_battle_nodes(CardTarget* list) {
    MapState* map = active_map(RUN_ENGINE->run, ENTERED_KINGDOM);

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        MapNode* node = map_node(map, i);

        if (node->type == MAP_NODE_BATTLE && !node->cleared &&
            node_selectable(map, i)) {
            card_target_push(list, TARGET_NODE, (int) i);
        }
    }
}

/// run_targets_modifier_nodes
///
/// Pushes every revealed, uncleared battle node whose modifier is still
/// hidden onto the target list as a TARGET_NODE, the battles ahead a
/// modifier-reveal event may scout. Node identity must already be known;
/// the pick only lifts the separate modifier mask.
///
/// Params:
/// - list -> target list to extend
///
void run_targets_modifier_nodes(CardTarget* list) {
    MapState* map = active_map(RUN_ENGINE->run, ENTERED_KINGDOM);

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        MapNode* node = map_node(map, i);

        if (node->type == MAP_NODE_BATTLE && node->modifier &&
            node->revealed && !node->modifier_revealed && !node->cleared) {
            card_target_push(list, TARGET_NODE, (int) i);
        }
    }
}

/// run_targets_figureheads
///
/// Pushes every chained kingdom onto the target list as a
/// TARGET_FIGUREHEAD, the eligible figureheads a chain-removal event
/// chooses among.
///
/// Params:
/// - list -> target list to extend
///
void run_targets_figureheads(CardTarget* list) {
    RunState* run = RUN_ENGINE->run;

    for (size_t k = 0; k < KINGDOM_COUNT; k++) {
        if (run->kingdoms[k].chain) {
            card_target_push(list, TARGET_FIGUREHEAD, (int) k);
        }
    }
}

/// run_event_pick
///
/// Records the chosen target value for the pending event so a run-persistent
/// effect can read it each battle from event_picks.
///
/// Params:
/// - value -> chosen target value to persist
///
void run_event_pick(int value) {
    if (PENDING_NODE) {
        RUN_ENGINE->run->event_picks[PENDING_NODE->content] = value;
    }
}

/// run_skip_node
///
/// Clears the chosen battle node without a battle, revealing its
/// successors, the resolution of a skip event.
///
/// Params:
/// - index -> node index to skip
///
void run_skip_node(size_t index) {
    MapState* map = active_map(RUN_ENGINE->run, ENTERED_KINGDOM);

    if (index >= map->nodes.vertices_count) {
        return;
    }

    map_node(map, index)->cleared = true;

    reveal_successors(map, index);
    protocol_emit("log skip node=%zu", index);
}

/// run_unchain
///
/// Removes one chain level from the chosen figurehead, the resolution of a
/// chain-removal event that targets a specific kingdom.
///
/// Params:
/// - kingdom -> figurehead kingdom to unchain a step
///
void run_unchain(KingdomID kingdom) {
    if (kingdom < KINGDOM_COUNT && RUN_ENGINE->run->kingdoms[kingdom].chain) {
        chain_remove(&RUN_ENGINE->run->kingdoms[kingdom]);
    }
}

/// run_offering
///
/// Removes the chosen card from the run's draw pool at an offering node
/// and clears the node.
///
/// Params:
/// - engine -> engine owning the run
/// - card   -> card to remove from the pool
///
void run_offering(EngineState* engine, CardID card) {
    MapNode* node = PENDING_NODE;

    if (!node || node->type != MAP_NODE_OFFERING) {
        return;
    }

    engine->run->cards[card] = false;

    protocol_emit("log offering removed=%d", card);

    node->cleared = true;
    PENDING_NODE  = nullptr;

    reveal_from(node);
    run_emit_map(engine);
}

/// run_relic_pick
///
/// Grants the chosen relic from a pending elite or overseer offer.
///
/// Params:
/// - engine -> engine owning the run
/// - relic  -> relic chosen by the player
///
void run_relic_pick(EngineState* engine, RelicID relic) {
    if (!RELIC_OFFER) {
        return;
    }

    engine->run->relics[relic] = true;
    RELIC_OFFER                = false;

    protocol_emit("log relic id=%d", relic);

    if (PENDING_NODE && PENDING_NODE->type == MAP_NODE_EVENT) {
        PENDING_NODE->cleared = true;

        reveal_from(PENDING_NODE);
        PENDING_NODE = nullptr;

        run_emit_map(engine);
    }
}

/// run_pressure
///
/// Computes the enemy pressure for battles in the kingdom, scaling the
/// free pieces the enemy starts with as the run progresses. Full scaling
/// arrives with the run-scale systems phase.
///
/// Params:
/// - run     -> run to measure
/// - kingdom -> kingdom the battle takes place in
///
/// Return: pressure level for the kingdom
///
/// run_innate_ready
///
/// Reports whether the human has unlocked a kingdom's innate: from its
/// Province map normally, or from Town once that kingdom reaches mastery
/// one.
///
/// Params:
/// - run     -> run to inspect
/// - kingdom -> kingdom whose innate is queried
///
/// Return: true when the innate is active for the human
///
bool run_innate_ready(RunState* run, KingdomID kingdom) {
    if (run->kingdoms[kingdom].mastery >= MASTERY_LEVEL_1) {
        return true;
    }

    return map_complete(run->kingdoms[kingdom].town_map);
}

/// run_reduce_vorath
///
/// Lowers the Global Vorath Counter, clamped at zero.
///
/// Params:
/// - run    -> run whose counter drops
/// - amount -> reduction amount
///
void run_reduce_vorath(RunState* run, size_t amount) {
    run->vorath_counter =
        run->vorath_counter > amount ? run->vorath_counter - amount : 0;
}

/// run_node_reveal
///
/// Reveals the next count still-hidden nodes of the entered kingdom's
/// active map in graph index order, logging each. Reveal events fold their
/// peek through this so the masked identity and content become visible.
///
/// Params:
/// - engine -> engine owning the run
/// - count  -> number of hidden nodes to reveal
///
void run_node_reveal(EngineState* engine, size_t count) {
    MapState* map = active_map(engine->run, ENTERED_KINGDOM);

    for (size_t i = 0; i < map->nodes.vertices_count && count; i++) {
        MapNode* node = map_node(map, i);

        if (node->revealed) {
            continue;
        }

        node->revealed = true;
        count--;

        protocol_emit("log reveal node=%zu", i);
    }
}

/// run_reveal_modifier
///
/// Reveals a single node's battle modifier without touching its node-identity
/// visibility, logging the peek. This is the modifier-only analogue of
/// run_node_reveal, the resolution of a modifier-reveal event or item.
///
/// Params:
/// - engine -> engine owning the run
/// - index  -> node index whose modifier becomes visible
///
void run_reveal_modifier(EngineState* engine, size_t index) {
    MapState* map = active_map(engine->run, ENTERED_KINGDOM);

    if (index >= map->nodes.vertices_count) {
        return;
    }

    map_node(map, index)->modifier_revealed = true;

    protocol_emit("log reveal modifier=%zu", index);
}

/// run_reveal_map_modifier
///
/// Reveals one deterministic battle node's modifier on the entered kingdom's
/// active map. The choice folds the same per-map seed used to generate the
/// map, so re-entering reveals the same node's modifier and never another.
///
/// Params:
/// - engine -> engine owning the run
///
void run_reveal_map_modifier(EngineState* engine) {
    RunState* run     = engine->run;
    KingdomID kingdom = ENTERED_KINGDOM;
    MapTypeID tier    = active_tier(run, kingdom);
    MapState* map     = active_map(run, kingdom);

    size_t    count   = 0;

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        MapNode* node = map_node(map, i);

        if (node->type == MAP_NODE_BATTLE && node->modifier) {
            count++;
        }
    }

    if (!count) {
        return;
    }

    size_t target =
        rng_mix(run->seed, (size_t) kingdom * 3 + (size_t) tier) % count;
    size_t seen   = 0;

    for (size_t i = 0; i < map->nodes.vertices_count; i++) {
        MapNode* node = map_node(map, i);

        if (node->type != MAP_NODE_BATTLE || !node->modifier) {
            continue;
        }

        if (seen == target) {
            run_reveal_modifier(engine, i);
            return;
        }

        seen++;
    }
}

/// run_remove_chain
///
/// Removes one accumulated chain penalty from the first chained figurehead.
///
/// Params:
/// - run -> run holding the figureheads
///
void run_remove_chain(RunState* run) {
    for (size_t kingdom = 0; kingdom < KINGDOM_COUNT; kingdom++) {
        if (run->kingdoms[kingdom].chain) {
            chain_remove(&run->kingdoms[kingdom]);
            return;
        }
    }
}

/// run_begin_elite
///
/// Begins an elite battle on the pending event node, stashing the reward a
/// win grants: a two-relic offer or a chain removal.
///
/// Params:
/// - engine -> engine owning the run
/// - reward -> 0 for a relic offer, 1 for a chain removal
/// - a      -> first offered relic when the reward is a relic offer
/// - b      -> second offered relic when the reward is a relic offer
///
void run_begin_elite(EngineState* engine, int reward, RelicID a, RelicID b) {
    if (!PENDING_NODE) {
        return;
    }

    EVENT_REWARD        = reward;
    EVENT_REWARD_ACTIVE = true;
    PENDING_RELICS[0]   = a;
    PENDING_RELICS[1]   = b;

    battle_begin(engine, PENDING_NODE);
}

size_t run_pressure(RunState* run, KingdomID kingdom) {
    size_t pressure = 0;

    for (size_t other = 0; other < KINGDOM_COUNT; other++) {
        if (other == (size_t) kingdom) {
            continue;
        }

        KingdomState* state   = &run->kingdoms[other];
        MapState*     maps[3] = {
            state->town_map,
            state->province_map,
            state->country_map,
        };

        for (size_t tier = 0; tier < 3; tier++) {
            if (maps[tier] && map_complete(maps[tier])) {
                pressure++;
            }
        }
    }

    return pressure;
}

/// run_enter_vorath
///
/// Starts the final Vorath battle when every overseer has fallen. The
/// bespoke Vorath board and army arrive with the overseers phase; this
/// wires the eligibility gate and the finalize-on-result path.
///
/// Params:
/// - engine -> engine owning the run
///
/// Return: true when Vorath was entered
///
bool run_enter_vorath(EngineState* engine) {
    if (!vorath_ready(engine->run)) {
        return false;
    }

    VORATH_NODE.type = MAP_NODE_OVERSEER;
    PENDING_NODE     = &VORATH_NODE;

    battle_begin(engine, &VORATH_NODE);

    return true;
}
