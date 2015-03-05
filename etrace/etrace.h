#ifndef etrace_h
#define etrace_h

#define LDATA struct event
#include <mlist.h>

#include <limits.h>
#define FILTER_MAX 255

struct event {
    char name[PATH_MAX];
    char filter[FILTER_MAX];
};

struct etrace {
    struct opts *opts;
    struct timeval stime;
    char outfname[PATH_MAX];
    handle_t pid_list;
    handle_t event_list;
};

#endif                          /* etrace_h */
