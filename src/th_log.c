#include "th_log.h"

#include <stdio.h>

/* global log instance */

static th_log* th_user_log = NULL;

TH_PUBLIC(void)
th_log_set(th_log* log)
{
    th_user_log = log;
}

/** th_log_get
 * @brief  Get the current user log instance.
 * @return The current user log instance, or the default log instance if no user log is set.
 */
TH_LOCAL(th_log*)
th_log_get(void)
{
    return th_user_log ? th_user_log : th_default_log_get();
}

/* th_default_log implementation begin */

/** th_default_log
 * @brief Default log implementation, simply prints log messages to stderr.
 */
typedef struct th_default_log {
    th_log base;
} th_default_log;

TH_LOCAL(void)
th_default_log_print(void* self, int level, const char* msg)
{
    (void)self;
    (void)level;
    fprintf(stderr, "%s\n", msg);
}

TH_PRIVATE(th_log*)
th_default_log_get(void)
{
    static th_default_log log = {
        .base = {
            .print = th_default_log_print,
        }};
    return (th_log*)&log;
}

TH_PRIVATE(void)
th_log_printf(int level, const char* fmt, ...)
{
    th_log* log = th_log_get();
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if (ret < 0 || (size_t)ret >= sizeof(buffer))
        goto on_error;
    log->print(log, level, buffer);
    return;
on_error:
    log->print(log, TH_LOG_LEVEL_ERROR, "ERROR: [th_log] Failed to format log message");
}

/* th_default_log implementation end */
