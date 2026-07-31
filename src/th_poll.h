#ifndef TH_POLL_H
#define TH_POLL_H

#include <th.h>

#include "th_allocator.h"
#include "th_clock.h"
#include "th_config.h"
#include "th_reactor.h"

#if !defined(TH_CONFIG_OS_WIN)
#include <poll.h>
#include <sys/types.h>

/** th_pollops
 * @brief The poll(2) syscall, injected so tests can control fd readiness
 * without a real fd. th_pollops_os() is the real implementation.
 */
typedef struct th_pollops {
    int (*poll)(void* self, struct pollfd* fds, nfds_t nfds, int timeout_ms);
} th_pollops;

TH_PRIVATE(th_pollops*)
th_pollops_os(void);

/** th_poll_create
 * @brief Create a poll-based reactor.
 * @param clock Clock used for per-handle I/O timeouts.
 * @param ops The poll(2) implementation to use; pass th_pollops_os() in
 * production, a fake in tests.
 */
TH_PRIVATE(th_err)
th_poll_create(th_reactor** out, th_allocator* allocator, th_clock* clock, th_pollops* ops);

#endif /* !TH_CONFIG_OS_WIN */
#endif
