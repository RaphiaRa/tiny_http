#include "th_loop.h"
#include "th_test.h"

typedef struct th_fake_reactor {
    th_reactor base;
    int run_count;
    int last_timeout_ms;
} th_fake_reactor;

static void
th_fake_reactor_run(void* self, int timeout_ms)
{
    th_fake_reactor* reactor = self;
    ++reactor->run_count;
    reactor->last_timeout_ms = timeout_ms;
}

static const th_reactor_methods th_fake_reactor_methods = {
    .run = th_fake_reactor_run,
    .create_handle = NULL,
    .destroy = NULL,
};

static void
th_fake_reactor_init(th_fake_reactor* reactor)
{
    reactor->base.methods = &th_fake_reactor_methods;
    reactor->run_count = 0;
    reactor->last_timeout_ms = 0;
}

static void
th_noop_task_fn(void* self)
{
    (void)self;
}

TH_TEST_BEGIN(loop)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);

    TH_TEST_CASE_BEGIN(loop_init_has_no_tasks)
    {
        TH_EXPECT(loop.num_tasks == 0);
        TH_EXPECT(th_loop_poll(&loop, 0) == TH_ERR_EOF);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(loop_polls_reactor_with_full_timeout_when_queue_only_has_placeholder)
    {
        TH_EXPECT(th_loop_poll(&loop, 1000) == TH_ERR_EOF);
        th_loop_increase_task_count(&loop);
        TH_EXPECT(th_loop_poll(&loop, 1000) == TH_ERR_OK);
        TH_EXPECT(reactor.run_count == 1);
        TH_EXPECT(reactor.last_timeout_ms == 1000);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(loop_runs_queued_task_polling_reactor_with_zero_timeout_first)
    {
        th_task task;
        th_task_init(&task, th_noop_task_fn, NULL);
        th_loop_push_task(&loop, &task);
        TH_EXPECT(th_loop_poll(&loop, 1000) == TH_ERR_OK);
        TH_EXPECT(reactor.run_count == 1);
        TH_EXPECT(reactor.last_timeout_ms == 0);
        TH_EXPECT(loop.num_tasks == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(loop_push_uncounted_task_does_not_change_count)
    {
        th_task task;
        th_task_init(&task, th_noop_task_fn, NULL);
        size_t before = loop.num_tasks;
        th_loop_push_uncounted_task(&loop, &task);
        TH_EXPECT(loop.num_tasks == before);
        th_loop_increase_task_count(&loop);
        TH_EXPECT(loop.num_tasks == before + 1);
        TH_EXPECT(th_loop_poll(&loop, 0) == TH_ERR_OK);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(loop_drain_runs_all_pending_tasks)
    {
        th_task tasks[3];
        for (int i = 0; i < 3; ++i) {
            th_task_init(&tasks[i], th_noop_task_fn, NULL);
            th_loop_push_task(&loop, &tasks[i]);
        }
        th_loop_run(&loop);
        TH_EXPECT(loop.num_tasks == 0);
    }
    TH_TEST_CASE_END

    th_loop_deinit(&loop);
}
TH_TEST_END
