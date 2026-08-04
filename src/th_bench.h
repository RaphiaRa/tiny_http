#ifndef TH_BENCH_H
#define TH_BENCH_H

#include "th_config.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

TH_INLINE(double)
th_bench_now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

/* Re-runs the whole function once per case, skipping all but the selected
 * one, so shared setup is fresh for every case (same dispatch as th_test.h). */
#define TH_BENCH_BEGIN(name)                         \
    int src_th_##name##_bench(int argc, char** argv) \
    {                                                \
        (void)argc;                                  \
        (void)argv;                                  \
        for (size_t th_target = 0;; th_target++) {   \
            size_t th_index = 0;                     \
            bool th_ran = false;

#define TH_BENCH_END \
    if (!th_ran)     \
        break;       \
    }                \
    return 0;        \
    }

/* Runs the case body `iterations` times, timing the whole loop once (not
 * per iteration - clock_gettime itself isn't free), then reports the
 * average. Keep setup out of the body - hoist it above TH_BENCH_BEGIN so
 * only the real work gets timed. */
#define TH_BENCH_CASE_BEGIN(name, iterations)      \
    if (th_index++ == th_target) {                 \
        th_ran = true;                             \
        const char* th_bench_name = #name;         \
        size_t th_bench_iterations = (iterations); \
        double th_bench_start = th_bench_now_ns(); \
        for (size_t th_bench_i = 0; th_bench_i < th_bench_iterations; th_bench_i++) {

#define TH_BENCH_CASE_END                                                                        \
    }                                                                                            \
    double th_bench_avg_ns = (th_bench_now_ns() - th_bench_start) / (double)th_bench_iterations; \
    printf("%-40s n=%-8zu avg=%9.1fns\n", th_bench_name, th_bench_iterations, th_bench_avg_ns);  \
    }

#endif
