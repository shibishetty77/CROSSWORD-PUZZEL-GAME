// main.c (updated portability + safe parsing)
#include "crossword.h"
#include "leaderboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* portability: ensure SIZE_MAX and strcasecmp are available */
#include <limits.h>
/* map _stricmp only for MSVC (Visual Studio). On GCC/MinGW prefer POSIX strcasecmp. */
#if defined(_MSC_VER)
  #ifndef _CRT_SECURE_NO_WARNINGS
  #define _CRT_SECURE_NO_WARNINGS
  #endif
  #include <string.h>
  #define strcasecmp _stricmp
#else
  #include <strings.h> /* for strcasecmp on POSIX/GCC/MinGW */
#endif

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#ifdef _WIN32
#include <windows.h>
static void enable_utf8_console(void) { SetConsoleOutputCP(65001); }
#else
static void enable_utf8_console(void) { (void)0; }
#endif

/* ---------------- Puzzle bank ---------------- */

typedef struct {
    const char *text;
    size_t row, col;
    Direction dir;
    const char *clue;
} Entry;

typedef struct {
    const char *title;
    size_t rows, cols;
    size_t count;
    Entry entries[16];
} Puzzle;

/* (same puzzles you used; edit if desired) */
static const Puzzle PUZZLES[] = {
    { .title = "Programming Basics", .rows = 10, .cols = 10, .count = 8,
      .entries = {
        {"CODE",0,0,DIR_ACROSS,"Write-and-compile activity"},
        {"ARRAY",1,0,DIR_ACROSS,"Indexed collection"},
        {"DEBUG",2,0,DIR_ACROSS,"Find and fix errors"},
        {"LOOP",3,0,DIR_ACROSS,"Repeated execution structure"},
        {"INPUT",4,0,DIR_ACROSS,"What a program reads"},
        {"OUTPUT",5,0,DIR_ACROSS,"What a program produces"},
        {"STACK",6,0,DIR_ACROSS,"LIFO data structure"},
        {"QUEUE",7,0,DIR_ACROSS,"FIFO data structure"}
      }
    },
    { .title = "Languages & Tools", .rows = 10, .cols = 10, .count = 8,
      .entries = {
        {"PYTHON",0,0,DIR_ACROSS,"Snake and a language"},
        {"JAVA",1,0,DIR_ACROSS,"Coffee-named language"},
        {"RUST",2,0,DIR_ACROSS,"Systems language focused on safety"},
        {"GIT",3,0,DIR_ACROSS,"Version control tool"},
        {"DOCKER",4,0,DIR_ACROSS,"Container platform"},
        {"NPM",5,0,DIR_ACROSS,"Node package manager"},
        {"REACT",6,0,DIR_ACROSS,"UI library by Meta"},
        {"BASH",7,0,DIR_ACROSS,"Shell for scripting"}
      }
    },
    { .title = "Computer Science Concepts", .rows = 10, .cols = 10, .count = 8,
      .entries = {
        {"ALGORITHM",0,0,DIR_ACROSS,"Step-by-step procedure"},
        {"COMPLEXITY",1,0,DIR_ACROSS,"Big-O topic"},
        {"GRAPH",2,0,DIR_ACROSS,"Non-linear structure of nodes"},
        {"TREE",3,0,DIR_ACROSS,"Acyclic connected graph"},
        {"HEAP",4,0,DIR_ACROSS,"Priority-based structure"},
        {"HASH",5,0,DIR_ACROSS,"Maps keys to indices"},
        {"CACHE",6,0,DIR_ACROSS,"Fast memory for recent data"},
        {"THREAD",7,0,DIR_ACROSS,"Unit of CPU execution"}
      }
    },
    { .title = "General Knowledge", .rows = 10, .cols = 10, .count = 8,
      .entries = {
        {"EARTH",0,0,DIR_ACROSS,"Our home planet"},
        {"OCEAN",1,0,DIR_ACROSS,"Vast body of salt water"},
        {"RIVER",2,0,DIR_ACROSS,"Flows to the sea"},
        {"DESERT",3,0,DIR_ACROSS,"Dry, sandy region"},
        {"MOUNTAIN",4,0,DIR_ACROSS,"Tall natural elevation"},
        {"FOREST",5,0,DIR_ACROSS,"Many trees together"},
        {"ISLAND",6,0,DIR_ACROSS,"Land surrounded by water"},
        {"VALLEY",7,0,DIR_ACROSS,"Low area between hills"}
      }
    },
    { .title = "Animals & Nature", .rows = 10, .cols = 10, .count = 8,
      .entries = {
        {"TIGER",0,0,DIR_ACROSS,"Striped big cat"},
        {"EAGLE",1,0,DIR_ACROSS,"Majestic bird of prey"},
        {"WHALE",2,0,DIR_ACROSS,"Largest ocean mammal"},
        {"PANDA",3,0,DIR_ACROSS,"Bamboo-loving bear"},
        {"ZEBRA",4,0,DIR_ACROSS,"Black-and-white stripes"},
        {"HORSE",5,0,DIR_ACROSS,"Ridden by cowboys"},
        {"CAMEL",6,0,DIR_ACROSS,"Desert ship"},
        {"OTTER",7,0,DIR_ACROSS,"Playful river mammal"}
      }
    }
};
static const size_t PUZZLE_COUNT = sizeof(PUZZLES) / sizeof(PUZZLES[0]);

