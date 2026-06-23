//! linked_list.c
//!
//! Doubly-linked list implementation for managing collections of effects.
//! Each node holds a void pointer allowing storage of arbitrary data types.
//! The list maintains head/tail pointers and a size counter for O(1) access.
//!
//! Created: 23/06/2026
//! Author : Alden Luthfi

#include <forward.h>
#include <prelude.h>

/// ll_init
///
/// Initializes an empty linked list with null head and tail pointers
/// and zero size. The returned list is safe to pass to ll_free.
///
LinkedList ll_init() {
    LinkedList list;
    list.head = nullptr;
    list.tail = nullptr;
    list.size = 0;
    return list;
}

/// ll_push
///
/// Appends a new node containing the given data pointer to the end of the
/// list. The list takes ownership of data -- it must be heap-allocated and
/// must not be freed by the caller. The list grows by one node and the tail
/// pointer is updated.
///
/// Params:
/// - list -> pointer to list to append to
/// - data -> heap-allocated data to transfer ownership of
///
void ll_push(LinkedList* list, void* data) {
    LLNode* node = malloc(sizeof(LLNode));

    if (!node) {
        return;
    }

    node->data = data;
    node->next = nullptr;
    node->prev = list->tail;

    if (list->tail) {
        list->tail->next = node;
    }

    list->tail = node;

    if (!list->head) {
        list->head = node;
    }

    list->size++;
}

/// ll_poll
///
/// Removes the specified node from the list and frees both the node and its
/// data (calls free on node->data). Handles head/tail updates automatically.
/// If node is null, does nothing. The caller must not use node or its data
/// after this call.
///
/// Params:
/// - list -> pointer to list containing the node
/// - node -> node to remove and free
///
void ll_poll(LinkedList* list, LLNode* node) {
    if (!node) {
        return;
    }

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->tail = node->prev;
    }

    free(node->data);
    free(node);

    list->size--;
}

/// ll_free
///
/// Frees all nodes in the list along with their data (calls free on each
/// node->data). After this call the list is empty with null head/tail and
/// zero size. The caller must not use any previously held node pointers or
/// their data after this call.
///
/// Params:
/// - list -> pointer to list to deallocate
///
void ll_free(LinkedList* list) {
    LLNode* node = list->head;

    while (node) {
        LLNode* next = node->next;

        free(node->data);
        free(node);

        node = next;
    }

    list->head = nullptr;
    list->tail = nullptr;
    list->size = 0;
}
