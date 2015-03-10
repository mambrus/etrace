/***************************************************************************
 *   Copyright (C) 2014 by Michael Ambrus                                  *
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
/*
 * Parse single options for etrace
 */
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "assure.h"
#include <log.h>
#include "opts.h"
#include "doc.h"
#include "etrace.h"

#define not_req    0
#define mandatory  1
#define at_least   0
#define precisely -1

static int uq_eid = 0;

struct req_opt {
    int val;                    /* Flag value (option letter) */
    int req;                    /* Times required to be seen at least */
    int max;                    /* Seen no more than */
    int cnt;                    /* Seen number of times */
};
struct req_opt *req_opt(int val);

extern struct etrace etrace;

/* Parse a single option. */
static int opts_parse_opt(const char *cmd,
                          int key, char *arg, struct opts *opts)
{
    struct event event;
    struct event *event_node;

    memset(&event, 0, sizeof(event));

    switch (key) {
        case 'T':
            req_opt('T')->cnt++;
            opts->ptime = arg ? atoi(arg) : -1;
            break;
        case 'v':
            req_opt('v')->cnt++;
            if (arg[0] >= '0' && arg[0] <= '9')
                *opts->loglevel = arg ? atoi(arg) : 0;
            else {
                int ok;
                *opts->loglevel = str2loglevel(arg, &ok);
                if (!ok)
                    LOGW("loglevel [%s] invalid. Falling back to default\n",
                         arg);
            }
            break;
        case 'z':
            req_opt('z')->cnt++;
            opts->daemon = 1;
            break;
        case 'm':
            req_opt('m')->cnt++;
            strncpy(opts->debugfs_path, arg, PATH_MAX);
            break;
        case 'w':
            req_opt('w')->cnt++;
            strncpy(opts->workdir, arg, PATH_MAX);
            break;
        case 'o':
            req_opt('o')->cnt++;
            strncpy(opts->outfname, arg, PATH_MAX);
            break;
        case 'p':
            req_opt('p')->cnt++;
            opts->pid = arg ? atoi(arg) : 0;
            break;
        case 't':
            req_opt('t')->cnt++;
            opts->threads = 1;
            break;
        case 'e':
            req_opt('e')->cnt++;
            strncpy(event.name, arg, PATH_MAX);
            event.id = uq_eid++;
            assert_np(mlist_add_last(etrace.event_list, &event));
            break;
        case 'f':
            req_opt('f')->cnt++;
            assert_np(event_node = mdata_curr(etrace.event_list));
            if (event_node->filter
                && strnlen(event_node->filter, FILTER_MAX) > 0) {

                LOGE("Filer [%s] is overwritten by [%s] for event [%s] (#%d)",
                     event_node->filter, arg, event_node->name,
                     mlist_len(etrace.event_list));

                LOGE("Check order for options -e and -f\n");
                return E_OPT_USAGE;
            }
            event_node->filter = strndup(arg, FILTER_MAX);
            break;
        case 'i':
            req_opt('i')->cnt++;
            opts->rid = arg ? atoi(arg) : 0;
            break;
        case 'u':
            req_opt('u')->cnt++;
            opts_help(stdout, HELP_USAGE | HELP_EXIT);
            break;
        case 'h':
            req_opt('h')->cnt++;
            opts_help(stdout, HELP_LONG | HELP_EXIT);
            break;
        case 'D':
            req_opt('D')->cnt++;
            doc_print();
            etrace_exit(0);
            break;
        case '?':
            /* getopt_long already printed an error message. */
            opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
            break;
        case ':':
            /* getopt_long already printed an error message. */
            fprintf(stderr, "%s: option `-%c' requires an argument\n",
                    cmd, optopt);
            opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
            break;
        case 'V':
            req_opt('V')->cnt++;
            opts_help(stdout, HELP_VERSION | HELP_EXIT);
            break;
        default:
            fprintf(stderr, "etrace: unrecognized option '-%c'\n", (char)key);
            opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
            break;
    }
    return OPT_OK;
}

static struct option long_options[] = {
/* *INDENT-OFF* */
    {"verbosity",      required_argument,  0,  'v'},
    {"period",         required_argument,  0,  'T'},
    {"debugfs",        required_argument,  0,  'm'},
    {"workdir",        required_argument,  0,  'w'},
    {"out-file",       required_argument,  0,  'o'},
    {"process",        required_argument,  0,  'p'},
    {"with-threads",   no_argument,        0,  't'},
    {"event",          no_argument,        0,  'e'},
    {"filter",         no_argument,        0,  'f'},
    {"run-id",         required_argument,  0,  'i'},
    {"daemon",         no_argument,        0,  'z'},
    {"documentation",  no_argument,        0,  'D'},
    {"help",           no_argument,        0,  'h'},
    {"usage",          no_argument,        0,  'u'},
    {"version",        no_argument,        0,  'V'},
    {0,                0,                  0,  0}
/* *INDENT-ON* */
};