/* ---------------- Helpers & UI ---------------- */

static void print_banner(const char *title, size_t number) {
    printf("\n=== Crossword %zu: %s ===\n", number + 1, title);
    puts("Type 'help' to see commands.\n");
}

static void print_help(void) {
    puts("\nCommands:");
    puts("  clues               - list all clues with IDs");
    puts("  guess <id> <WORD>   - answer a clue by its ID");
    puts("  show                - show grid (solved letters only)");
    puts("  reveal              - reveal full crossword (answers visible)");
    puts("  graph               - show connectivity (NO answers shown)");
    puts("  next / skip         - load a new random puzzle");
    puts("  progress            - show solved %");
    puts("  leaderboard [N|all] - show top N entries or 'all' (default 10)");
    puts("  lb [N|all]          - shorthand for leaderboard");
    puts("  submit <name>       - submit current progress to leaderboard");
    puts("  help                - show this help");
    puts("  quit                - exit\n");
}

static void load_puzzle_into(Crossword *cw, const Puzzle *pz) {
    for (size_t i = 0; i < pz->count; ++i) {
        const Entry *e = &pz->entries[i];
        (void)add_word(cw, e->text, e->row, e->col, e->dir, e->clue);
    }
}

static Crossword *make_crossword_from_index(size_t index) {
    const Puzzle *pz = &PUZZLES[index % PUZZLE_COUNT];
    Crossword *cw = create_crossword(pz->rows, pz->cols);
    load_puzzle_into(cw, pz);
    return cw;
}

static size_t random_index_except(size_t except, size_t count) {
    if (count <= 1) return 0;
    size_t r;
    do { r = (size_t)(rand() % (int)count); } while (r == except);
    return r;
}

static void display_crossword_partial(const Crossword *cw) {
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
    putchar('\n');
    for (size_t r = 0; r < cw->rows; ++r) {
        fputs("    ", stdout);
        for (size_t c = 0; c < cw->cols; ++c) fputs("┌───", stdout);
        fputs("┐\n    ", stdout);

        for (size_t c = 0; c < cw->cols; ++c) {
            char gridc = cw->cells[r][c];
            if (gridc == '.') { fputs("│███", stdout); continue; }

            char show = ' ';
            for (size_t i = 0; i < cw->word_count && show == ' '; ++i) {
                const Word *w = &cw->words[i];
                if (!w->solved) continue;
                size_t len = strlen(w->text);
                for (size_t k = 0; k < len; ++k) {
                    size_t rr = w->row + (w->dir == DIR_DOWN ? k : 0);
                    size_t cc = w->col + (w->dir == DIR_ACROSS ? k : 0);
                    if (rr == r && cc == c) { show = gridc; break; }
                }
            }
            printf("│ %c ", show);
        }
        fputs("│\n", stdout);
    }
    fputs("    ", stdout);
    for (size_t c = 0; c < cw->cols; ++c) fputs("└───", stdout);
    fputs("┘\n", stdout);
}

/* parse optional argument after leaderboard command */
static size_t parse_lb_arg(const char *arg, size_t default_n) {
    if (!arg) return default_n;
    while (*arg && isspace((unsigned char)*arg)) ++arg;
    if (*arg == '\0') return default_n;
    if (strcasecmp(arg, "all") == 0) return SIZE_MAX;
    char *endptr = NULL;
    long v = strtol(arg, &endptr, 10);
    if (endptr == arg || v <= 0) return default_n;
    return (size_t)v;
}

/* helper: parse "guess <id> <word>" without relying on %zu */
static int parse_guess_command(const char *line, size_t *out_id, char *out_word, size_t out_word_len) {
    char buf[512];
    if (strlen(line) >= sizeof(buf)) return 0;
    strcpy(buf, line);

    char *tok = strtok(buf, " \t"); /* skip "guess" */
    if (!tok) return 0;
    tok = strtok(NULL, " \t"); /* id */
    if (!tok) return 0;
    char *endptr = NULL;
    unsigned long idv = strtoul(tok, &endptr, 10);
    if (endptr == tok) return 0;

    tok = strtok(NULL, " \t"); /* word */
    if (!tok) return 0;
    strncpy(out_word, tok, out_word_len - 1);
    out_word[out_word_len - 1] = '\0';
    *out_id = (size_t)idv;
    return 1;
}

/* ---------------- Main ---------------- */

