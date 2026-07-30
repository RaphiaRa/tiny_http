#ifndef TH_TIMER_H
#define TH_TIMER_H

#include <th.h>

#include "th_clock.h"
#include "th_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct th_timer {
    th_clock* clock;
    time_t expire;
} th_timer;

/** th_timer_init
 * @brief Initialize a timer, unexpired, using the given clock as its time
 * source. Pass th_clock_os() in production; tests can supply a fake clock.
 */
TH_PRIVATE(void)
th_timer_init(th_timer* timer, th_clock* clock);

/** th_timer_from_duration
 * @brief Create a timer that expires after the given duration.
 * Equivalent to th_timer_init followed by th_timer_set, but the assert-only
 * error handling of th_timer_set means this can never fail in practice.
 */
TH_PRIVATE(th_timer)
th_timer_from_duration(th_clock* clock, th_duration duration);

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration);

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer);

/** th_timer_remaining
 * @brief Time left until the timer expires, clamped to zero (never negative).
 */
TH_PRIVATE(th_duration)
th_timer_remaining(const th_timer* timer);

/** th_timer_less
 * @brief True if `a` expires before `b`. For use in timer lists/heaps.
 */
TH_PRIVATE(bool)
th_timer_less(const th_timer* a, const th_timer* b);

#endif
