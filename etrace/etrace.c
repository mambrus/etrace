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

/* Cleanup before exit even if exit-with-error */
void etrace_exit(int status)
{
    int rc;

    assert_ext((rc = mlist_close(etrace.event_list)
               ) == 0);
    assert_ext((rc = mlist_close(etrace.pid_trigger_list)
               ) == 0);

    exit(status);
}

int main(int argc, char **argv)
{
    int rc, cnt;

    LOGI("etrace version v%s \n", VERSION);

    assert_ext((rc =
                mlist_opencreate(sizeof(struct event), NULL, &etrace.event_list)
               ) == 0);
    assert_ext((rc =
                mlist_opencreate(sizeof(struct pid_trigger), NULL,
                                 &etrace.pid_trigger_list)
               ) == 0);

    ASSURE_E((rc = opts_parse(argc, argv, &opts)) > 0, goto err);
    LOGI("Parsed %d options.\n", rc);
    ASSURE_E(opts_check(&opts) == OPT_OK, goto err);
    LOGI("Option passed rule-check OK\n", rc);

    log_test();

    assert_np(time_now(&etrace.stime));
    sprintf(etrace.outfname, "%d_%06d_%06d_%d.etrace", etrace.opts->rid,
            (int)etrace.stime.tv_sec, (int)etrace.stime.tv_usec,
            etrace.opts->pid);
    LOGD("Out-file name: %s", etrace.outfname);

    /* Diagnostic print-out of events */
    for (mlist_head(etrace.event_list), cnt = 0;
         mlist_curr(etrace.event_list); mlist_next(etrace.event_list), cnt++) {

        struct event *e;
        assert(n->pl);
        e = mdata_curr(etrace.event_list);
        LOGI("Event #%d:\n", cnt);
        if (strlen(e->name) == 0)
            LOGE("  %s\n", e->name);
        else
            LOGI("  %s\n", e->name);
        LOGI("  %s\n", e->filter);
    }

    LOGD("Adding root-PID to list: %d\n", etrace.opts->pid);
    ASSURE_E(mlist_add_last(etrace.pid_trigger_list, &(struct pid_trigger) {
                            etrace.opts->pid}), goto err);
    if (opts.threads) {
        ASSURE_E(tid_tolist
                 (etrace.opts->pid, etrace.pid_trigger_list), goto err);
    }

    tid_expand_events(etrace.pid_trigger_list, etrace.event_list);

    /* Diagnostic print-out of pid_triggers */
    for (mlist_head(etrace.pid_trigger_list), cnt = 0;
         mlist_curr(etrace.pid_trigger_list);
         mlist_next(etrace.pid_trigger_list), cnt++) {

        struct pid_trigger *pd;
        assert(n->pl);
        pd = mdata_curr(etrace.pid_trigger_list);
        LOGI("PID #%d :\n", pd->pid);
    }

    etrace_exit(0);
err:
    etrace_exit(1);
    /*Shut up gcc */
    return 0;
}
