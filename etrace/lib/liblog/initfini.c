#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <config.h>
#include <syslog.h>
#include "log.h"
#define __init __attribute__((constructor))
#define __fini __attribute__((destructor))

/* Module initializers/deinitializers. When used as library (who don't have
 * a natural entry/exit function) these are used to initialize
 * deinitialize. Use to set predefined/default states and cleanup.
 *
 * This will work with shared libraries as well as with static as they get
 * invoked by RTL load/unload, with or without C++ code (i.e. functions will
 * play nice with C++ normal ctors/dtors).
 *
 * Keep log in to at least once per new build-/run-environment assert that
 * the mechanism works.
 */
extern log_level log_filter_level;

void __init __liblog_init(void)
{
    int log_level;

    log_filter_level = DEF_LOG_LEVEL;   /* Global variable initializer, if missed */
    log_level = log_getenv_loglevel();

    openlog(PROJ_NAME,
            LOG_CONS | LOG_NDELAY | LOG_NOWAIT | LOG_PERROR | LOG_PID,
            LOG_USER);

    log_set_verbosity(log_level);
    log_debug("%s %s: initializing\n", PROJ_NAME, VERSION);
}

void __fini __liblog_fini(void)
{
    int log_level = log_getenv_loglevel();

    log_set_verbosity(log_level);
    log_debug("% %s: deinitializing\n", PROJ_NAME, VERSION);
    fflush(NULL);
    closelog();
}
