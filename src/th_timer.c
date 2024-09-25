#include "th_timer.h"
#include "th_config.h"
#include "th_utility.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <time.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <windows.h>
#endif

TH_PRIVATE(void)
th_timer_init(th_timer* timer)
{
    timer->expire = 0;
}

TH_LOCAL(th_err)
th_timer_monotonic_now(uint64_t* out)
{
#if defined(TH_CONFIG_OS_POSIX)
    struct timespec ts = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret != 0) {
        return TH_ERR_SYSTEM(errno);
    }
    *out = ts.tv_sec;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_WIN)
    (void)out;
    return TH_ERR_NOSUPPORT;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)out;
    return TH_ERR_NOSUPPORT;
#endif
}

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration)
{
    uint64_t now = 0;
    th_err err = th_timer_monotonic_now(&now);
    TH_ASSERT(err == TH_ERR_OK && "th_timer_monotonic_now failed");
    if (err != TH_ERR_OK)
        return err;
    timer->expire = now + duration.seconds;
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer)
{
    uint64_t now = 0;
    th_err err = th_timer_monotonic_now(&now);
    TH_ASSERT(err == TH_ERR_OK && "th_timer_monotonic_now failed");
    /* We don't return the error here, as it's already handled in th_timer_set
     * and we can safely assume that the error won't happen here. */
    if (err != TH_ERR_OK)
        return true;
    return now >= timer->expire;
}
