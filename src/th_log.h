#ifndef TH_LOGGER_H
#define TH_LOGGER_H

#include <th.h>

#include <stdarg.h>
#include <stdio.h>

#include "th_config.h"

#ifndef TH_LOG_LEVEL
#define TH_LOG_LEVEL TH_LOG_LEVEL_NONE
#endif

#define TH_LOG_TAG "default"

TH_PRIVATE(th_log*)
th_default_log_get(void);

TH_PRIVATE(void)
th_log_printf(int level, const char* fmt, ...) TH_MAYBE_UNUSED;

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_TRACE
#define TH_LOG_TRACE(...) th_log_printf(TH_LOG_LEVEL_TRACE, "TRACE: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_TRACE(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_DEBUG
#define TH_LOG_DEBUG(...) th_log_printf(TH_LOG_LEVEL_DEBUG, "DEBUG: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_DEBUG(...) ((void)0)
#endif

#if (TH_LOG_LEVEL <= TH_LOG_LEVEL_INFO)
#define TH_LOG_INFO(...) th_log_printf(TH_LOG_LEVEL_INFO, "INFO: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_INFO(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_WARN
#define TH_LOG_WARN(...) th_log_printf(TH_LOG_LEVEL_WARN, "WARN: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_WARN(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_ERROR
#define TH_LOG_ERROR(...) th_log_printf(TH_LOG_LEVEL_ERROR, "ERROR: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_ERROR(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_FATAL
#define TH_LOG_FATAL(...) th_log_printf(TH_LOG_LEVEL_FATAL, "FATAL: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_FATAL(...) ((void)0)
#endif

#endif
