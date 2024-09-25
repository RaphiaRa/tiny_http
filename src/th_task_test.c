#include "th_task.h"
#include "th_test.h"

static void mock_fn(void* data)
{
    (void)data;
}

TH_TEST_BEGIN(task)
{
    TH_TEST_CASE_BEGIN(task_init)
    {
        th_task task;
        th_task_init(&task, mock_fn, NULL);
        TH_EXPECT(task.fn == mock_fn);
        TH_EXPECT(task.destroy == NULL);
        TH_EXPECT(task.next == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(task_queue_init)
    {
        th_task_queue queue = {0};
        TH_EXPECT(th_task_queue_empty(&queue));
        TH_EXPECT(th_task_queue_pop(&queue) == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(task_queue_push_pop)
    {
        th_task_queue queue = {0};
        th_task task1, task2;
        th_task_init(&task1, mock_fn, NULL);
        th_task_init(&task2, mock_fn, NULL);
        th_task_queue_push(&queue, &task1);
        TH_EXPECT(!th_task_queue_empty(&queue));
        th_task_queue_push(&queue, &task2);
        TH_EXPECT(th_task_queue_pop(&queue) == &task1);
        TH_EXPECT(th_task_queue_pop(&queue) == &task2);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
