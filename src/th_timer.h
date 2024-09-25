#ifndef TH_TIMER_H
#define TH_TIMER_H

#include <th.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "th_config.h"

typedef struct th_timer {
    uint32_t expire;
} th_timer;

TH_PRIVATE(void)
th_timer_init(th_timer* timer);

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration);

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer);

#endif
