#include "th_test.h"
#include "th_timer.h"

typedef struct th_fake_clock {
    th_clock base;
    time_t now;
} th_fake_clock;

static th_err
th_fake_clock_monotonic_now(void* self, time_t* out)
{
    th_fake_clock* clock = self;
    *out = clock->now;
    return TH_ERR_OK;
}

static void
th_fake_clock_init(th_fake_clock* clock, time_t now)
{
    clock->base.monotonic_now = th_fake_clock_monotonic_now;
    clock->now = now;
}

TH_TEST_BEGIN(timer)
{
    th_fake_clock clock;
    th_fake_clock_init(&clock, 100);

    TH_TEST_CASE_BEGIN(timer_init)
    {
        th_timer timer;
        th_timer_init(&timer, &clock.base);
        TH_EXPECT(timer.expire == 0);
        TH_EXPECT(th_timer_expired(&timer));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(timer_set_future_not_expired)
    {
        th_timer timer;
        th_timer_init(&timer, &clock.base);
        TH_EXPECT(th_timer_set(&timer, th_seconds(60)) == TH_ERR_OK);
        TH_EXPECT(!th_timer_expired(&timer));
        clock.now += 59;
        TH_EXPECT(!th_timer_expired(&timer));
        clock.now += 1;
        TH_EXPECT(th_timer_expired(&timer));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(timer_set_zero_expired)
    {
        th_timer timer;
        th_timer_init(&timer, &clock.base);
        TH_EXPECT(th_timer_set(&timer, th_seconds(0)) == TH_ERR_OK);
        TH_EXPECT(th_timer_expired(&timer));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(timer_from_duration)
    {
        th_timer timer = th_timer_from_duration(&clock.base, th_seconds(60));
        TH_EXPECT(!th_timer_expired(&timer));
        th_timer expired = th_timer_from_duration(&clock.base, th_seconds(0));
        TH_EXPECT(th_timer_expired(&expired));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(timer_remaining)
    {
        th_timer timer;
        th_timer_init(&timer, &clock.base);
        TH_EXPECT(th_timer_set(&timer, th_seconds(60)) == TH_ERR_OK);
        TH_EXPECT(th_timer_remaining(&timer).seconds == 60);
        clock.now += 40;
        TH_EXPECT(th_timer_remaining(&timer).seconds == 20);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(timer_remaining_clamped_to_zero)
    {
        th_timer timer;
        th_timer_init(&timer, &clock.base);
        TH_EXPECT(th_timer_set(&timer, th_seconds(10)) == TH_ERR_OK);
        clock.now += 100;
        TH_EXPECT(th_timer_remaining(&timer).seconds == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(timer_less)
    {
        th_timer sooner;
        th_timer later;
        th_timer_init(&sooner, &clock.base);
        th_timer_init(&later, &clock.base);
        TH_EXPECT(th_timer_set(&sooner, th_seconds(10)) == TH_ERR_OK);
        TH_EXPECT(th_timer_set(&later, th_seconds(60)) == TH_ERR_OK);
        TH_EXPECT(th_timer_less(&sooner, &later));
        TH_EXPECT(!th_timer_less(&later, &sooner));
        TH_EXPECT(!th_timer_less(&sooner, &sooner));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
