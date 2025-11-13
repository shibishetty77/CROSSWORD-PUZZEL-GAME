// leaderboard.c (robust CSV parsing, no sscanf warnings)
#include "leaderboard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static LBEntry lb_store[LB_MAX_ENTRIES];
static size_t lb_used = 0;

/* comparator: return negative if A should appear before B (A better) */
static int lb_cmp(const void *pa, const void *pb) {
    const LBEntry *A = (const LBEntry*)pa;
    const LBEntry *B = (const LBEntry*)pb;
    if (A->solved != B->solved) return (int)(B->solved - A->solved); /* more solved first */
    if (A->time_sec != B->time_sec) return (A->time_sec - B->time_sec); /* less time first */
    if (A->when < B->when) return -1;
    if (A->when > B->when) return 1;
    return 0;
}

void lb_init(void) {
    lb_used = 0;
    FILE *f = fopen(LB_FILE, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f) && lb_used < LB_MAX_ENTRIES) {
        /* CSV format: name,solved,time_sec,when */
        /* we'll parse by tokens using strtok to avoid scanf format issues */
        char *p = line;
        /* trim newline */
        size_t L = strlen(p);
        if (L && p[L-1] == '\n') p[L-1] = '\0';

        char *name_tok = strtok(p, ",");
        if (!name_tok) continue;
        char *solved_tok = strtok(NULL, ",");
        char *time_tok = strtok(NULL, ",");
        char *when_tok = strtok(NULL, ",");
        if (!solved_tok || !time_tok || !when_tok) continue;

        /* parse numbers safely */
        char *endptr = NULL;
        long solved = strtol(solved_tok, &endptr, 10);
        if (endptr == solved_tok) continue;
        long time_sec = strtol(time_tok, &endptr, 10);
        if (endptr == time_tok) continue;
        long long when_ll = strtoll(when_tok, &endptr, 10);
        if (endptr == when_tok) continue;

        LBEntry e;
        strncpy(e.name, name_tok, LB_NAME_LEN - 1);
        e.name[LB_NAME_LEN - 1] = '\0';
        e.solved = (int)solved;
        e.time_sec = (int)time_sec;
        e.when = (time_t)when_ll;
        lb_store[lb_used++] = e;
    }
    fclose(f);
    if (lb_used) qsort(lb_store, lb_used, sizeof(LBEntry), lb_cmp);
}

static void lb_save(void) {
    FILE *f = fopen(LB_FILE, "w");
    if (!f) return;
    for (size_t i = 0; i < lb_used; ++i) {
        fprintf(f, "%s,%d,%d,%lld\n",
                lb_store[i].name,
                lb_store[i].solved,
                lb_store[i].time_sec,
                (long long)lb_store[i].when);
    }
    fclose(f);
}

void lb_shutdown(void) {
    lb_save();
}

bool lb_add(const char *name, int solved, int time_sec) {
    if (!name || name[0] == '\0') return false;
    LBEntry newe;
    strncpy(newe.name, name, LB_NAME_LEN - 1); newe.name[LB_NAME_LEN - 1] = '\0';
    newe.solved = solved; newe.time_sec = time_sec; newe.when = time(NULL);

    if (lb_used < LB_MAX_ENTRIES) {
        lb_store[lb_used++] = newe;
        qsort(lb_store, lb_used, sizeof(LBEntry), lb_cmp);
        lb_save();
        return true;
    }
    /* leaderboard full: compare with worst (last item after sort) */
    if (lb_cmp(&newe, &lb_store[lb_used - 1]) < 0) {
        lb_store[lb_used - 1] = newe;
        qsort(lb_store, lb_used, sizeof(LBEntry), lb_cmp);
        lb_save();
        return true;
    }
    return false;
}

size_t lb_count(void) { return lb_used; }
const LBEntry *lb_get_entries(void) { return lb_store; }

void lb_print_top(size_t n) {
    if (n == 0) return;
    if (n > lb_used) n = lb_used;
    printf("\n--- Leaderboard (top %zu) ---\n", n);
    printf("%-3s %-20s %-6s %-8s %-20s\n", "#", "NAME", "SOLVED", "TIME(s)", "DATE");
    for (size_t i = 0; i < n; ++i) {
        char tbuf[32] = "unknown";
        struct tm tmp;
        struct tm *ptm = localtime(&lb_store[i].when);
        if (ptm) { tmp = *ptm; strftime(tbuf, sizeof tbuf, "%Y-%m-%d", &tmp); }
        printf("%-3zu %-20s %-6d %-8d %-20s\n",
               i + 1, lb_store[i].name, lb_store[i].solved, lb_store[i].time_sec, tbuf);
    }
}

void lb_print_all(void) {
    lb_print_top(lb_used);
}
