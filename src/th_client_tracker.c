#include "th_client_tracker.h"
#include "th_align.h"
#include "th_client.h"

TH_LOCAL(void)
th_client_tracker_on_client_init(th_client_observer* observer, th_client_observable* observable)
{
    th_client_tracker* tracker = (th_client_tracker*)observer;
    th_client_observable_list_push_back(&tracker->observables, observable);
    ++tracker->count;
}

TH_LOCAL(void)
th_client_tracker_on_client_deinit(th_client_observer* observer, th_client_observable* observable)
{
    th_client_tracker* tracker = (th_client_tracker*)observer;
    th_client_observable_list_erase(&tracker->observables, observable);
    --tracker->count;
    if (tracker->task) {
        th_task* task = TH_MOVE_PTR(tracker->task);
        th_task_complete(task);
        th_task_destroy(task);
    }
}

TH_PRIVATE(void)
th_client_tracker_init(th_client_tracker* tracker)
{
    tracker->base.on_init = th_client_tracker_on_client_init;
    tracker->base.on_deinit = th_client_tracker_on_client_deinit;
    tracker->observables = (th_client_observable_list){0};
    tracker->task = NULL;
    tracker->count = 0;
}

TH_PRIVATE(void)
th_client_tracker_cancel_all(th_client_tracker* client_tracker)
{
    th_client_observable* observable = NULL;
    for (observable = th_client_observable_list_front(&client_tracker->observables);
         observable != NULL;
         observable = th_client_observable_list_next(observable)) {
        th_client* client = &observable->base;
        th_socket_cancel(th_client_get_socket(client));
    }
}

TH_PRIVATE(void)
th_client_tracker_async_wait(th_client_tracker* client_tracker, th_task* task)
{
    TH_ASSERT(client_tracker->task == NULL && "Task already set");
    TH_ASSERT(th_client_observable_list_front(&client_tracker->observables) != NULL && "No clients to wait for");
    client_tracker->task = task;
}

TH_PRIVATE(size_t)
th_client_tracker_count(th_client_tracker* client_tracker)
{
    return client_tracker->count;
}

TH_PRIVATE(void)
th_client_tracker_deinit(th_client_tracker* tracker)
{
    (void)tracker;
    TH_ASSERT(th_client_observable_list_front(&tracker->observables) == NULL && "All clients must be destroyed before deinit");
}
