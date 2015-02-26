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

#include <log.h>
#include "opts.h"
#include "doc.h"

/* Parse a single option. */
static void opts_parse_opt(
	const char *cmd,
	int key,
	char *arg,
	struct opts *opts
){
	switch (key) {
		case 'T':
			opts->ptime = arg ? atoi (arg) : -1;
			break;
		case 'v':
			if (arg[0]>='0' && arg[0]<='9')
				opts->loglevel = arg ? atoi (arg) : 0;
			else {
				int ok;
				opts->loglevel=str2loglevel(arg, &ok);
				if (!ok)
					LOGW("loglevel [%s] invalid. Falling back to default\n",
						arg);
			}
			break;
		case 'z':
			opts->daemon = 1;
			break;
		case 'm':
			strncpy(opts->debugfs_path, arg, PATH_MAX);
			break;
		case 'w':
			strncpy(opts->workdir, arg, PATH_MAX);
			break;
		case 'p':
			opts->pid = arg ? atoi (arg) : 0;
			break;
		case 'u':
			opts_help(stdout, HELP_USAGE | HELP_EXIT);
			break;
		case 'h':
			opts_help(stdout, HELP_LONG | HELP_EXIT);
			break;
		case 'D':
			doc_print();
			exit(0);
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
			opts_help(stdout, HELP_VERSION | HELP_EXIT);
			break;
		default:
			fprintf(stderr, "etrace: unrecognized option '-%c'\n", (char)key);
			opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
			break;
		}
}

static struct option long_options[] = {
	{"verbosity",     required_argument, 0, 'v'},
	{"period",        required_argument, 0, 'T'},
	{"debugfs",       required_argument, 0, 'm'},
	{"workdir",       required_argument, 0, 'w'},
	{"process",       required_argument, 0, 'p'},
	{"daemon",        no_argument,       0, 'z'},
	{"documentation", no_argument,       0, 'D'},
	{"help",          no_argument,       0, 'h'},
	{"usage",         no_argument,       0, 'u'},
	{"version",       no_argument,       0, 'V'},
	{0, 0, 0, 0}
};

/* Returns 0 on success, -1 on error */
static int become_daemon()
{
	switch (fork()) {
		case -1: return -1;
		case 0:  break;
		default: _exit(EXIT_SUCCESS);
	}

	if (setsid() == -1)
		return -1;

	switch (fork()) {               /* Ensure we are not session leader */
		case -1: return -1;
		case 0:  break;
		default: _exit(EXIT_SUCCESS);
	}

	umask(0);                       /* Clear file mode creation mask */
	chdir("/");                     /* Change to root directory */

	return 0;
}

int opts_parse(int argc, char **argv, struct opts *opts) {
	int parsed_options=0;

	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv,
			"v:T:m:p:w:zDuhV",
			long_options,
			&option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		opts_parse_opt(argv[0], c, optarg, opts);
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

