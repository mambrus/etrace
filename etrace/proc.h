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
#ifndef proc_h
#define proc_h

#include <limits.h>
#include <mlist.h>
#include "etrace.h"

int proc_tid_tolist(pid_t pid, handle_t pid_trigger_list);
int proc_expand_events(handle_t pid_trigger_list, handle_t event_list);
int proc_concat_epieces(handle_t event_list, handle_t pid_trigger_list);
int proc_ftrace_arm(handle_t event_list);
int proc_pheader(struct etrace *e);

int write_by_name(const char *outbuff, const char *format, ...);
int read_by_name(char *outbuff, int ssize, const char *format, ...);
int write_by_fd(int fd, const char *msg_format, ...);

#endif                          /* proc_h */
