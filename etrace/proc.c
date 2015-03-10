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

/* -Functions that do something with the system (rather loosely speaking)- */

#include <stdio.h>
#include <stdlib.h>
#include <config.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <regex.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#include "assure.h"
#include <log.h>
#include <mtime.h>
#include "opts.h"
#include "proc.h"
#include <mlist.h>
#include "etrace.h"

#define TID_REGEXP "%tid%"
#define MAX_DGTS 16             /*Max number of digits in a PID */
#define MARK_POS 1              /*Marker position of where etrace internal data stops */

static inline int expand_each_event(pid_t, handle_t, handle_t);
static inline int rreplace(char *buf, int size, regex_t *re, char *rp);

extern struct etrace etrace;

/* Populate list with thread process id:s. On linux kernel V3+ this is very
easy: https://lkml.org/lkml/2011/12/28/59
*/
int proc_tid_tolist(pid_t pid, handle_t pid_trigger_list)
{
    DIR *dp;
    struct dirent *ep;
    char dirname[PATH_MAX];
    int cnt;
    struct pid_trigger pd;

    snprintf(dirname, PATH_MAX, "/proc/%d/task", pid);
    LOGI("Getting threads PID:s from %s", dirname);

    dp = opendir(dirname);
    if (dp != NULL) {
        while ((ep = readdir(dp))) {
            if (ep->d_name[0] >= '0' && ep->d_name[0] <= '9') {
                LOGD("Adding thread-PID to list: %s\n", ep->d_name);
                pd.pid = atoi(ep->d_name);
                if (pd.pid == pid) {
                    LOGW("Trying to add root-PID (%d) "
                         "as thread-PID (%d). Skipping...\n", pid, pd.pid);
                    continue;
                }
                ASSURE_E(mlist_add_last(pid_trigger_list, &pd), return 0);
                cnt++;
            }
        }
        (void)closedir(dp);
    } else {
        LOGE("Couldn't open the directory");
        return 0;
    }

    LOGI("Read %d threads PID:s to list\n", cnt);

    return 1;
}

/* Iterate through PID's and for each, create a new list with a translated
 * version of the event_list */
int proc_expand_events(handle_t pid_trigger_list, handle_t event_list)
{
    int rc, cnt;
    struct node *n;

    for (n = mlist_head(pid_trigger_list), cnt = 0;
         n; n = mlist_next(pid_trigger_list), cnt++) {
        struct pid_trigger *pt;

        assert(n->pl);
        pt = mdata_curr(pid_trigger_list);
        LOGD("PID #%d in progress\n", pt->pid);
        assert_np((rc =
                   mlist_opencreate(sizeof(struct efilter), NULL,
                                    &pt->efilter_list)) == 0);
        ASSURE_E(expand_each_event(pt->pid, pt->efilter_list, event_list),
                 goto err);
    }

    LOGI("Build-up of %d interpreted event-filters one for each PID", cnt);

    return 1;
err:
    return 0;
}

/*
 * Takes every piece from the trigger_list and builds up final filters
 * It will very actively modify the event-list => the filter string */
int proc_concat_epieces(handle_t event_list, handle_t pid_trigger_list)
{
    int cnt, i;
    int n_ev;

    n_ev = mlist_len(event_list);
    char *s_table[n_ev];
    int i_table[n_ev];

    for (i = 0; i < n_ev; i++) {
        s_table[i] = malloc(FINAL_FILTER_MAX);
        memset(s_table[i], 0, FINAL_FILTER_MAX);
        i_table[i] = 0;
    }

    LOGV("Build-up of final event filter {#n PID,name,filter}:\n");
    for (mlist_head(pid_trigger_list), cnt = 0;
         mlist_curr(pid_trigger_list); mlist_next(pid_trigger_list)) {

        struct pid_trigger *pd = mdata_curr(pid_trigger_list);
        for (mlist_head(pd->efilter_list);
             mlist_curr(pd->efilter_list);
             mlist_next(pd->efilter_list), cnt++) {

            struct efilter *ef = mdata_curr(pd->efilter_list);
            int ti = ef->event->id;
            i_table[ti] +=
                sprintf(&(s_table[ti])[i_table[ti]], "( %s ) || ", ef->efilter);
            LOGV("  #%d %d,%s,%s\n", cnt, pd->pid, ef->event->name,
                 s_table[ti]);

        }
    }
    /* Shorten strings somewhat to remove the final residual operator */
    for (i = 0; i < n_ev; i++) {
        (s_table[i])[i_table[i] - 3] = 0;
    }

    /* Finally replace the old filters with the newly created ones */
    for (mlist_head(event_list), cnt = 0;
         mlist_curr(event_list); mlist_next(event_list), cnt++) {

        struct event *ev = mdata_curr(event_list);
        free(ev->filter);
        ev->filter = s_table[cnt];
    }
    return 1;
}

