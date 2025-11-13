#include "crossword.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static inline char up(char c) { return (char)toupper((unsigned char)c); }

Crossword *create_crossword(size_t rows, size_t cols) {
    if (rows > MAX_GRID) rows = MAX_GRID;
    if (cols > MAX_GRID) cols = MAX_GRID;
    Crossword *cw = (Crossword*)calloc(1, sizeof(Crossword));
    if (!cw) return NULL;
    cw->rows = rows;
    cw->cols = cols;
    for (size_t r = 0; r < rows; ++r)
        for (size_t c = 0; c < cols; ++c)
            cw->cells[r][c] = '.';
    cw->word_count = 0;
    return cw;
}

void destroy_crossword(Crossword *cw) {
    free(cw);
}

static bool fits_and_matches(const Crossword *cw, const char *text,
                             size_t row, size_t col, Direction dir)
{
    size_t len = strlen(text);
    if (dir == DIR_ACROSS && col + len > cw->cols) return false;
    if (dir == DIR_DOWN && row + len > cw->rows) return false;

    for (size_t i = 0; i < len; ++i) {
        size_t rr = row + (dir == DIR_DOWN ? i : 0);
        size_t cc = col + (dir == DIR_ACROSS ? i : 0);
        char gridc = cw->cells[rr][cc];
        char want = up(text[i]);
        if (gridc != '.' && gridc != want) return false;
    }
    return true;
}

static void place_word(Crossword *cw, const char *text,
                       size_t row, size_t col, Direction dir)
{
    size_t len = strlen(text);
    for (size_t i = 0; i < len; ++i) {
        size_t rr = row + (dir == DIR_DOWN ? i : 0);
        size_t cc = col + (dir == DIR_ACROSS ? i : 0);
        cw->cells[rr][cc] = up(text[i]);
    }
}

bool add_word(Crossword *cw, const char *text, size_t row, size_t col, Direction dir, const char *clue) {
    if (!cw || !text || !text[0] || !clue) return false;
    if (cw->word_count >= MAX_WORDS) return false;
    char U[MAX_WORD_LEN];
    size_t n = 0;
    for (; text[n] && n < MAX_WORD_LEN - 1; ++n) U[n] = up(text[n]);
    U[n] = '\0';
    if (n == 0) return false;
    if (!fits_and_matches(cw, U, row, col, dir)) return false;
    place_word(cw, U, row, col, dir);
    Word *w = &cw->words[cw->word_count++];
    strncpy(w->text, U, MAX_WORD_LEN-1); w->text[MAX_WORD_LEN-1] = '\0';
    strncpy(w->clue, clue, MAX_CLUE_LEN-1); w->clue[MAX_CLUE_LEN-1] = '\0';
    w->row = row; w->col = col; w->dir = dir;
    w->placed = true; w->solved = false;
    return true;
}

void display_crossword(const Crossword *cw, bool reveal) {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    putchar('\n');
    for (size_t r = 0; r < cw->rows; ++r) {
        fputs("    ", stdout);
        for (size_t c = 0; c < cw->cols; ++c) fputs("┌───", stdout);
        fputs("┐\n    ", stdout);
        for (size_t c = 0; c < cw->cols; ++c) {
            char ch = cw->cells[r][c];
            if (ch == '.') { fputs("│███", stdout); continue; }
            printf("│ %c ", reveal ? ch : ' ');
        }
        fputs("│\n", stdout);
    }
    fputs("    ", stdout);
    for (size_t c = 0; c < cw->cols; ++c) fputs("└───", stdout);
    fputs("┘\n", stdout);
}

void display_progress(const Crossword *cw) {
    size_t solved = 0;
    for (size_t i = 0; i < cw->word_count; ++i) if (cw->words[i].solved) ++solved;
    double pct = cw->word_count ? (100.0 * solved / (double)cw->word_count) : 100.0;
    printf("Progress: %zu/%zu solved (%.1f%%)\n", solved, cw->word_count, pct);
}

void list_clues(const Crossword *cw, bool show_status) {
    puts("\nAcross:");
    for (size_t i = 0; i < cw->word_count; ++i) {
        const Word *w = &cw->words[i];
        if (w->dir != DIR_ACROSS) continue;
        printf("  %2zu) (%zu,%zu) %s%s\n", i, w->row, w->col, w->clue,
               show_status ? (w->solved ? "  [solved]" : "") : "");
    }
    puts("Down:");
    for (size_t i = 0; i < cw->word_count; ++i) {
        const Word *w = &cw->words[i];
        if (w->dir != DIR_DOWN) continue;
        printf("  %2zu) (%zu,%zu) %s%s\n", i, w->row, w->col, w->clue,
               show_status ? (w->solved ? "  [solved]" : "") : "");
    }
}

void show_solution(const Crossword *cw) {
    puts("\nSOLUTION:");
    display_crossword(cw, true);
    for (size_t i = 0; i < cw->word_count; ++i) {
        const Word *w = &cw->words[i];
        printf("  %2zu) %s (%s) at (%zu,%zu)\n",
               i, w->text, w->dir == DIR_ACROSS ? "ACROSS" : "DOWN", w->row, w->col);
    }
}

bool make_guess(Crossword *cw, const char *guess, size_t word_index) {
    if (!cw || word_index >= cw->word_count || !guess) return false;
    char upg[MAX_WORD_LEN];
    size_t n = 0;
    for (; guess[n] && n < MAX_WORD_LEN - 1; ++n) upg[n] = up(guess[n]);
    upg[n] = '\0';
    if (strcmp(upg, cw->words[word_index].text) == 0) {
        cw->words[word_index].solved = true;
        return true;
    }
    return false;
}

bool is_puzzle_complete(const Crossword *cw) {
    for (size_t i = 0; i < cw->word_count; ++i)
        if (!cw->words[i].solved) return false;
    return true;
}
