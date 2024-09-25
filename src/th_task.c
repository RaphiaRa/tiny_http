#include "th_task.h"
#include "th_allocator.h"
#include "th_utility.h"

#include <assert.h>
#include <stdlib.h>

/* th_task functions begin */

TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void*), void (*destroy)(void*))
{
    TH_ASSERT(task);
    task->fn = fn;
    task->destroy = destroy;
    task->next = NULL;
}

TH_PRIVATE(void)
th_task_complete(th_task* task)
{
    if (task->fn)
        task->fn(task);
}

TH_PRIVATE(void)
th_task_destroy(th_task* task)
{
    if (task->destroy)
        task->destroy(task);
}

/* th_task functions end */
