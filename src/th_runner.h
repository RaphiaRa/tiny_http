#ifndef TH_RUNNER_H
#define TH_RUNNER_H

#include "th_io_service.h"
#include "th_task.h"

#include <th.h>

typedef struct th_runner {
    th_io_service* io_service;
    th_task service_task;
    int waiting;
    th_task_queue queue;
    size_t num_tasks;
} th_runner;

TH_PRIVATE(void)
th_runner_init(th_runner* runner);

TH_PRIVATE(void)
th_runner_set_io_service(th_runner* runner, th_io_service* service);

TH_PRIVATE(void)
th_runner_push_task(th_runner* runner, th_task* task);

TH_PRIVATE(void)
th_runner_push_uncounted_task(th_runner* runner, th_task* task);

TH_PRIVATE(void)
th_runner_increase_task_count(th_runner* runner);

TH_PRIVATE(th_err)
th_runner_poll(th_runner* runner, int timeout_ms);

TH_PRIVATE(void)
th_runner_drain(th_runner* runner);

TH_PRIVATE(void)
th_runner_deinit(th_runner* runner);

#endif
