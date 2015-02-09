#include <config.h>
#include <strings.h>
#include <log.h>
#if !defined(WIN32) && !defined(__CYGWIN__) && defined(ENABLE_SYSLOG)
#include <syslog.h>
#endif
#include <stdio.h>
#include <stdarg.h>


log_level str2loglevel(const char *str, int *ok)
{
    log_level level = LOG_LEVEL_ERROR;
    int valid = 0;

    if (!strcasecmp(str, "critical")) {
        level = LOG_LEVEL_CRITICAL;
    } else if (!strcasecmp(str, "error")) {
        level = LOG_LEVEL_ERROR;
    } else if (!strcasecmp(str, "warning")) {
        level = LOG_LEVEL_WARNING;
    } else if (!strcasecmp(str, "info")) {
        level = LOG_LEVEL_INFO;
    } else if (!strcasecmp(str, "debug")) {
        level = LOG_LEVEL_DEBUG;
    } else if (!strcasecmp(str, "verbose")) {
        level = LOG_LEVEL_VERBOSE;
    } else {
        valid = 0;
    }

    *ok = valid;
    return level;
}

static log_level filter_level = LOG_LEVEL_INFO;

void log_write(log_level level, const char *format, ...)
{
    /* Only process this message if its level exceeds the current threshold */
    if (level >= filter_level)
    {
        va_list args;

        /* Write the log message */
        va_start(args, format);
#if defined(WIN32) || defined(__CYGWIN__)
        vfprintf(stderr, format, args);
#else
#  if defined (ENABLE_SYSLOG)
        {
            int syslog_level;

            switch (level) {
                case LOG_LEVEL_VERBOSE:
                case LOG_LEVEL_DEBUG:
                    syslog_level = LOG_DEBUG;
                    break;

                case LOG_LEVEL_INFO:
                    syslog_level = LOG_INFO;
                    break;

                case LOG_LEVEL_WARNING:
                    syslog_level = LOG_WARNING;
                    break;

                case LOG_LEVEL_ERROR:
                    syslog_level = LOG_ERR;
                    break;

                case LOG_LEVEL_CRITICAL:
                    syslog_level = LOG_CRIT;
                    break;

                default:
                    /* Shouldn't be used, so just route it to a low level */
                    syslog_level = LOG_DEBUG;
                    break;
            }

            vsyslog(syslog_level | LOG_USER, format, args);
        }
#  else
        vfprintf(stderr, format, args);
#  endif
#endif
        va_end(args);
    }
}

void log_set_verbosity(log_level level)
{
    filter_level = level;
}
