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
#include <string.h>
#include <stdio.h>
#include "opts.h"

/* Parse a single option. */
static void opts_parse_opt(
	const char *cmd,
	int key,
	char *arg,
	struct arguments *arguments
){
	extern const char ppclock_doc[];

	switch (key) {
		case 'v':
			arguments->verbose = 1;
			break;
		case 'T':
			arguments->ptime = arg ? atoi (arg) : -1;
			break;
		case 'O':
			arguments->offtime = arg ? atoi (arg) : -1;
			break;
		case 'p':
			if ((arg != NULL) && (!strncmp(arg,"0x",2) || !strncmp(arg,"0X",2)))
				sscanf(arg,"%x",&(arguments->port));
			else
				arguments->port = arg ? atoi (arg) : -1;
			break;
		case 'n':
			arguments->decimals = arg ? atoi (arg) : 0;
			break;
		case 'i':
			arguments->tio_trim = arg ? atoi (arg) : 0;
			break;
		case 'R':
			arguments->realtime = 1;
			break;
		case 'S':
			arguments->ssync = 1;
			break;
		case 'd':
			arguments->debuglevel = arg ? atoi (arg) : 0;
			break;
		case 'z':
			arguments->daemon = 1;
			break;
		case 'L': 
			arguments->segments_tst = 1;
			if (arg!=NULL) {
				arguments->tst_has_startval=1;
				int i=0,divisor=1;
				for (i=0; i<arguments->decimals; i++) {
					divisor*=10;
				}
				arguments->segtst_start_us = arg ? (atoi (arg) % divisor) : -1;
				if (arguments->segtst_start_us == -1)
					arguments->segtst_start_s = -1;
				else
					arguments->segtst_start_s = arg ? (atoi (arg) / divisor) : 0;
			} else {
				arguments->segtst_start_us = arg ? atoi (arg) : -1;
			}
			break;

		case 'u':
			opts_help(stdout, HELP_USAGE | HELP_EXIT);
			break;
		case 'h':
			opts_help(stdout, HELP_LONG | HELP_EXIT);
			break;
		case 'D':
			printf("%s\n",ppclock_doc);
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
			fprintf(stderr, "pclock: unrecognized option '-%c'\n", (char)key);
			opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
			break;
		}
}

static struct option long_options[] = {
	{"verbose",       no_argument,       0, 'v'},
	{"realtime",      no_argument,       0, 'R'},
	{"debuglevel",    required_argument, 0, 'd'},
	{"period",        required_argument, 0, 'T'},
	{"port",          required_argument, 0, 'p'},
	{"decimals",      required_argument, 0, 'n'},
	{"tiotrim",       required_argument, 0, 'i'},
	{"syncstrobe",    no_argument,       0, 'S'},
	{"documentation", no_argument,       0, 'D'},
	{"usage",         no_argument,       0, 'u'},
	{"LED seg test",  optional_argument, 0, 'L'},
	{"version",       no_argument,       0, 'V'},
	{"daemon",        no_argument,       0, 'z'},
	{"version",       no_argument,       0, 'V'},
	{0, 0, 0, 0}
};

/* Returns 0 on success, -1 on error */
static int become_daemon()
{
	int fd;

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

int opts_parse(int argc, char **argv, struct arguments *arguments) {
	int parsed_options=0;

	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv,
			"vT:O:p:n:SRd:i:DzuhVL::",
			long_options,
			&option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		opts_parse_opt(argv[0], c, optarg, arguments);
		parsed_options++;
	}

	/* Handle any remaining command line arguments (not options). */
	if (optind < argc) {
		perror("pclock: Too many arguments, ppclock takes only options.\n");
		fflush(stderr);
		opts_help(stderr, HELP_TRY | HELP_EXIT_ERR);
	}

	if (arguments->daemon) {
		become_daemon();
	}
	return parsed_options;
}

