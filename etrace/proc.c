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
#include <sys/types.h>
#include <dirent.h>

#include <assert.h>
#include "assure.h"
#include <log.h>
#include <mtime.h>
#include "opts.h"
#define LDATA struct pid_trigger
#include "proc.h"
#include <mlist.h>
#include "etrace.h"

/* Populate list with thread process id:s. On linux kernel V3+ this is very
easy: https://lkml.org/lkml/2011/12/28/59
*/
int tid_populate(pid_t pid, handle_t pid_trigger_list, handle_t event_list)
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
            LOGD("Adding PID to list: %d\n", ep->d_name);
            ASSURE_E(mlist_add_last(pid_trigger_list, &pd), return 0);
            cnt++;
        }
        (void)closedir(dp);
    } else {
        LOGE("Couldn't open the directory");
        return 0;
    }

    LOGI("Read %d threads PID:s to list\n", cnt);

    return 1;
}
