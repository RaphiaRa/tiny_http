#ifndef TH_TASK_H
#define TH_TASK_H

#include <stdbool.h>
#include <stdlib.h>

#include "th_allocator.h"
#include "th_queue.h"

typedef struct th_task {
    /** fn
     * @brief The function to execute.
     */
    void (*fn)(void* self);

    /** destroy
     * @brief The destructor for the th_task.
     * Can be NULL if the th_task does not need to be destroyed.
     */
    void (*destroy)(void* self);

    /** This is used internally by the runner. */
    struct th_task* next;
} th_task;

/** th_task_init
 * @brief Initializes a task.
 */
TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void* self), void (*destroy)(void* self));

/** th_task complete
 * @brief Runs the task.
 */
TH_PRIVATE(void)
th_task_complete(th_task* task);

/** th_task_destroy
 * @brief Destroys the task, if the task has a destroy function.
 */
TH_PRIVATE(void)
th_task_destroy(th_task* task);

/* th_task_queue declarations begin */

#ifndef TH_TASK_QUEUE
#define TH_TASK_QUEUE
TH_DEFINE_QUEUE(th_task_queue, th_task)
#endif

/* th_task_queue declarations end */

#endif
