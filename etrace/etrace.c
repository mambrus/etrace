#include <stdio.h>
#include <config.h>

//#undef NDEBUG
#define NDEBUG
#include "assure.h"
#include <log.h>
#include "opts.h"

/* This binds when grobal variable initialization is run in .start, i.e.
 * before CTOR */
log_level log_filter_level = DEF_LOG_LEVEL;

struct opts opts = {
/* *INDENT-OFF* */
    .loglevel       = &log_filter_level,
    .ptime          = DEF_PTIME,
    .debugfs_path   = DEF_DEBUGFS_PATH,
    .workdir        = DEF_WORKDIR,
    .pid            = 0,
    .daemon         = 0
/* *INDENT-ON* */
};

int main(int argc, char **argv)
{
    int rc;

    rc = opts_parse(argc, argv, &opts);
    LOGI("Parsed %d options\n", rc);
    assert_ext(opts_check(&opts) == 1);

    printf("Hello world: v%s \n", VERSION);
    //log_set_verbosity(LOG_LEVEL_VERBOSE);
    LOGV("Hello [VERBOSE] %d\n", LOG_LEVEL_VERBOSE);
    LOGD("Hello [DEBUG] %d\n", LOG_LEVEL_DEBUG);
    LOGI("Hello [INFO] %d\n", LOG_LEVEL_INFO);
    LOGW("Hello [WARNING] %d\n", LOG_LEVEL_WARNING);
    LOGE("Hello [ERROR] %d\n", LOG_LEVEL_ERROR);
    //LOGC("Hello [CRITICAL] %d\n", LOG_LEVEL_CRITICAL);

    return 0;
}
