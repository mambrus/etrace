#include <stdio.h>
#include <config.h>
#include <sys/time.h>

//#undef NDEBUG
#define NDEBUG
#include <assert.h>
#include "assure.h"
#include <log.h>
#include <mtime.h>
#include "opts.h"
#include "etrace.h"
#include <mlist.h>

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
    .rid            = 0,
    .daemon         = 0
/* *INDENT-ON* */
};

struct etrace etrace = {
/* *INDENT-OFF* */
    .opts             = &opts,
/* *INDENT-ON* */
};

int main(int argc, char **argv)
{
    int rc, cnt;
    struct node *n;

    assert_ext((rc =
                mlist_opencreate(sizeof(struct event), NULL,
                                 &etrace.event_list)
               ) == 0);
    assert_ext((rc = mlist_opencreate(sizeof(pid_t), NULL, &etrace.pid_list)
               ) == 0);

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

    assert_np(time_now(&etrace.stime));
    sprintf(etrace.outfname, "%d_%06d_%06d_%d.trc", etrace.opts->rid,
            (int)etrace.stime.tv_sec, (int)etrace.stime.tv_usec,
            etrace.opts->pid);
    LOGI("Out-file name: %s", etrace.outfname);

/* *INDENT-OFF* */
    for (
        n = mlist_head(etrace.event_list), cnt=0;
        n; 
        n = mlist_next(etrace.event_list)
    ) {
/* *INDENT-ON* */
        struct event *e;
        assert(n->pl);
        e=mlist_curr(etrace.event_list);
        LOGI("Event #%d:\n",cnt++);
        if (strlen(e->name)==0)
            LOGE("  %s\n",e->name);
        else
            LOGI("  %s\n",e->name);
        LOGI("  %s\n",e->filter);
    }

    assert_ext((rc = mlist_close(etrace.event_list)
               ) == 0);
    assert_ext((rc = mlist_close(etrace.pid_list)
               ) == 0);

    return 0;
}
