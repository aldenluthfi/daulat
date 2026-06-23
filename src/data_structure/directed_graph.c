//! directed_graph.c
//!
//! Directed acyclic graph implementation for campaign map representation.
//! Each node contains arbitrary data and edges point from lower-indexed
//! nodes to higher-indexed nodes, ensuring acyclicity by construction.
//!
//! Created: 23/06/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// dag_init
///
/// Initializes an empty directed graph with zero nodes and zero edges.
/// The returned graph is safe to pass to dag_free regardless of state.
///
DirectedGraph dag_init() {
    DirectedGraph graph;
    graph.nodes          = nullptr;
    graph.vertices_count = 0;
    graph.edges_count    = 0;

    return graph;
}

/// dag_rand
///
/// Generates a random directed acyclic graph with the specified number of
/// vertices and edges. Edges always point from lower-indexed nodes to
/// higher-indexed nodes, guaranteeing no cycles. Uses the global RNG_SEED
/// for deterministic generation. assumes e < v * (v - 1) / 2.
///
/// Params:
/// - graph    -> pointer to graph to populate
/// - vertices -> number of nodes to create
/// - edges    -> number of edges to create
///
void dag_rand(DirectedGraph* graph, size_t vertices, size_t edges) {
    graph->nodes          = calloc(vertices, sizeof(DGNode));
    graph->vertices_count = vertices;
    graph->edges_count    = 0;

    unsigned int seed     = RNG_SEED;

    while (graph->edges_count < edges) {
        size_t to = rand_r(&seed) % vertices;

        if (to == 0) {
            continue;
        }

        size_t  from = rand_r(&seed) % to;

        DGEdge* prev = nullptr;
        DGEdge* curr = graph->nodes[from].edges;

        while (curr && curr->to < to) {
            prev = curr;
            curr = curr->next;
        }

        if (curr && curr->to == to) {
            continue;
        }

        DGEdge* edge = malloc(sizeof(DGEdge));
        edge->to     = to;
        edge->next   = curr;

        if (prev) {
            prev->next = edge;
        } else {
            graph->nodes[from].edges = edge;
        }

        graph->edges_count++;
    }
}

/// dag_free
///
/// Deallocates all memory associated with the graph including all edges
/// and node data. After calling this, the graph is in the same state as
/// one returned by dag_init.
///
/// Params:
/// - graph -> pointer to graph to deallocate
///
void dag_free(DirectedGraph* graph) {
    for (size_t i = 0; i < graph->vertices_count; i++) {
        DGEdge* edge = graph->nodes[i].edges;

        while (edge) {
            DGEdge* next = edge->next;
            free(edge);
            edge = next;
        }

        free(graph->nodes[i].data);
    }

    free(graph->nodes);

    graph->nodes          = nullptr;
    graph->vertices_count = 0;
    graph->edges_count    = 0;
}
