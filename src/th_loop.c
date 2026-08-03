#include "th_loop.h"
#include "th_utility.h"

TH_PRIVATE(void)
th_loop_init(th_loop* loop, th_reactor* reactor)
{
    loop->reactor = reactor;
    loop->queue = th_task_queue_make();
    loop->num_tasks = 0;
    th_task_init(&loop->reactor_task, NULL, NULL);
    th_task_queue_push(&loop->queue, &loop->reactor_task);
}

TH_PRIVATE(void)
th_loop_push_task(th_loop* loop, th_task* task)
{
    ++loop->num_tasks;
    th_task_queue_push(&loop->queue, task);
}

TH_PRIVATE(void)
th_loop_push_uncounted_task(th_loop* loop, th_task* task)
{
    th_task_queue_push(&loop->queue, task);
}

TH_PRIVATE(void)
th_loop_increase_task_count(th_loop* loop)
{
    ++loop->num_tasks;
}

TH_PRIVATE(void)
th_loop_decrease_task_count(th_loop* loop)
{
    --loop->num_tasks;
}

TH_PRIVATE(th_err)
th_loop_poll(th_loop* loop, int timeout_ms)
{
    if (loop->num_tasks == 0) {
        return TH_ERR_EOF;
    }
    while (1) {
        th_task* task = th_task_queue_pop(&loop->queue);
        TH_ASSERT(task && "Task queue must never be empty");
        bool empty = th_task_queue_empty(&loop->queue);
        if (task == &loop->reactor_task) {
            th_reactor_run(loop->reactor, empty ? timeout_ms : 0);
            th_task_queue_push(&loop->queue, &loop->reactor_task);
            if (empty)
                return TH_ERR_OK;
        } else {
            th_task_complete(task);
            th_task_destroy(task);
            --loop->num_tasks;
            return TH_ERR_OK;
        }
    }
}

TH_PRIVATE(void)
th_loop_run(th_loop* loop)
{
    while (th_loop_poll(loop, 0) == TH_ERR_OK) {
    }
}

TH_PRIVATE(void)
th_loop_deinit(th_loop* loop)
{
    th_task* task = NULL;
    while ((task = th_task_queue_pop(&loop->queue))) {
        th_task_destroy(task);
    }
}
