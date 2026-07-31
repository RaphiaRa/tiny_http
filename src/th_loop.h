#ifndef TH_LOOP_H
#define TH_LOOP_H

#include <th.h>

#include "th_config.h"
#include "th_reactor.h"
#include "th_task.h"

/** th_loop
 * @brief Task scheduler: runs queued tasks, and polls the reactor for more
 * work whenever the queue would otherwise go empty.
 */
typedef struct th_loop {
    th_reactor* reactor;
    th_task reactor_task;
    th_task_queue queue;
    size_t num_tasks;
} th_loop;

TH_PRIVATE(void)
th_loop_init(th_loop* loop, th_reactor* reactor);

/** th_loop_push_task
 * @brief Queue a task to run on a future th_loop_poll call.
 */
TH_PRIVATE(void)
th_loop_push_task(th_loop* loop, th_task* task);

/** th_loop_push_uncounted_task
 * @brief Like th_loop_push_task, but for tasks the reactor already counted
 * (e.g. a completion handed back from th_reactor_run) — avoids double count.
 */
TH_PRIVATE(void)
th_loop_push_uncounted_task(th_loop* loop, th_task* task);

/** th_loop_increase_task_count
 * @brief Tells the loop it has pending work it wouldn't otherwise see —
 * e.g. a reactor holding an op pending for readiness, not yet queued.
 * Pair with th_loop_decrease_task_count once that work resolves.
 */
TH_PRIVATE(void)
th_loop_increase_task_count(th_loop* loop);

TH_PRIVATE(void)
th_loop_decrease_task_count(th_loop* loop);

/** th_loop_poll
 * @brief Run exactly one pending task, or poll the reactor for readiness if
 * the queue is otherwise empty (blocking up to timeout_ms in that case).
 * @return TH_ERR_OK on success, TH_ERR_EOF if there are no tasks at all.
 */
TH_PRIVATE(th_err)
th_loop_poll(th_loop* loop, int timeout_ms);

/** th_loop_run
 * @brief Repeatedly polls with a zero timeout until th_loop_poll reports
 * no work left (TH_ERR_EOF). Never blocks.
 */
TH_PRIVATE(void)
th_loop_run(th_loop* loop);

TH_PRIVATE(void)
th_loop_deinit(th_loop* loop);

#endif