int main(void) {
    enable_utf8_console();
    srand((unsigned)time(NULL));
    lb_init();

    size_t current = (size_t)(rand() % (int)PUZZLE_COUNT);
    Crossword *cw = make_crossword_from_index(current);
    time_t puzzle_started = time(NULL);

    print_banner(PUZZLES[current].title, current);
    display_crossword_partial(cw);
    list_clues(cw, true);
    display_progress(cw);

    char line[512];
    char word[128];
    size_t id;

    for (;;) {
        printf("\n> ");
        if (!fgets(line, sizeof line, stdin)) break;
        size_t L = strlen(line);
        if (L && line[L-1] == '\n') line[L-1] = '\0';

        if (strcmp(line, "quit") == 0) break;
        if (strcmp(line, "help") == 0) { print_help(); continue; }
        if (strcmp(line, "show") == 0) { display_crossword_partial(cw); continue; }
        if (strcmp(line, "reveal") == 0) { show_solution(cw); continue; }
        if (strcmp(line, "clues") == 0) { list_clues(cw, true); continue; }
        if (strcmp(line, "progress") == 0) { display_progress(cw); continue; }

        if (strncmp(line, "leaderboard", 11) == 0 || strncmp(line, "lb", 2) == 0) {
            const char *arg = NULL;
            if (strncmp(line, "leaderboard", 11) == 0) arg = line + 11;
            else arg = line + 2;
            size_t n = parse_lb_arg(arg, 10);
            if (n == SIZE_MAX) lb_print_all();
            else lb_print_top(n);
            continue;
        }

        if (strncmp(line, "submit ", 7) == 0) {
            char name[LB_NAME_LEN];
            if (sscanf(line + 7, "%31s", name) >= 1) {
                int solved = 0;
                for (size_t i = 0; i < cw->word_count; ++i) if (cw->words[i].solved) ++solved;
                time_t now = time(NULL);
                int tsec = (int)difftime(now, puzzle_started);
                if (lb_add(name, solved, tsec)) printf("Submitted to leaderboard.\n");
                else printf("Could not submit (leaderboard full or not improved).\n");
            } else {
                printf("Usage: submit <name>\n");
            }
            continue;
        }

        if (strcmp(line, "graph") == 0) {
            Graph *g = build_crossword_graph(cw);
            printf("\nGraph connectivity: %s\n",
                   is_crossword_connected(g) ? "✅ All words connected" : "⚠️ Not fully connected");
            for (size_t i = 0; i < g->num_vertices; ++i) {
                size_t deg = 0;
                for (GraphNode *n = g->adj[i]; n; n = n->next) ++deg;
                printf("Node %zu: degree=%zu neighbors:", i, deg);
                for (GraphNode *n = g->adj[i]; n; n = n->next) printf(" %zu", n->index);
                printf("\n");
            }
            free_graph(g);
            continue;
        }

        if (strcmp(line, "next") == 0 || strcmp(line, "skip") == 0) {
            destroy_crossword(cw);
            size_t prev = current;
            current = random_index_except(prev, PUZZLE_COUNT);
            cw = make_crossword_from_index(current);
            puzzle_started = time(NULL);
            print_banner(PUZZLES[current].title, current);
            display_crossword_partial(cw);
            list_clues(cw, true);
            display_progress(cw);
            continue;
        }

        if (strncmp(line, "guess", 5) == 0) {
            if (!parse_guess_command(line, &id, word, sizeof word)) {
                printf("Usage: guess <id> <WORD>\n");
                continue;
            }
            if (id >= cw->word_count) {
                printf("Invalid clue id. Use 'clues' to see IDs.\n");
                continue;
            }
            if (make_guess(cw, word, id)) {
                printf("✅ Correct! Revealed \"%s\" on the grid.\n", cw->words[id].text);
            } else {
                printf("❌ Incorrect. Keep trying.\n");
            }
            display_crossword_partial(cw);
            display_progress(cw);

            if (is_puzzle_complete(cw)) {
                puts("\n🎉 Puzzle complete!");
                show_solution(cw);

                time_t finished = time(NULL);
                int tsec = (int)difftime(finished, puzzle_started);
                int solved = (int)cw->word_count;
                char name[LB_NAME_LEN] = "";
                printf("Enter your name to record on leaderboard (or press Enter to skip): ");
                if (fgets(name, sizeof name, stdin)) {
                    size_t ln = strlen(name);
                    if (ln && name[ln-1] == '\n') name[ln-1] = '\0';
                    if (name[0] != '\0') {
                        if (lb_add(name, solved, tsec)) {
                            printf("Recorded on leaderboard. Congratulations, %s!\n", name);
                        } else {
                            printf("Could not record on leaderboard.\n");
                        }
                    } else {
                        puts("Skipped recording.");
                    }
                }

                destroy_crossword(cw);
                size_t prev = current;
                current = random_index_except(prev, PUZZLE_COUNT);
                cw = make_crossword_from_index(current);
                puzzle_started = time(NULL);
                print_banner(PUZZLES[current].title, current);
                display_crossword_partial(cw);
                list_clues(cw, true);
                display_progress(cw);
            }
            continue;
        }

        puts("Unrecognized command. Type 'help' for options.");
    }

    lb_shutdown();
    destroy_crossword(cw);
    return 0;
}
