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
#include "config.h"


const char *program_version = "etrace " VERSION;

void opts_help(FILE* file, int flags) {
	if (file && flags & HELP_USAGE) {
		fprintf(file, "%s",
			"Usage: etrace [-DzhuV]\n"
			"            [-T time] [--period=time]\n"
			"            [-v level] [--verbosity=level] \n"
			"            [--documentation]\n"
			"            [--help] [--usage] [--version]\n");
		fflush(file);
	}
	if (file && flags & HELP_LONG) {
		fprintf(file, "%s",
			"Usage: etrace [OPTION...] \n"
			"etrace -- A ftrace events companion."
			"Set-up events/filters, then harvest for a specific time\n"
			"\n"
			"Generic options:\n"
			"  -D, --documentation        Output full documentation, then exit\n"
			"  -T, --period=time          Period-/on-time. In normal mode:\n"
			"                             Period-time in uS. -1 equals event-driven output.\n"
			"                             In \"synchronous strobe mode\":\n"
			"                             Time represents on-time and period is on+off time\n"
			"                             Default: ["xstr(DEF_PRD_OR_ON)"] uS\n"
			"  -v, --verbosity            Set the verbosity level.\n"
			"                             Levels, listed in increasing verbosity, are:\n"
			"                             critical, error, warning, info, debug, verbose\n"
			"Special:\n"
			"  -z, --daemon               Run as a daemon\n"
			"  -h, --help                 Print this help\n"
			"  -u, --usage                Give a short usage message\n"
			"  -V, --version              Print program version\n"
			"\n"
			"Mandatory or optional arguments to long options are also mandatory or optional\n"
			"for any corresponding short options.\n"
			"\n"
			"Read the manual by passing the -D option\n"
			"\n"
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
		exit(0);

	if (file && flags & HELP_EXIT_ERR)
		exit(1);
}
