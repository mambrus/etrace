/***************************************************************************
 *   Copyright (C) 2006 by Michael Ambrus <ambrmi09@gmail.com>             *
 *   Copyright (C) 2015 by Michael Ambrus <ambrmi09@gmail.com>             *
 *                                                                         *
 *   This file originates from the TinKer project:                         *
 *   https://github.com/mambrus/tinker                                     *
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

/***************************************************************************
   Static inline functions. Need to be outside normal #ifndef file_h trap
 ***************************************************************************/
#include <stdlib.h>

#include <log.h>
#include "config.h"
static inline void notify_failure(char *sassure, const char *sfun, char *sfile,
                                  int iline)
{
#ifdef ENABLE_LOGGING
#  ifdef LOG_INCLUDE_FILE_INFO
    if (sfun != NULL)
        LOGE("%s failed in [%s]\n", sassure, sfun);
    else
        LOGE("%s failed\n", sassure);
#  else
    if (sfun != NULL)
        LOGE("%s failed in [%s] @ [%s:%d]\n", sassure, sfun, sfile, iline);
    else
        LOGE("%s failed @ [%s:%d]\n", sassure, sfile, iline);
#  endif
#else
    if (sfun != NULL)
        fprintf(stderr, "ERROR: %s failed in [%s] @ [%s:%d]\n", sassure,
                sfun, sfile, iline);
    else
        fprintf(stderr, "ERROR: %s failed @ [%s:%d]\n", sassure, sfile, iline);
    fflush(stderr);
#endif
}

static inline void assertfail(char *assertstr,
                              char *filestr, const char *sfun, int line)
{
    notify_failure(assertstr, sfun, filestr, line);

#ifndef NDEBUG
    /* Generate coredump */
    fprintf(stderr, "Calling abort() for coredump \n");
    abort();
    fprintf(stderr, "Abort failed. Null-pointer assignement for coredump \n");
    /* Should never return, but just in case lib is broken (Android?)
     * make a deliberate null pointer assignment */
    *((int *)NULL) = 1;
#else
    fprintf(stderr, "Exit with failure\n");
    exit(1);
#endif
}

/***************************************************************************/
#ifndef assure_h
#define assure_h
/***************************************************************************/

#define _STR(x) #x
#define STR(x) _STR(x)

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# endif
#endif

#if defined(__PRETTY_FUNCTION__)
#define _FNC __PRETTY_FUNCTION__
#elif defined (__FUNCTION__)
#define _FNC __FUNCTION__
#elif defined (__func__)
#define _FNC __func__
#elif defined (_func_)
#define _FNC _func_
#endif

#ifdef _FNC
#define FNC _FNC
#else
#define FNC NULL
#endif
/***************************************************************************
 * ASSURE* / TRUEDO* macros
 ***************************************************************************
 *
 * ASSURE     - Use as assert with the difference that it ignores NDEBUG.
 *              I.e. code inside '(' ')' will never be optimized away.
 * ASSURE_E   - Same as above, but has a dedicated exit ability instead of
 *              aborting (second argument). This can be call to (error)
 *              handling function or semantics like "goto" and "return"
 * TRUEDO     - Same as ASSURE, but with inverse logic. I.e. on "true" it
 *                 will abort.
 *              I.e. code inside '(' ')' will never be optimized away.
 * TRUEDO_E   - Same as above, ASSURE_E but with same inverse logic as
 *                 TRUEDO.
 *
 ***************************************************************************/
#include <string.h>

#define FLE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#ifndef __GNUC__
# define ASSURE(p) ((p) ? (void)0 : (void) notify_failure( \
    #p, FNC, FLE, __LINE__ ) )
# define ASSURE_E(p,e) if (!(p)) {(void) notify_failure( \
    #p, FNC, FLE, __LINE__ ); e;}
# define TRUEDO(p,e) if (p) {(void) notify_failure( \
    #p, FNC, FLE, __LINE__ ); e;}
#else
# define ASSURE(p) ((p) ? (void)0 : (void) notify_failure( \
    #p, __FUNCTION__, FLE, __LINE__ ) )
# define ASSURE_E(p,e) if (!(p)) {(void) notify_failure( \
    #p, __FUNCTION__, FLE, __LINE__ ); e;}
# define TRUEDO(p,e) if (p) {(void) notify_failure( \
    #p, __FUNCTION__, FLE, __LINE__ ); e;}
#endif

/***************************************************************************
 * assrt_* macros
 ***************************************************************************
 *
 * assert_np  - This macro does exactly and what assert does, except that if
 *              NDEBUG is defined it will NOT remove arg. I.e. it is a safe
 *              macro to use instead of assert as replacement
 *
 *              This is a good macro checking return values until design
 *              rigid enough when that isn't needed anymore, when handling
 *              any failure is ignored (by setting NDEBUG).
 *
 * assert_ret - Returns from function with the result. Assumes the function
 *              returns error-codes. Think twice before using this.
 *
 * assert_ext - Same as assert, but handles NDEBUG differently. If NDEBUG is
 *              set, it mimics assert. If not, it exits with a failure.
 *              Either way it always terminates execution.
 *
 ***************************************************************************
   Use of the assert_* macros is discouraged as they will be discontinued.
   Use the for new ASSURE* / TRUEDO* macros instead.
 ***************************************************************************/

#include <stdio.h>

#ifndef NDEBUG
#include <assert.h>
#  define assert_np assert
#else
#  define assert_np(p) (p)
#endif

#define assert_ext(p) ((p) ? (void)0 : (void) assertfail( \
        #p, __FILE__, FNC, __LINE__ ) )

/* Lazy error-handling. Careful using this. Assumes function invoked from
accepts returning with code, and that the code means error */
#  define assert_ret(p) (                                  \
    {                                                      \
        int rc = (p);                                      \
                                                           \
        rc ? (void)0 : (void) notify_failure(              \
            #p, __FUNCTION__, FLE, __LINE__ );             \
        return rc;                                         \
    }                                                      \
)

#endif                          /* assure_h */
