#include "th_queue.h"
#include "th_test.h"

typedef struct th_queue_test_item {
    struct th_queue_test_item* next;
    int value;
} th_queue_test_item;

TH_DEFINE_QUEUE(th_queue_test_queue, th_queue_test_item)

TH_TEST_BEGIN(queue)
{
    TH_TEST_CASE_BEGIN(queue_pop_last_item_clears_tail)
    {
        th_queue_test_queue queue = th_queue_test_queue_make();
        th_queue_test_item item = {.value = 1};
        th_queue_test_queue_push(&queue, &item);

        TH_EXPECT(th_queue_test_queue_pop(&queue) == &item);
        TH_EXPECT(queue.head == NULL);
        TH_EXPECT(queue.tail == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(queue_push_after_draining_to_empty_reuses_tail_correctly)
    {
        th_queue_test_queue queue = th_queue_test_queue_make();
        th_queue_test_item item1 = {.value = 1};
        th_queue_test_item item2 = {.value = 2};
        th_queue_test_queue_push(&queue, &item1);
        th_queue_test_queue_pop(&queue);

        // if pop left a stale tail pointing at item1, this push would
        // write item2 into item1's already-popped next field instead of
        // becoming both head and tail itself
        th_queue_test_queue_push(&queue, &item2);
        TH_EXPECT(queue.head == &item2);
        TH_EXPECT(queue.tail == &item2);
        TH_EXPECT(th_queue_test_queue_pop(&queue) == &item2);
        TH_EXPECT(th_queue_test_queue_pop(&queue) == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(queue_push_pop_multiple)
    {
        th_queue_test_queue queue = th_queue_test_queue_make();
        th_queue_test_item item1 = {.value = 1};
        th_queue_test_item item2 = {.value = 2};
        th_queue_test_queue_push(&queue, &item1);
        th_queue_test_queue_push(&queue, &item2);

        TH_EXPECT(th_queue_test_queue_pop(&queue) == &item1);
        TH_EXPECT(th_queue_test_queue_pop(&queue) == &item2);
        TH_EXPECT(queue.tail == NULL);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
