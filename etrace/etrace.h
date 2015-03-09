/***************************************************************************
 *   Copyright (C) 2015 by Michael Ambrus                                  *
 *   michael.ambrus@sonymobile.com                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef etrace_h
#define etrace_h

#include <mlist.h>

#include <limits.h>
#define FILTER_MAX 255
#define FINAL_FILTER_MAX 4096

/* Root event definition */
struct event {
    int id;                     /* Unique incremental ID starting from 0 */
    char name[PATH_MAX];        /* Filename relative to tracefs root */
    char *filter;               /* Filter expression. Note that this can be
                                   abstract. In case several threads are to
                                   be followed the string %tid% has special
                                   meaning and will be translated into the
                                   numerical PID for each thread. */
};

/* Event expression part expanded to be associated to a corresponding
 * threads PID */
struct efilter {
    struct event *event;        /* Pointer to the event serving as template */
    char efilter[FILTER_MAX];
};

/* PID trigger
 * The process-id, with list of expanded event-filters
 */
struct pid_trigger {
    pid_t pid;
    handle_t efilter_list;
};

struct etrace {
    struct opts *opts;
    struct timeval stime;
    char outfname[PATH_MAX];
    handle_t pid_trigger_list;
    handle_t event_list;
};

void etrace_exit(int status);

#endif                          /* etrace_h */
