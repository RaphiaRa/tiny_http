#ifndef TH_client_tracker_H
#define TH_client_tracker_H

/** th_client_tracker
 * @brief The client tracker keep track of all clients that are currently active.
 * It is used to cancel all clients when the server is shutting down.
 */

#include "th_allocator.h"
#include "th_client.h"
#include "th_list.h"
#include "th_task.h"

TH_DEFINE_LIST(th_client_observable_list, th_client_observable, prev, next)

typedef struct th_client_tracker {
    th_client_observer base;
    th_client_observable_list observables;
    th_task* task;
    size_t count;
} th_client_tracker;

TH_PRIVATE(void)
th_client_tracker_init(th_client_tracker* client_tracker);

TH_PRIVATE(void)
th_client_tracker_cancel_all(th_client_tracker* client_tracker);

TH_PRIVATE(void)
th_client_tracker_async_wait(th_client_tracker* client_tracker, th_task* task);

TH_PRIVATE(size_t)
th_client_tracker_count(th_client_tracker* client_tracker) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_client_tracker_deinit(th_client_tracker* client_tracker);

#endif
