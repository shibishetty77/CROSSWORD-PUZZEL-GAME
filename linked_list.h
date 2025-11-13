#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

typedef struct Node {
    char value;
    struct Node *next;
} Node;

Node *ll_from_cstr_upper(const char *s);
void ll_free(Node *head);
size_t ll_len(const Node *head);

#endif
