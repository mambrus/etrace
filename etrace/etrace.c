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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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

#define CPY_MAX 4096

/* This binds when global variable initialization is run in .start, i.e.
* before CTOR */
log_level log_filter_level = DEF_LOG_LEVEL;

struct opts opts = {
/* *INDENT-OFF* */
    .loglevel       = &log_filter_level,
    .ptime          = DEF_PTIME,
    .ftrace_clock   = DEF_FTRACE_CLOCK,
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
    .opts           = &opts,
    .pid            = -1,
    .out_fd         = 1
/* *INDENT-ON* */
};

static void free_content_event_list(handle_t handle)
{
    int cnt;

    for (mlist_head(handle), cnt = 0;
         mlist_curr(handle); mlist_next(handle), cnt++) {

        struct event *e = mdata_curr(handle);
        free(e->filter);
    }
    LOGD("Freed %d dynamic elements from event_list\n", cnt);
}

static void free_content_pid_trigger_list(handle_t handle)
{
    int cnt;

/* Closing any efilter list (no dynamic elements) */
    for (mlist_head(etrace.pid_trigger_list), cnt = 0;
         mlist_curr(etrace.pid_trigger_list);
         mlist_next(etrace.pid_trigger_list), cnt++) {

        struct pid_trigger *pd = mdata_curr(etrace.pid_trigger_list);
        assert_ext(mlist_close(pd->efilter_list) == 0);
    }
    LOGD("Freed %d efilter_list from pid_trigger_list\n", cnt);
}

/* Cleanup before exit even if exit-with-error */
void etrace_exit(int status)
{
    LOGD("etrace_exit initiated\n");

    free_content_event_list(etrace.event_list);
    free_content_pid_trigger_list(etrace.pid_trigger_list);

    assert_ext(mlist_close(etrace.event_list) == 0);
    assert_ext(mlist_close(etrace.pid_trigger_list) == 0);
    free(opts.req_opts);

    exit(status);
}

