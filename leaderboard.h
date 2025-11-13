#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <time.h>
#include <stddef.h>
#include <stdbool.h>

#define LB_NAME_LEN 32
#define LB_FILE "leaderboard.csv"
#define LB_MAX_ENTRIES 128

typedef struct {
    char name[LB_NAME_LEN];
    int solved;         /* number of words solved */
    int time_sec;       /* time taken in seconds */
    time_t when;        /* timestamp */
} LBEntry;

/* lifecycle */
void lb_init(void);
void lb_shutdown(void);

/* operations */
bool lb_add(const char *name, int solved, int time_sec);
size_t lb_count(void);
const LBEntry *lb_get_entries(void);

/* utilities */
void lb_print_top(size_t n);
void lb_print_all(void);

#endif
