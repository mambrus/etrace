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

/* Options and opts */ 

#ifndef opts_h
#define opts_h

#define HELP_USAGE     1
#define HELP_LONG      2
#define HELP_VERSION   4
#define HELP_TRY       8
#define HELP_EXIT     16
#define HELP_EXIT_ERR 32

#define DEF_PRD_OR_ON	10
#define DEF_OFF 		1149
#define DEF_SYNC 		1
#define DEF_DECIMALS 	4
#define DEF_PORT 		0x3F8
#define DEF_TIO_TRIM 	48

#define xstr(S) str(S)
#define str(S) #S

/* General opts */
struct opts
{
	int verbose;
	unsigned ptime;
	unsigned offtime;
	unsigned port;
	unsigned decimals;
	unsigned tio_trim;
	int ssync;
	int core;
	int debuglevel;
	int daemon;
	int segments_tst;
	int tst_has_startval;
	int segtst_start_us;
	int segtst_start_s;
	int realtime;
};

#ifndef __KERNEL__
#include <stdio.h>
void opts_help(FILE* file, int flags);
int opts_parse(int argc, char **argv, struct opts *);
#endif

#endif //opts_h

