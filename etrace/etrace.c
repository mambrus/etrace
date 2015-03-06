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

#include <stdio.h>
#include <config.h>
#include <sys/time.h>

//#undef NDEBUG
#define NDEBUG
#include <assert.h>
#include "assure.h"
#include <log.h>
#include <mtime.h>
#include "opts.h"
#include "etrace.h"
#include "proc.h"
#include <mlist.h>

/* This binds when global variable initialization is run in .start, i.e.
 * before CTOR */
log_level log_filter_level = DEF_LOG_LEVEL;

struct opts opts = {
/* *INDENT-OFF* */
    .loglevel       = &log_filter_level,
    .ptime          = DEF_PTIME,
    .debugfs_path   = DEF_DEBUGFS_PATH,
    .workdir        = DEF_WORKDIR,
    .threads        = 0,
    .pid            = 0,
    .rid            = 0,
    .daemon         = 0
/* *INDENT-ON* */
};

struct etrace etrace = {
/* *INDENT-OFF* */
    .opts             = &opts,
/* *INDENT-ON* */
};

int main(int argc, char **argv)
{
    int rc, cnt;
    struct node *n;

    assert_ext((rc =
                mlist_opencreate(sizeof(struct event), NULL, &etrace.event_list)
               ) == 0);
    assert_ext((rc =
                mlist_opencreate(sizeof(struct pid_trigger), NULL,
                                 &etrace.pid_trigger_list)
               ) == 0);

    ASSURE_E((rc = opts_parse(argc, argv, &opts)) > 0, goto done);
    LOGI("Parsed %d options.\n", rc);
    ASSURE_E(opts_check(&opts) == OPT_OK, goto done);
    LOGI("Option passed rule-check OK\n", rc);

    printf("Hello world: v%s \n", VERSION);
    //log_set_verbosity(LOG_LEVEL_VERBOSE);
    LOGV("Hello [VERBOSE] %d\n", LOG_LEVEL_VERBOSE);
    LOGD("Hello [DEBUG] %d\n", LOG_LEVEL_DEBUG);
    LOGI("Hello [INFO] %d\n", LOG_LEVEL_INFO);
    LOGW("Hello [WARNING] %d\n", LOG_LEVEL_WARNING);
    LOGE("Hello [ERROR] %d\n", LOG_LEVEL_ERROR);
    //LOGC("Hello [CRITICAL] %d\n", LOG_LEVEL_CRITICAL);

    assert_np(time_now(&etrace.stime));
    sprintf(etrace.outfname, "%d_%06d_%06d_%d.etrace", etrace.opts->rid,
            (int)etrace.stime.tv_sec, (int)etrace.stime.tv_usec,
            etrace.opts->pid);
    LOGI("Out-file name: %s", etrace.outfname);

    /* Diagnostic print-out of events */
    for (n = mlist_head(etrace.event_list), cnt = 0;
         n; n = mlist_next(etrace.event_list)
        ) {
        struct event *e;
        assert(n->pl);
        e = mlist_curr(etrace.event_list);
        LOGI("Event #%d:\n", cnt++);
        if (strlen(e->name) == 0)
            LOGE("  %s\n", e->name);
        else
            LOGI("  %s\n", e->name);
        LOGI("  %s\n", e->filter);
    }

    if (opts.threads) {
        ASSURE_E(tid_populate
                 (etrace.opts->pid, etrace.pid_trigger_list,
                  etrace.pid_trigger_list), goto done);
    }

done:

    assert_ext((rc = mlist_close(etrace.event_list)
               ) == 0);
    assert_ext((rc = mlist_close(etrace.pid_trigger_list)
               ) == 0);

    return 0;
}
