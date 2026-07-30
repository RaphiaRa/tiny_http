#ifndef TH_CLOCK_H
#define TH_CLOCK_H

#include <th.h>

#include "th_config.h"

#include <time.h>

/** th_clock
 * @brief Source of monotonic time for th_timer. Injected as a dependency so
 * tests can supply a fully controllable clock instead of the real one.
 */
typedef struct th_clock {
    /** monotonic_now
     * @brief Write the current monotonic time (in seconds) to *out.
     * @return TH_ERR_OK on success, TH_ERR_SYSTEM(errno) on failure.
     */
    th_err (*monotonic_now)(void* self, time_t* out);
} th_clock;

/** th_clock_os
 * @brief The real, OS-backed clock (POSIX clock_gettime / Windows GetTickCount64).
 */
TH_PRIVATE(th_clock*)
th_clock_os(void);

#endif
