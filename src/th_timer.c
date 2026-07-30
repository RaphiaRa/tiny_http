#include "th_timer.h"
#include "th_config.h"
#include "th_utility.h"

TH_PRIVATE(void)
th_timer_init(th_timer* timer, th_clock* clock)
{
    timer->clock = clock;
    timer->expire = 0;
}

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration)
{
    time_t now = 0;
    th_err err = timer->clock->monotonic_now(timer->clock, &now);
    TH_ASSERT(err == TH_ERR_OK && "clock->monotonic_now failed");
    if (err != TH_ERR_OK)
        return err;
    timer->expire = now + duration.seconds;
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer)
{
    time_t now = 0;
    th_err err = timer->clock->monotonic_now(timer->clock, &now);
    TH_ASSERT(err == TH_ERR_OK && "clock->monotonic_now failed");
    /* We don't return the error here, as it's already handled in th_timer_set
     * and we can safely assume that the error won't happen here. */
    if (err != TH_ERR_OK)
        return true;
    return now >= timer->expire;
}

TH_PRIVATE(th_timer)
th_timer_from_duration(th_clock* clock, th_duration duration)
{
    th_timer timer;
    th_timer_init(&timer, clock);
    th_timer_set(&timer, duration);
    return timer;
}

TH_PRIVATE(th_duration)
th_timer_remaining(const th_timer* timer)
{
    time_t now = 0;
    th_err err = timer->clock->monotonic_now(timer->clock, &now);
    TH_ASSERT(err == TH_ERR_OK && "clock->monotonic_now failed");
    if (err != TH_ERR_OK)
        return th_seconds(0);
    return th_seconds(TH_MAX((int)(timer->expire - now), 0));
}

TH_PRIVATE(bool)
th_timer_less(const th_timer* a, const th_timer* b)
{
    return a->expire < b->expire;
}
