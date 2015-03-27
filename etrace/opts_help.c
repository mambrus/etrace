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
#include "opts.h"
#include "etrace.h"
#include "config.h"

const char *program_version = "etrace " VERSION;

void opts_help(FILE *file, int flags)
{
    if (file && flags & HELP_USAGE) {
        fprintf(file, "%s",
                "Usage: etrace [-DzhutV]\n"
                "            [-T time] [--period=time]\n"
                "            [-c name] [--clock=name] \n"
                "            [-v level] [--verbosity=level] \n"
                "            [-m path] [--debugfs=path] \n"
                "            [-w path] [--workdir=path] \n"
                "            [-o path] [--out-file=path] \n"
                "            [-p pid] [--process=pid] \n"
                "            [-e name] [--event=name] \n"
                "            [-f expr] [--filter=expr] \n"
                "            [-i id] [--run-id=id] \n"
                "            [--documentation]\n"
                "            [--help] [--usage] [--version]\n");
        fflush(file);
    }
    if (file && flags & HELP_LONG) {
        fprintf(file, "%s",
                "Usage: etrace [OPTION...] \n"
                "etrace command line ftrace-events utility (" VERSION ").\n"
                "\n"
                "Generic options:\n"
                "  -D, --documentation        Output full documentation, then exit\n"
                "  -c, --clock=name           Which ftrace_clock to use for time-stamping\n"
                "                             Set if option seen, else not touched\n"
                "                             Note that valid names depend on system. ("
                xstr(DEF_FTRACE_CLOCK) ")\n"
                "  -T, --period=time          Harvest-time (uS). ("
                xstr(DEF_PTIME) ")\n"
                "  -m, --debugfs=path         Debugfs mount-point path ("
                xstr(DEF_DEBUGFS_PATH) ")\n"
                "  -w, --workdir=path         Output directory ("
                xstr(DEF_WORKDIR) ")\n"
                "  -o, --out-file=path        Filename for output relative workdir.\n"
                "                             If not given, a unique filename is deducted based on\n"
                "                             PID and time, Filename '-' equals stdout\n"
                "  -p, --process=pid          Process-id to track. MANDATORY - NO DEFAULT\n"
                "  -t, --with-threads         Trace also a process threads with equivalent patterns\n"
                "  -e, --event=name           Event to trace. This option can be given multiple times\n"
                "  -f, --filter=expr          Filter for each event. This option can given multiple times.\n"
                "  -i, --run-id=id            A marker to tag a run\n"
                "  -v, --verbosity            Set the verbosity level.\n"
                "                             Levels, listed in increasing verbosity, are:\n"
                "                             critical, error, warning, info, debug, verbose\n"
                "Special:\n" "  -z, --daemon               Run as a daemon\n"
                "  -h, --help                 Print this help\n"
                "  -u, --usage                Give a short usage message\n"
                "  -V, --version              Print program version\n" "\n"
                "Mandatory or optional arguments to long options are also mandatory or optional\n"
                "for any corresponding short options.\n" "\n"
                "Read the manual by passing the -D option\n" "\n"
                "Report bugs to <michael.ambrus@sonymobile.com>.\n");
        fflush(file);
    }

    if (file && flags & HELP_VERSION) {
        fprintf(file, "%s\n", program_version);
        fflush(file);
    }

    if (file && flags & HELP_TRY) {
        fprintf(file, "%s",
                "Try `etrace --help' or `etrace --usage' for more information.\n");
        fflush(file);
    }

    if (file && flags & HELP_EXIT)
        etrace_exit(0);

    if (file && flags & HELP_EXIT_ERR)
        etrace_exit(1);
}
