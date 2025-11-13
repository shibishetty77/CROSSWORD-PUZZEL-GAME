#include "crossword.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Returns true if words a and b overlap on at least one grid cell */
static bool words_intersect(const Crossword *cw, const Word *a, const Word *b) {
    size_t lenA = strlen(a->text);
    size_t lenB = strlen(b->text);
    for (size_t i = 0; i < lenA; ++i) {
        size_t ra = a->row + (a->dir == DIR_DOWN ? i : 0);
        size_t ca = a->col + (a->dir == DIR_ACROSS ? i : 0);
        for (size_t j = 0; j < lenB; ++j) {
            size_t rb = b->row + (b->dir == DIR_DOWN ? j : 0);
            size_t cb = b->col + (b->dir == DIR_ACROSS ? j : 0);
            if (ra == rb && ca == cb && cw->cells[ra][ca] != '.') return true;
        }
    }
    return false;
}

static GraphNode *new_node(size_t idx) {
    GraphNode *n = (GraphNode*)malloc(sizeof(GraphNode));
    if (!n) return NULL;
    n->index = idx; n->next = NULL;
    return n;
}

Graph *build_crossword_graph(const Crossword *cw) {
    Graph *g = (Graph*)calloc(1, sizeof(Graph));
    if (!g) return NULL;
    g->num_vertices = cw->word_count;
    for (size_t i = 0; i < cw->word_count; ++i) g->adj[i] = NULL;

    for (size_t i = 0; i < cw->word_count; ++i) {
        for (size_t j = i + 1; j < cw->word_count; ++j) {
            if (words_intersect(cw, &cw->words[i], &cw->words[j])) {
                GraphNode *n1 = new_node(j);
                if (n1) { n1->next = g->adj[i]; g->adj[i] = n1; }
                GraphNode *n2 = new_node(i);
                if (n2) { n2->next = g->adj[j]; g->adj[j] = n2; }
            }
        }
    }
    return g;
}

void free_graph(Graph *g) {
    if (!g) return;
    for (size_t i = 0; i < g->num_vertices; ++i) {
        GraphNode *cur = g->adj[i];
        while (cur) {
            GraphNode *tmp = cur->next;
            free(cur);
            cur = tmp;
        }
    }
    free(g);
}

void dfs_graph(const Graph *g, size_t start, bool visited[]) {
    if (!g || start >= g->num_vertices) return;
    visited[start] = true;
    for (GraphNode *n = g->adj[start]; n; n = n->next) {
        if (!visited[n->index]) dfs_graph(g, n->index, visited);
    }
}

bool is_crossword_connected(const Graph *g) {
    if (!g) return true;
    if (g->num_vertices == 0) return true;
    bool visited[MAX_WORDS] = {0};
    dfs_graph(g, 0, visited);
    for (size_t i = 0; i < g->num_vertices; ++i) if (!visited[i]) return false;
    return true;
}
