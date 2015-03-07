#ifndef log_h
#define log_h

/**
 * Severity levels for logging functions
 */
typedef enum {
    LOG_LEVEL_VERBOSE,          /* Verbose level logging */
    LOG_LEVEL_DEBUG,            /* Debug level logging */
    LOG_LEVEL_INFO,             /* Information level logging */
    LOG_LEVEL_WARNING,          /* Warning level logging */
    LOG_LEVEL_ERROR,            /* Error level logging */
    LOG_LEVEL_CRITICAL,         /* Fatal error level logging */
    LOG_LEVEL_SILENT            /* No output */
} log_level;

#ifdef LOG_SYSLOG_ENABLED
#   define DEF_LOG_LEVEL LOG_LEVEL_WARNING
#else
#   define DEF_LOG_LEVEL LOG_LEVEL_INFO
#endif

void log_write(log_level level, const char *format, ...);
void log_set_verbosity(log_level level);
log_level str2loglevel(const char *str, int *ok);
int log_getenv_loglevel(void);
void log_test(void);

/* Bladerf compatible macros (too much to type though) */
#define log_verbose(...)	log_write(LOG_LEVEL_VERBOSE, "[V] "__VA_ARGS__)
#define log_debug(...)		log_write(LOG_LEVEL_DEBUG, "[D] "__VA_ARGS__)
#define log_info(...)		log_write(LOG_LEVEL_INFO, "[I] "__VA_ARGS__)
#define log_warning(...)	log_write(LOG_LEVEL_WARNING, "[W] "__VA_ARGS__)
#define log_error(...)		log_write(LOG_LEVEL_ERROR, "[E] "__VA_ARGS__)
#define log_critical(...)	log_write(LOG_LEVEL_CRITICAL, "[C] "__VA_ARGS__)

/* Android translation of the macros above (smoother)*/
#define LOGV log_verbose
#define LOGD log_debug
#define LOGI log_info
#define LOGW log_warning
#define LOGE log_error

#endif                          //log_h
