#include "linked_list.h"
#include <stdlib.h>
#include <ctype.h>

Node *ll_from_cstr_upper(const char *s) {
    if (!s) return NULL;
    Node *head = NULL, *tail = NULL;
    for (size_t i = 0; s[i]; ++i) {
        Node *n = (Node*)malloc(sizeof(Node));
        if (!n) { ll_free(head); return NULL; }
        n->value = (char)toupper((unsigned char)s[i]);
        n->next = NULL;
        if (!head) head = n;
        else tail->next = n;
        tail = n;
    }
    return head;
}

void ll_free(Node *head) {
    while (head) {
        Node *n = head->next;
        free(head);
        head = n;
    }
}

size_t ll_len(const Node *head) {
    size_t n = 0;
    for (; head; head = head->next) ++n;
    return n;
}
