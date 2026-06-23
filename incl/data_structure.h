//! data_structure.h
//!
//! This file contains all the core data structure declarations used
//! throughout the game. It defines the linked list for managing effects
//! and the directed acyclic graph for representing campaign maps.
//!
//! Created: 12/06/2026
//! Author : Alden Luthfi

/*----------------------------------------------------------------------------*\
                                LINKED_LIST.C
\*----------------------------------------------------------------------------*/

/// LLNode
///
/// A node in a doubly-linked list. Contains a void pointer for storing
/// arbitrary data and pointers to the previous and next nodes.
///
struct LLNode {
    void*   data;
    LLNode* next;
    LLNode* prev;
};

/// LinkedList
///
/// A doubly-linked list with O(1) push to tail and removal operations.
/// Maintains head and tail pointers along with a size counter.
///
struct LinkedList {
    LLNode* head;
    LLNode* tail;
    size_t  size;
};

LinkedList ll_init(void);
void       ll_push(LinkedList* list, void* data);
void       ll_poll(LinkedList* list, LLNode* index);
void       ll_free(LinkedList* list);

/*----------------------------------------------------------------------------*\
                              DIRECTED_GRAPH.C
\*----------------------------------------------------------------------------*/

/// DGEdge
///
/// An edge in a directed graph pointing from one node to another.
///
struct DGEdge {
    size_t  to;
    DGEdge* next;
};

/// DGNode
///
/// A node in a directed graph containing arbitrary data and a list of edges.
///
struct DGNode {
    void*   data;
    DGEdge* edges;
};

/// DirectedGraph
///
/// A directed acyclic graph represented as an adjacency list. All edges
/// point from lower-indexed nodes to higher-indexed nodes ensuring acyclicity.
///
struct DirectedGraph {
    DGNode* nodes;
    size_t  vertices_count;
    size_t  edges_count;
};

DirectedGraph dag_init(void);
void          dag_rand(DirectedGraph* graph, size_t vertices, size_t edges);
void          dag_free(DirectedGraph* graph);
