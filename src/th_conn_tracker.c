#include "th_conn_tracker.h"
#include "th_align.h"
#include "th_conn.h"

TH_LOCAL(void)
th_conn_tracker_on_conn_init(th_conn_observer* observer, th_conn_observable* observable)
{
    th_conn_tracker* tracker = (th_conn_tracker*)observer;
    th_conn_observable_list_push_back(&tracker->observables, observable);
    ++tracker->count;
}

TH_LOCAL(void)
th_conn_tracker_on_conn_deinit(th_conn_observer* observer, th_conn_observable* observable)
{
    th_conn_tracker* tracker = (th_conn_tracker*)observer;
    th_conn_observable_list_erase(&tracker->observables, observable);
    --tracker->count;
    if (tracker->task) {
        th_task* task = TH_MOVE_PTR(tracker->task);
        th_task_complete(task);
        th_task_destroy(task);
    }
}

TH_PRIVATE(void)
th_conn_tracker_init(th_conn_tracker* tracker)
{
    tracker->base.on_init = th_conn_tracker_on_conn_init;
    tracker->base.on_deinit = th_conn_tracker_on_conn_deinit;
    tracker->observables = (th_conn_observable_list){0};
    tracker->task = NULL;
    tracker->count = 0;
}

TH_PRIVATE(void)
th_conn_tracker_cancel_all(th_conn_tracker* conn_tracker)
{
    th_conn_observable* observable = NULL;
    for (observable = th_conn_observable_list_front(&conn_tracker->observables);
         observable != NULL;
         observable = th_conn_observable_list_next(observable)) {
        th_conn* client = &observable->base;
        th_socket_cancel(th_conn_get_socket(client));
    }
}

TH_PRIVATE(void)
th_conn_tracker_async_wait(th_conn_tracker* conn_tracker, th_task* task)
{
    TH_ASSERT(conn_tracker->task == NULL && "Task already set");
    TH_ASSERT(th_conn_observable_list_front(&conn_tracker->observables) != NULL && "No clients to wait for");
    conn_tracker->task = task;
}

TH_PRIVATE(size_t)
th_conn_tracker_count(const th_conn_tracker* conn_tracker)
{
    return conn_tracker->count;
}

TH_PRIVATE(void)
th_conn_tracker_deinit(th_conn_tracker* tracker)
{
    (void)tracker;
    TH_ASSERT(th_conn_observable_list_front(&tracker->observables) == NULL && "All clients must be destroyed before deinit");
}
