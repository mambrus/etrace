#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <config.h>
#include <syslog.h>
#include "log.h"
#define __init __attribute__((constructor))
#define __fini __attribute__((destructor))


#ifdef LOG_SYSLOG_ENABLED
#   define DEF_LOG_LEVEL LOG_LEVEL_WARNING
#else
#   define DEF_LOG_LEVEL LOG_LEVEL_INFO
#endif
#define ENV_LOG_LEVEL "LOG_LEVEL"

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

static int get_loglevel(void) {
    int log_level = DEF_LOG_LEVEL;

    if ((getenv(ENV_LOG_LEVEL) != NULL)
        && (strlen(getenv(ENV_LOG_LEVEL)) > 0)) {

        int valid_value;
        log_level = str2loglevel(getenv(ENV_LOG_LEVEL), &valid_value);

        if (!valid_value) {
            log_level = DEF_LOG_LEVEL;
        }
    }
    return log_level;
}

void __init __logging_init(void)
{
    int log_level = get_loglevel();

    openlog(PROJ_NAME,
            LOG_CONS | LOG_NDELAY | LOG_NOWAIT | LOG_PERROR | LOG_PID,
            LOG_USER);

    bladerf_log_set_verbosity(log_level);
    log_debug("%s %s: initializing\n", PROJ_NAME, VERSION);
}

void __fini __logging_fini(void)
{
    int log_level = get_loglevel();

    bladerf_log_set_verbosity(log_level);
    log_debug("% %s: deinitializing\n", PROJ_NAME, VERSION);
    fflush(NULL);
    closelog();
}
