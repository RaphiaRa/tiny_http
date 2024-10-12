#ifndef TH_CONN_TRACKER_H
#define TH_CONN_TRACKER_H

/** th_conn_tracker
 * @brief The client tracker keep track of all clients that are currently active.
 * It is used to cancel all clients when the server is shutting down.
 */

#include "th_allocator.h"
#include "th_conn.h"
#include "th_list.h"
#include "th_task.h"

TH_DEFINE_LIST(th_conn_observable_list, th_conn_observable, prev, next)

typedef struct th_conn_tracker {
    th_conn_observer base;
    th_conn_observable_list observables;
    th_task* task;
    size_t count;
} th_conn_tracker;

TH_PRIVATE(void)
th_conn_tracker_init(th_conn_tracker* conn_tracker);

TH_PRIVATE(void)
th_conn_tracker_cancel_all(th_conn_tracker* conn_tracker);

TH_PRIVATE(void)
th_conn_tracker_async_wait(th_conn_tracker* conn_tracker, th_task* task);

TH_PRIVATE(size_t)
th_conn_tracker_count(const th_conn_tracker* conn_tracker);

TH_PRIVATE(void)
th_conn_tracker_deinit(th_conn_tracker* conn_tracker);

#endif