/* Additional structure to keep track of mandatory options. Each entry-index
 * should correspond exactly to long_options. Therefore keep tables close.
 */
static struct req_opt req_opts[] = {
/* *INDENT-OFF* */
    {'v',  not_req,    at_least,   0},
    {'T',  not_req,    precisely,  0},
    {'m',  not_req,    precisely,  0},
    {'w',  not_req,    precisely,  0},
    {'o',  not_req,    precisely,  0},
    {'p',  mandatory,  at_least,   0},
    {'t',  not_req,    precisely,  0},
    {'e',  mandatory,  at_least,   0},
    {'f',  not_req,    at_least,   0},
    {'i',  not_req,    precisely,  0},
    {'z',  not_req,    precisely,  0},
    {'D',  not_req,    at_least,   0},
    {'h',  not_req,    at_least,   0},
    {'u',  not_req,    at_least,   0},
    {'V',  not_req,    at_least,   0},
    {0,    0,          0,          0}
/* *INDENT-ON* */
};

struct req_opt *req_opt(int val)
{
    struct req_opt *rop = req_opts;

    for (rop = req_opts; rop->val != 0; rop++) {
        if (rop->val == val)
            return rop;
    }
    assert("req_opt reached sentinel" == 0);
    return rop;
}

/* Returns 0 on success, -1 on error */
static int become_daemon()
{
    switch (fork()) {
        case -1:
            return -1;
        case 0:
            break;
        default:
            _exit(EXIT_SUCCESS);
    }

    if (setsid() == -1)
        return -1;

    switch (fork()) {           /* Ensure we are not session leader */
        case -1:
            return -1;
        case 0:
            break;
        default:
            _exit(EXIT_SUCCESS);
    }

    umask(0);                   /* Clear file mode creation mask */
    chdir("/");                 /* Change to root directory */

    return 0;
}

int opts_check(const struct opts *opts)
{
    int resok = OPT_OK;
    struct option *op = long_options;
    struct req_opt *rop = req_opts;

    LOGD("Checking if mandatory options are set\n");

    for (rop = req_opts, op = long_options; op->name != NULL; rop++, op++) {
        assert(op->val == rop->val);
        if (op->flag)
            LOGV("%-15s %d %4d %c %2d %2d %2d\n",
                 op->name, op->has_arg, &op->flag, op->val,
                 rop->req, rop->cnt, rop->max);
        else
            LOGV("%-15s %d NULL %c %2d %2d %2d\n",
                 op->name, op->has_arg, op->val, rop->req, rop->cnt, rop->max);

        if (rop->cnt < rop->req) {
            LOGE("Mandatory option [\"%s\",'%c'] requirement failed. "
                 "Seen [%d] times, required [%d]",
                 op->name, op->val, rop->cnt, rop->req, rop->max);
            resok = E_OPT_REQ;
        }
        if (rop->max > 0 && rop->cnt > rop->max) {
            LOGE("Count of option [\"%s\",'%c'] requirement failed. "
                 "Seen [%d] times, permitted [%d]",
                 op->name, op->val, rop->cnt, rop->req, rop->max);
            resok = E_OPT_REQ;
        }
        if (rop->max == precisely && rop->cnt != rop->max && rop->req > 0) {
            LOGE("Count of option [\"%s\",'%c'] requirement failed. "
                 "Seen [%d] times, expected [%d]",
                 op->name, op->val, rop->cnt, rop->req, rop->max);
            resok = E_OPT_REQ;
        }
    }

    return resok;
}

/* Parse options
 *
 * Returns number of options parsed.
 * Negative return-value indicate error.
 *
 * */
int opts_parse(int argc, char **argv, struct opts *opts)
{
    int rc, parsed_options = 0;

    while (1) {
        int option_index = 0;
        int c = getopt_long(argc, argv,
                            "v:T:m:p:te:f:i:w:o:zDuhV",
                            long_options,
                            &option_index);
        /* Detect the end of the options. */
        if (c == -1)
            break;
        ASSURE_E((rc =
                  opts_parse_opt(argv[0], c, optarg, opts)) == OPT_OK,
                 return rc);
        parsed_options++;
    }

    /* Handle any remaining command line arguments (not options). */
    if (optind < argc) {
        perror("etrace: Too many arguments, etrace takes only options.\n");
        fflush(stderr);
        opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
    }

    if (opts->daemon) {
        become_daemon();
    }
    return parsed_options;
}
