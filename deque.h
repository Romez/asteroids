typedef struct Node {
    struct Node* prev;
    struct Node* next;
    void* val;
} Node;

typedef struct {
    Node* first;
    Node* last;
    int count;
} Deque;

void prepend_node(Deque *dq, void *val);

Deque* init_deque();

void remove_node(Deque *dq, Node *node);
