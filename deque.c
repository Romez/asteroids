#include <stdlib.h>
#include <assert.h>
#include "deque.h"

Deque* init_deque() {
    Node* sentinel_first = malloc(sizeof(Node));
    assert(sentinel_first != NULL && "Can't allocate sentinel first");

    Node* sentinel_last = malloc(sizeof(Node));
    assert(sentinel_last != NULL && "Can't allocate sentinel last");

    sentinel_first->next = sentinel_last;
    sentinel_first->val = NULL;

    sentinel_last->prev = sentinel_first;
    sentinel_last->val = NULL;

    Deque* dq = malloc(sizeof(Deque));
    assert(dq != NULL && "Can't allocate dq");

    dq->first = sentinel_first;
    dq->last = sentinel_last;

    return dq;
}

void prepend_node(Deque* dq, void* val) {
    Node* n = malloc(sizeof(Node));
    assert(n != NULL && "Can't allocate sentinel first");

    Node* first = dq->first;

    n->next =  first->next;
    n->prev = first;
    n->val = val;

    Node* t = first->next;

    first->next = n;
    t->prev = n;

    dq->count++;
}

void remove_node(Deque* dq, Node* node) {
    free(node->val);
    Node* prev_node = node->prev;
    Node* next_node = node->next;

    prev_node->next = next_node;
    next_node->prev = prev_node;

    free(node);

    dq->count--;
}