int main(int argc, char **argv)
{
    int rc, cnt;
    char *tfname;

    tfname = malloc(PATH_MAX);

    LOGI("etrace version v%s \n", VERSION);

    assert_ext((rc =
                mlist_opencreate(sizeof(struct event), NULL, &etrace.event_list)
               ) == 0);
    assert_ext((rc =
                mlist_opencreate(sizeof(struct pid_trigger), NULL,
                                 &etrace.pid_trigger_list)
               ) == 0);

    opts_init();
    memset(opts.outfname, 0, PATH_MAX);
    memset(etrace.outfname, 0, PATH_MAX);
    ASSURE_E((rc = opts_parse(argc, argv, &opts)) > 0, goto err);
    LOGI("Parsed %d options.\n", rc);
    ASSURE_E(opts_check(&opts) == OPT_OK, goto err);
    LOGI("Option passed rule-check OK\n", rc);

    etrace.pid = opts.pid;
    assert_np(time_now(&etrace.stime));

    if (strlen(opts.outfname)) {
        /* Out to file, but deduct the filename */

        sprintf(etrace.outfname, "%d_%06d_%06d_%d.etrace", etrace.opts->rid,
                (int)etrace.stime.tv_sec, (int)etrace.stime.tv_usec,
                etrace.opts->pid);
        snprintf(tfname, PATH_MAX, "%s/%s", opts.workdir, etrace.outfname);
    } else {
        if (!(strcmp(opts.outfname, "-"))) {
            if (opts.outfname[0] == '/') {
                strncpy(etrace.outfname, opts.outfname, PATH_MAX);
            } else {
                snprintf(tfname, PATH_MAX, "%s/%s", opts.workdir,
                         etrace.outfname);
            }
        }
    }
    if ((strlen(etrace.outfname) == 0) && (!(strcmp(etrace.outfname, "-")))) {
        LOGD("Out-file name: %s\n", tfname);
        ASSURE_E((etrace.out_fd = open(tfname, O_WRONLY)) != -1, goto open_err);
    } else {
        LOGD("Writing output to stdout\n");
    }
    free(tfname);

    snprintf(etrace.tracefs_path, PATH_MAX, "%s/tracing", opts.debugfs_path);

    /* Make sure tracing is stopped */
    ASSURE_E(write_by_name("0", "%s/tracing_on", etrace.tracefs_path),
             goto err);
    /* Make sure nop is really chosen */
    ASSURE_E(write_by_name("nop", "%s/current_tracer", etrace.tracefs_path),
             goto err);

    /* Set the ftrace-clock, but only if it has actually been given on
     * command-line */
    if (req_opt('c', opts.req_opts)->cnt > 0) {
        LOGD("Setting new ftrace clock to: %s\n", opts.ftrace_clock);

        ASSURE_E(write_by_name
                 (opts.ftrace_clock, "%s/tracing/trace_clock",
                  opts.debugfs_path), goto err);
    }

    /* Clear trace buffer */
    /* Low level I/O doesn't work, why? Find out (TBD) */
    //ASSURE_E(write_by_name("0", "%s/trace", etrace.tracefs_path), goto err);

    /*Use high level form as work-around */
    {
        FILE *f;
        ASSURE_E((f =
                  fopen("/sys/kernel/debug/tracing/trace", "w")) != NULL,
                 goto err);

        fprintf(f, "");
    }

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
        ASSURE_E(proc_tid_tolist
                 (etrace.opts->pid, etrace.pid_trigger_list), goto err);
    }

    ASSURE_E(proc_expand_events(etrace.pid_trigger_list, etrace.event_list),
             goto err);

    ASSURE_E(proc_pheader(&etrace), goto err);

    /* Diagnostic print-out of pid_triggers */
    LOGD("List of pid_triggers {#n PID,name,filter}:\n");
    for (mlist_head(etrace.pid_trigger_list), cnt = 0;
         mlist_curr(etrace.pid_trigger_list);
         mlist_next(etrace.pid_trigger_list)) {

        struct pid_trigger *pd = mdata_curr(etrace.pid_trigger_list);
        for (mlist_head(pd->efilter_list);
             mlist_curr(pd->efilter_list);
             mlist_next(pd->efilter_list), cnt++) {

            struct efilter *ef = mdata_curr(pd->efilter_list);
            LOGD("  #%d %d,%s,%s\n", cnt, pd->pid, ef->event->name,
                 ef->efilter);
        }
    }

    ASSURE_E(proc_concat_epieces(etrace.event_list, etrace.pid_trigger_list),
             goto err);

    /* Informative print-out of final event with filters  */
    LOGV("List of pid_triggers {event::filter}:\n");
    for (mlist_head(etrace.event_list), cnt = 0;
         mlist_curr(etrace.event_list); mlist_next(etrace.event_list), cnt++) {

        struct event *ev = mdata_curr(etrace.event_list);
        LOGV("%s::%s\n", ev->name, ev->filter);
    }
    ASSURE_E(proc_ftrace_arm(etrace.event_list), goto err);

    LOGI("Tracing starts\n");
    ASSURE_E(write_by_name("1", "%s/tracing_on", etrace.tracefs_path),
             goto err);
//    usleep(opts.ptime);
//    LOGI("Tracing stops\n");
//    ASSURE_E(write_by_name("0", "%s/tracing_on", etrace.tracefs_path),
//             goto err);

    int fd_in;
    char tofname[PATH_MAX];
    snprintf(tofname, PATH_MAX, "%s/trace_pipe", etrace.tracefs_path);
    LOGD("Accessing file: %s\n", tofname);
    ASSURE_E((fd_in = open(tofname, O_RDONLY)) != -1, goto open_err);

//#ifdef NEVER
    while (1) {
        /* Copy trace buffer to output */
        char cpy_buf[CPY_MAX];
        //char tofname[PATH_MAX];
        int /*fd_in, */ rc, done = 0;

#ifdef NEVER
        snprintf(tofname, PATH_MAX, "%s/trace", etrace.tracefs_path);
        LOGD("Accessing file: %s\n", tofname);
        ASSURE_E((fd_in = open(tofname, O_RDONLY)) != -1, goto open_err);
#endif

        /* Copy trace buffer to output */
        while (!done) {
            rc = read(fd_in, cpy_buf, CPY_MAX);
            if (rc > 0) {
                rc = write(etrace.out_fd, cpy_buf, rc);
            } else {
                if (rc == EAGAIN || rc == EINTR) {
                    LOGW("EAGAIN or rc==EINTR happened. Restarting read.\n");
                } else {
                    ASSURE_E(rc == 0, goto io_err);
                    done = 1;
                }
            }
        }
        usleep(opts.ptime);
    }
//#endif //NEVER

    etrace_exit(0);

io_err:
    if (etrace.out_fd > 2) {
        close(etrace.out_fd);
    }
open_err:
    LOGE("File operation failed with errno: %d \"%s\"\n", errno,
         strerror(errno));
err:
    etrace_exit(1);
    /* GCC, please shut up! */
    return 0;
}
