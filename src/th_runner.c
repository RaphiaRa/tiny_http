#include "th_runner.h"
#include "th_allocator.h"
#include "th_log.h"
#include "th_utility.h"

/* th_runner begin */

TH_PRIVATE(void)
th_runner_init(th_runner* runner)
{
    runner->queue = th_task_queue_make();
    runner->num_tasks = 0;
    runner->waiting = 0;
    th_task_queue_push(&runner->queue, &runner->service_task);
}

TH_PRIVATE(void)
th_runner_set_io_service(th_runner* runner, th_io_service* service)
{
    runner->io_service = service;
}

TH_PRIVATE(void)
th_runner_push_task(th_runner* runner, th_task* task)
{
    ++runner->num_tasks;
    th_task_queue_push(&runner->queue, task);
}

TH_PRIVATE(void)
th_runner_push_uncounted_task(th_runner* runner, th_task* task)
{
    th_task_queue_push(&runner->queue, task);
}

TH_PRIVATE(void)
th_runner_increase_task_count(th_runner* runner)
{
    ++runner->num_tasks;
}

TH_PRIVATE(th_err)
th_runner_poll(th_runner* runner, int timeout_ms)
{
    if (runner->num_tasks == 0) {
        return TH_ERR_EOF;
    }
    while (1) {
        th_task* task = th_task_queue_pop(&runner->queue);
        TH_ASSERT(task && "Task queue must never be empty");
        int empty = th_task_queue_empty(&runner->queue);
        if (task == &runner->service_task) {
            th_io_service_run(runner->io_service, empty ? timeout_ms : 0);
            th_task_queue_push(&runner->queue, &runner->service_task);
            if (empty)
                return TH_ERR_OK;
        } else {
            task->fn(task);
            if (task->destroy)
                task->destroy(task);
            --runner->num_tasks;
            return TH_ERR_OK;
        }
    }
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_runner_drain(th_runner* runner)
{
    th_task* task = NULL;
    while ((task = th_task_queue_pop(&runner->queue))) {
        if (task != &runner->service_task) {
            task->fn(task);
            if (task->destroy)
                task->destroy(task);
            --runner->num_tasks;
        }
    }
}

TH_PRIVATE(void)
th_runner_deinit(th_runner* runner)
{
    th_task* task = NULL;
    while ((task = th_task_queue_pop(&runner->queue))) {
        if (task->destroy)
            task->destroy(task);
    }
}

/* runner end */
