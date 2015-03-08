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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <assure.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "local.h"

static inline struct node *forward(off_t n)
{
    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

static inline struct node *reverse(off_t n)
{
    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

/* Frees payloads memory and node itself, removes list from the
   module-global list, inerts new sentinels if needed and lastly free the
   memory */
static inline void free_list(struct listheader *L)
{
    struct node *bp = L->bp;
    assert(bp);
    struct node *p;

    /* Lists content of nodes are freed */
    for (p = L->phead; p; p = p->next) {
        free(p->pl);            /* Need checking if list is shallow or deep (TBD) */
        free(p);
    }

    /* Lists itself is unlinked from module-global list */
    if (bp->prev)
        bp->prev->next = bp->next;

    if (bp->next)
        bp->next->prev = bp->prev;

    mlistmod_data.nlists--;

    /* Module list of list will always point at the last node. If this is the
       one we're removing, change that that pointer to the nodes previous.
       Recursively this should eventually also point to NULL so the modules fini
       is happy */

    if (bp == mlistmod_data.mlists)
        mlistmod_data.mlists = bp->prev;

    free(bp);
    free(L);
}

/* Operations list level                            */
/* ================================================ */
int mlist_opencreate(int sz,
                     int (*cmpfunc) (LDATA * lval, LDATA * rval),
                     handle_t * hndl)
{
    struct listheader *L = NULL;
    assert_ext(mlistmod_data.isinit);

    if (!mlistmod_data.mlists) {
        /* If list of lists is all empty, create also first empty node */
        mlistmod_data.mlists = malloc(sizeof(struct node));
        assert_ext(mlistmod_data.mlists);
        memset(mlistmod_data.mlists, 0, sizeof(struct node));
        /* nodes prev/next left zero (NULL) on purpose */
    } else {
        /*Create a new empty node at tail */
        mlistmod_data.mlists->next = malloc(sizeof(struct node));
        assert_ext(mlistmod_data.mlists->next);
        memset(mlistmod_data.mlists->next, 0, sizeof(struct node));
        mlistmod_data.mlists->next->prev = mlistmod_data.mlists;
        /*Last one created becomes list head (i.e. order reversed) */
        mlistmod_data.mlists = mlistmod_data.mlists->next;
    }

    mlistmod_data.mlists->pl = malloc(sizeof(struct listheader));
    assert_ext(mlistmod_data.mlists->pl);

    /*Cotinue initialize our list headers payload */
    L = mlistmod_data.mlists->pl;
    memset(L, 0, sizeof(struct listheader));

    L->bp = mlistmod_data.mlists;
    L->nr_links = 0;
    L->owner = NULL;
    L->p = NULL;
    L->phead = NULL;
    L->ptail = NULL;
    L->pl_sz = sz;
    L->cmpfunc = cmpfunc;
    mlistmod_data.nlists++;
    *hndl = (handle_t) L;
    return 0;
};

/* Create a copy of one list-header. This can be used to iterate (seek)
 * over. Note that the content (file) is still the same, and content is
 * still not thread safe (like any ordinary file, content is not thread
 * safe).
 * */
int mlist_dup(handle_t * new_hndl, handle_t orig_hndl)
{
    struct listheader *L;
    assert_ext(mlistmod_data.isinit);

    assert_ext(orig_hndl);
    L = malloc(sizeof(struct listheader));
    memcpy(L, (void *)orig_hndl, sizeof(struct listheader));

    ((struct listheader *)orig_hndl)->nr_links++;
    L->owner = ((struct listheader *)orig_hndl);
    L->nr_links = 0;

    *new_hndl = (handle_t) L;
    return 0;
}

/* Closes (destroys) a handle */
int mlist_close(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    if ((struct listheader *)L->owner) {
        /* This list was duped */
        ((struct listheader *)L->owner)->nr_links--;

        /* If original listheader (owner) is also finally unlinked, it can be
           destroyed */
        free_list(L->owner);
    }
    if (L->nr_links == 0) {
        /* If list has links, this must remain stale until the last link has
           closed, only then can this listheader also be freed */

        free_list(L);
        return (0);
    }
    /* Note that for the root handle, more sophisticated stuff is needed */
    return 0;
};

/* Operations on node level                         */
/* ================================================ */
int mlist_delete(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return 0;
};

int dstrct_mlist(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return 0;
};

struct node *mlist_next(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;

    L->p = L->p->next;
    return (L->p);
};

/*Returns payload at current nodes position */
LDATA *mlist_curr(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;

    return (L->p->pl);
};

struct node *mlist_prev(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;

    L->p = L->p->prev;
    return (L->p);
};

struct node *mlist_head(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    return L->p = L->phead;
};

struct node *mlist_tail(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    return L->p = L->ptail;
};

struct node *mlist_add(const handle_t handle, const LDATA * data)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    if (!L->cmpfunc)
        return mlist_add_last(handle, data);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_add_last(const handle_t handle, const LDATA * data)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;

    if (!L->nelem) {
        /* If list is all empty, create also first empty node  */
        L->p = malloc(sizeof(struct node));
        assert_ext(L->p);
        memset(L->p, 0, sizeof(struct node));
        /*All 3 element pointers point at the same */
        L->phead = L->ptail = L->p;
        /* nodes prev/next left zero (NULL) on purpose */
    } else {
        /*Create a new empty node at tail */
        L->ptail->next = malloc(sizeof(struct node));
        assert_ext(L->ptail->next);
        memset(L->ptail->next, 0, sizeof(struct node));
        L->ptail->next->prev = L->ptail;
        L->ptail = L->ptail->next;
        /* Note. No need to save/restore list header temporary as it's intact. */
    }

    L->ptail->pl = malloc(L->pl_sz);
    memcpy(L->ptail->pl, data, L->pl_sz);
    L->nelem++;
    assert_ext(L != L->p->pl);

    /* Note that this enforces that *different* handles are used if the same
     * list can be both iterated and added too at the same time concurrently */
    L->p = L->ptail;
    return L->p;
};

struct node *mlist_add_first(const handle_t handle, const LDATA * data)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_del(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_del_last(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_del_first(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_dsrct(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_dsrct_last(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_dsrct_first(const handle_t handle)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_lseek(const handle_t handle, off_t offset, int whence)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

struct node *mlist_search(const handle_t handle, const LDATA * data)
{
    assert_ext(mlistmod_data.isinit);
    assert_ext(handle && "invalid or not initialized");

    struct listheader *L = (struct listheader *)handle;
    assert(L);

    assert_ext(!TBD_UNFINISHED);
    return NULL;
};

/*
struct node *mlist_curr(const handle_t handle)
{
    return ((struct listheader *)handle)->p;
}
*/

off_t mlist_offset(const handle_t handle)
{
    return ((struct listheader *)handle)->o;
}

int mlist_len(const handle_t handle)
{
    return ((struct listheader *)handle)->nelem;
}

int mlist_dsize(const handle_t handle)
{
    return ((struct listheader *)handle)->pl_sz;
}

int mlist_nlinks(const handle_t handle)
{
    return ((struct listheader *)handle)->nr_links;
}

/*
    int (*cmpfunc) (LDATA * lval, LDATA * rval);
    return ((struct listheader *)handle)->(*cmpfunc);
    }
*/

/*
struct node *mlist_head(const handle_t handle)
{
    return ((struct listheader *)handle)->phead;
}

struct node *mlist_tail(const handle_t handle)
{
    return ((struct listheader *)handle)->ptail;
}
*/
