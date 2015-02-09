#ifndef log_h
#define log_h
/**
 * Severity levels for logging functions
 */
typedef enum {
    LOG_LEVEL_VERBOSE,  /* Verbose level logging */
    LOG_LEVEL_DEBUG,    /* Debug level logging */
    LOG_LEVEL_INFO,     /* Information level logging */
    LOG_LEVEL_WARNING,  /* Warning level logging */
    LOG_LEVEL_ERROR,    /* Error level logging */
    LOG_LEVEL_CRITICAL, /* Fatal error level logging */
    LOG_LEVEL_SILENT    /* No output */
} log_level;

void log_write(log_level level, const char *format, ...);
void log_set_verbosity(log_level level);
log_level str2loglevel(const char *str, int *ok);

#endif //log_h
