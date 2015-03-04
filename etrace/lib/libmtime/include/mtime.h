/***************************************************************************
 *   Copyright (C) 2013 by Michael Ambrus                                  *
 *   ambrmi09@gmail.com                                                    *
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
#ifndef mtime_h
#define mtime_h

#include <sys/time.h>

typedef enum {
    AUTODETECT = 0,             /* Probes during startup and selects best type */
    KERNEL_CLOCK,               /* As close to kernel time as possible */
    CALENDER_CLOCK              /* Best form of calender clock is used */
} clock_type;

void set_clocktype(clock_type ct);
int time_now(struct timeval *tv);
struct timeval tv_diff(struct timeval t0, struct timeval t1);

#endif                          /* define mtime_h */
