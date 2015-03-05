#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <config.h>
#include "include/mtime.h"

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
 * Keep mtime in to at least once per new build-/run-environment assert that
 * the mechanism works.
 */

void __init __mtime_init(void)
{
    set_clocktype(KERNEL_CLOCK);
}

void __fini __mtime_fini(void)
{
}
