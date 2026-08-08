#include "th_task.h"
#include "th_allocator.h"
#include "th_utility.h"

#include <assert.h>
#include <stdlib.h>

/* th_task functions begin */

TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void*))
{
    TH_ASSERT(task);
    task->fn = fn;
    task->next = NULL;
}

TH_PRIVATE(void)
th_task_complete(th_task* task)
{
    if (task->fn)
        task->fn(task);
}

/* th_task functions end */
