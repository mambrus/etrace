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

/* Lazy error-handling based on variations of assert  */

/*
 * Like in assert, these macros can be re-assigned and the listen to the
 * same NDEBUG compilation, but differently from assert, they have a
 * different behavior on error when NDEBUG is set.
 *
 * assert_ign - Do the stuff inside the brackets, but ignore handling the
 *              result.
 * assert_np  - same as assert_ign. Comparability reasons only.
 * assert_ret - Returns from function with the result. Assumes the function
 *              returns error-codes. Think twice before use.
 * assert_ext - Same as assert, but don't listen to NDEBUG - always assert.
 *              This is a safe macro to use, but might consume unnecessary
 *              CPU.
 */

#ifndef assure_h
#define assure_h
#include <assert.h>
#include <stdlib.h>

#ifndef NDEBUG
#  define assert_ign assert
#  define assert_np  assert
#  define assert_ret assert
#  define assert_ext assert
#else
#include <stdio.h>
static inline void assertfail(char *assertstr, char *filestr, int line) {

		fprintf(stderr,"assert_ext: \"%s\" %s:%d\n",
			assertstr, filestr, line);
		/* Generate coredump */
		fprintf(stderr,"Calling abort() for coredump \n");
		abort();
		fprintf(stderr,"Abort failed. Null-pointer assignement for coredump \n");
		/* Should never return, but just in case lib is broken (Android?)
		 * make a deliberate null pointer assignment */
		*((int *)NULL) = 1;
}

/* Do the stuff, just ignore acting on the result. */
#  define assert_np(p) (p)
#  define assert_ign assert_np

/* Mimic assert real behavior when NDEBUG is not set. I.e. always act. */

#  define assert_ext(p) ((p) ? (void)0 : (void) assertfail( \
		#p, __FILE__, __LINE__ ) )

/* Lazy error-handling. Careful using this. Assumes function invoked from
accepts returning with code, and that the code means error */
#  define assert_ret(p) (                                  \
	{                                                      \
		int rc = (p);                                      \
                                                           \
		rc ? (void)0 :                                     \
		fprintf(stderr,"assert_ex %s (%s:%d)\n",           \
			#p, __FILE__, __LINE__ );                      \
		exit(rc);                                          \
	}                                                      \
)
#endif

/*****************************************************************************
   Use of the assert_* macros is discouraged as they will be discontinued.
   Use the following for new code instead.
 *****************************************************************************/
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

static inline void notify_failure(
        char *sassure, const char *sfun, char *sfile, int iline) {
#if LOGGING_ENABLED
#  ifdef LOG_INCLUDE_FILE_INFO
    if (sfun!=NULL)
        log_error("%s failed in [%s]\n", sassure, sfun);
    else
        log_error("%s failed\n", sassure);
#  else
    if (sfun!=NULL)
        log_error("%s failed in [%s] @ [%s:%d]\n", sassure,
                sfun, sfile, iline);
    else
        log_error("%s failed @ [%s:%d]\n", sassure,
                sfile, iline);
#  endif
#else
    if (sfun!=NULL)
        fprintf(stderr, "ERROR: %s failed in [%s] @ [%s:%d]\n", sassure,
                sfun, sfile, iline);
    else
        fprintf(stderr, "ERROR: %s failed @ [%s:%d]\n", sassure,
                sfile, iline);
    fflush(stderr);
#endif
}

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

#endif /* assure_h */