/*
 * Transports filters and corresponding events. I.e. enables ftrace by
 * arming it. */
int proc_ftrace_arm(handle_t event_list)
{
    int cnt;
    struct event *ev;

    LOGD("Transfer list to tracefs \n");
    for (mlist_head(event_list), cnt = 0;
         mlist_curr(event_list); mlist_next(event_list), cnt++) {

        ev = mdata_curr(event_list);
        ASSURE_E(write_by_name
                 ("0", "%s/events/%s/enable", etrace.tracefs_path, ev->name),
                 goto err);
        ASSURE_E(write_by_name
                 (ev->filter, "%s/events/%s/filter", etrace.tracefs_path,
                  ev->name), goto err);
        ASSURE_E(write_by_name
                 ("1", "%s/events/%s/enable", etrace.tracefs_path, ev->name),
                 goto err);
    }
    return 1;
err:
    LOGE("Failure trying to arm event #%d/%s\n", ev->id, ev->name);
    return 0;
}

static inline int pline(int fd, char c, int len, char *marker)
{
    int i, mlen;

    mlen = marker ? strlen(marker) : 0;

    ASSURE_E(write_by_fd(fd, "#"), return 0);
    for (i = 0; i < (MARK_POS - 1); i++) {
        ASSURE_E(write_by_fd(fd, "%c", c), return 0);
    }
    if (mlen != 0) {
        ASSURE_E(write_by_fd(fd, " %s ", marker), return 0);
        mlen += 2;
    }
    for (i = 0; i < (len - mlen); i++) {
        ASSURE_E(write_by_fd(fd, "%c", c), return 0);
    }
    ASSURE_E(write_by_fd(fd, "\n"), return 0);

    return 1;
}

int proc_pheader(struct etrace *etrace)
{
    char buff[1024];
    int i;

    ASSURE_E(pline(etrace->out_fd, '#', 80, NULL), return 0);
    ASSURE_E(write_by_fd(etrace->out_fd, "# ETRACE\n"), return 0);
    ASSURE_E(pline(etrace->out_fd, '#', 80, NULL), return 0);
    ASSURE_E(write_by_fd(etrace->out_fd, "# ID: %d\n", etrace->opts->rid),
             return 0);
    ASSURE_E(write_by_fd
             (etrace->out_fd, "# Time: %6d.%06d\n", (int)etrace->stime.tv_sec,
              (int)etrace->stime.tv_usec), return 0);
    ASSURE_E(write_by_fd(etrace->out_fd, "# PID: %d\n", etrace->opts->pid),
             return 0);

    ASSURE_E(read_by_name(buff, sizeof(buff), "/proc/%d/comm", etrace->pid)
             >= 0, return 0);
    /*Note: no EOL. Data contains one already */
    ASSURE_E(write_by_fd(etrace->out_fd, "# comm: %s", buff), return 0);

    ASSURE_E(write_by_fd(etrace->out_fd, "# TIDs:\n#   "), return 0);

    for (mlist_head(etrace->pid_trigger_list), i = 0;
         mlist_curr(etrace->pid_trigger_list);
         mlist_next(etrace->pid_trigger_list), i++) {

        struct pid_trigger *pd = mdata_curr(etrace->pid_trigger_list);

        if (i < 10) {
            ASSURE_E(write_by_fd(etrace->out_fd, "%d ", pd->pid), return 0);
        } else {
            i = 0;
            ASSURE_E(write_by_fd(etrace->out_fd, "\n#   %d ", pd->pid),
                     return 0);
        }
    }
    ASSURE_E(write_by_fd(etrace->out_fd, "\n#\n"), return 0);
    ASSURE_E(pline(etrace->out_fd, '#', 80, "~~HEADER END~~"), return 0);

    return 1;
}

/*
 * Write to file by name
 * */
int write_by_name(const char *outbuff, const char *format, ...)
{
    char fname[PATH_MAX];
    int fd, ssize, nchars;
    va_list args;

    va_start(args, format);
    memset(fname, 0, PATH_MAX);
    vsnprintf(fname, PATH_MAX, format, args);
    va_end(args);

    LOGD("Accessing file: %s\n", fname);
    ASSURE_E((fd = open(fname, O_WRONLY)) != -1, goto open_err);
    LOGV("Writing buffer: %s\n", outbuff);
    ssize = strlen(outbuff);
    ASSURE_E((nchars = write(fd, outbuff, ssize)) != -1, goto io_err);
    LOGD("Wrote %d/%d\n", ssize, nchars);
    ASSURE_E(ssize == nchars, goto wr_err);

    close(fd);
    return 1;

io_err:
    close(fd);
open_err:
    LOGE("File operation failed with errno: %d \"%s\"\n", errno,
         strerror(errno));
    return 0;
wr_err:
    close(fd);
    LOGE("Didn't write full buffer: %d/%d\n", ssize, nchars);
    return 0;
}

