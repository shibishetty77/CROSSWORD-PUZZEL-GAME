#ifndef CROSSWORD_H
#define CROSSWORD_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_WORD_LEN   32
#define MAX_CLUE_LEN   128
#define MAX_GRID       12
#define MAX_WORDS      32

typedef enum { DIR_ACROSS = 0, DIR_DOWN = 1 } Direction;

typedef struct {
    char text[MAX_WORD_LEN];
    char clue[MAX_CLUE_LEN];
    size_t row, col;       // start position
    Direction dir;
    bool placed;
    bool solved;
} Word;

typedef struct {
    size_t rows, cols;
    char   cells[MAX_GRID][MAX_GRID]; // '.' for block/empty, 'A'..'Z' for letters
    size_t word_count;
    Word   words[MAX_WORDS];
} Crossword;

/* Crossword management */
Crossword *create_crossword(size_t rows, size_t cols);
void destroy_crossword(Crossword *cw);

/* Crossword operations */
bool add_word(Crossword *cw, const char *text, size_t row, size_t col, Direction dir, const char *clue);
void display_crossword(const Crossword *cw, bool reveal);
void display_progress(const Crossword *cw);
void list_clues(const Crossword *cw, bool show_status);
void show_solution(const Crossword *cw);
bool make_guess(Crossword *cw, const char *guess, size_t word_index);
bool is_puzzle_complete(const Crossword *cw);

/* ================= GRAPH STRUCTURES ================= */

typedef struct GraphNode {
    size_t index;               // index of word
    struct GraphNode *next;     // adjacency list link
} GraphNode;

typedef struct {
    size_t num_vertices;
    GraphNode *adj[MAX_WORDS];
} Graph;

Graph *build_crossword_graph(const Crossword *cw);
void free_graph(Graph *g);
void dfs_graph(const Graph *g, size_t start, bool visited[]);
bool is_crossword_connected(const Graph *g);

#endif
