#include "th_clock.h"

#ifdef TH_CONFIG_OS_POSIX
#include <errno.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <windows.h>
#endif

TH_LOCAL(th_err)
th_os_clock_monotonic_now(void* self, time_t* out)
{
    (void)self;
#if defined(TH_CONFIG_OS_POSIX)
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return TH_ERR_SYSTEM(errno);
    }
    *out = ts.tv_sec;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_WIN)
    *out = (time_t)(GetTickCount64() / 1000);
    return TH_ERR_OK;
#else
    (void)out;
    return TH_ERR_NOSUPPORT;
#endif
}

TH_PRIVATE(th_clock*)
th_clock_os(void)
{
    static th_clock os_clock = {
        .monotonic_now = th_os_clock_monotonic_now,
    };
    return &os_clock;
}