/*
 * Write to file by fd
 *
 * Write short formatted messages. Note that on error fd is *not* closed to
 * avoid double free, if used concurrently.
 * */
int write_by_fd(int fd, const char *msg_format, ...)
{
    int ssize, nchars;
    va_list args;
    char buff[80];
    int buff_size = sizeof(buff);

    va_start(args, msg_format);
    memset(buff, 0, buff_size);
    ASSURE_E(vsnprintf(buff, buff_size, msg_format, args), goto buf_oflow);
    va_end(args);

    LOGV("Write buffer: %s\n", buff);
    ssize = strlen(buff);
    ASSURE_E((nchars = write(fd, buff, ssize)) != -1, goto io_err);
    LOGV("Wrote %d/%d\n", ssize, nchars);
    ASSURE_E(ssize == nchars, goto wr_err);

    return 1;

io_err:
    LOGE("File operation failed with errno: %d \"%s\"\n", errno,
         strerror(errno));
    return 0;
wr_err:
    LOGE("Didn't write full buffer: %d/%d\n", ssize, nchars);
    return 0;
buf_oflow:
    LOGE("Buffer too small for formatted message: %d\n", buff_size);
    return 0;
}

/*
 * Read from file by name
 * */
int read_by_name(char *inbuff, int ssize, const char *format, ...)
{
    char fname[PATH_MAX];
    int fd, nchars = -1;
    va_list args;

    va_start(args, format);
    memset(fname, 0, PATH_MAX);
    vsnprintf(fname, PATH_MAX, format, args);
    va_end(args);

    LOGD("Accessing file: %s\n", fname);
    ASSURE_E((fd = open(fname, O_RDONLY)) != -1, goto open_err);
    ASSURE_E((nchars = read(fd, inbuff, ssize)) >= 0, goto io_err);
    LOGV("Read buffer: %s\n", inbuff);
    LOGD("Read %d\n", nchars);
    ASSURE_E(nchars <= ssize, goto buf_oflow);

    close(fd);
    return nchars;

io_err:
    close(fd);
open_err:
    LOGE("File operation failed with errno: %d \"%s\"\n", errno,
         strerror(errno));
    return nchars;
buf_oflow:
    LOGE("Buffer too small: %d\n", ssize);
    return -1;
}

static inline int expand_each_event(pid_t pid, handle_t efilter_list,
                                    handle_t event_list)
{
    regex_t re;
    char numbuf[MAX_DGTS];
    snprintf(numbuf, MAX_DGTS, "%d", pid);

    assert_np(regcomp(&re, TID_REGEXP, REG_ICASE | REG_EXTENDED) == 0);

    for (mlist_head(event_list); mlist_curr(event_list); mlist_next(event_list)) {
        struct efilter ef;
        struct event *e = mdata_curr(event_list);

        assert(e);
        ef.event = e;
        strncpy(ef.efilter, e->filter, FILTER_MAX);
        EXCEPTION_E(rreplace(ef.efilter, FILTER_MAX, &re, numbuf), goto err_rr);
        LOGD("  Filter [%s]: [%s]->[%s]\n", ef.event->name,
             ef.event->filter, ef.efilter);

        ASSURE_E(mlist_add_last(efilter_list, &ef), goto err_list);

    }

    regfree(&re);
    return 1;
err_rr:
    regfree(&re);
    LOGE("FAIL parsing");
    return 0;
err_list:
    regfree(&re);
    LOGE("Critical: list-operation");
    return 0;

}

/*
 * Replace occurrences (defined by compiled re) in buff by rp
 * */
static inline int rreplace(char *buf, int size, regex_t *re, char *rp)
{
    char *pos;
    int sub, so, n;
    regmatch_t pmatch[10];      /* regoff_t is int so size is int */

    if (regexec(re, buf, 10, pmatch, 0))
        return 0;
    for (pos = rp; *pos; pos++)
        if (*pos == '\\' && *(pos + 1) > '0' && *(pos + 1) <= '9') {
            so = pmatch[*(pos + 1) - 48].rm_so;
            n = pmatch[*(pos + 1) - 48].rm_eo - so;
            if (so < 0 || strlen(rp) + n - 1 > size)
                return 1;
            memmove(pos + n, pos + 2, strlen(pos) - 1);
            memmove(pos, buf + so, n);
            pos = pos + n - 2;
        }
    sub = pmatch[1].rm_so;      /* no repeated replace when sub >= 0 */
    for (pos = buf; !regexec(re, pos, 1, pmatch, 0);) {
        n = pmatch[0].rm_eo - pmatch[0].rm_so;
        pos += pmatch[0].rm_so;
        if (strlen(buf) - n + strlen(rp) + 1 > size)
            return 1;
        memmove(pos + strlen(rp), pos + n, strlen(pos) - n + 1);
        memmove(pos, rp, strlen(rp));
        pos += strlen(rp);
        if (sub >= 0)
            break;
    }
    return 0;
}
