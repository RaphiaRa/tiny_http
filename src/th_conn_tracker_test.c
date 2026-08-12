#include "th_conn_tracker.h"
#include "th_test.h"

typedef struct th_fake_conn {
    th_conn_observable base;
    bool destroyed;
    bool cancelled;
} th_fake_conn;

static void
th_fake_conn_cancel(void* self)
{
    th_fake_conn* conn = self;
    conn->cancelled = true;
}

static void
th_fake_conn_free(void* self)
{
    th_fake_conn* conn = self;
    conn->destroyed = true;
}

static const th_conn_methods th_fake_conn_methods = {
    .get_address = NULL,
    .start = NULL,
    .recv = NULL,
    .send = NULL,
    .cancel = th_fake_conn_cancel,
    .destroy = th_conn_observable_destroy,
};

static void
th_fake_conn_init(th_fake_conn* conn, th_conn_observer* observer)
{
    conn->destroyed = false;
    conn->cancelled = false;
    th_conn_observable_init(&conn->base, &th_fake_conn_methods, th_fake_conn_free, observer);
}

typedef struct th_recording_task {
    th_task base;
    int complete_count;
} th_recording_task;

static void
th_recording_task_fn(void* self)
{
    th_recording_task* task = self;
    ++task->complete_count;
}

static void
th_recording_task_init(th_recording_task* task)
{
    th_task_init(&task->base, th_recording_task_fn);
    task->complete_count = 0;
}

TH_TEST_BEGIN(conn_tracker)
{
    th_conn_tracker tracker;
    th_conn_tracker_init(&tracker);

    TH_TEST_CASE_BEGIN(conn_tracker_init_has_zero_count)
    {
        TH_EXPECT(th_conn_tracker_count(&tracker) == 0);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_tracks_conn_on_init)
    {
        th_fake_conn conn;
        th_fake_conn_init(&conn, &tracker.base);

        TH_EXPECT(th_conn_tracker_count(&tracker) == 1);

        th_conn_destroy(&conn.base.base);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_untracks_conn_on_destroy)
    {
        th_fake_conn conn;
        th_fake_conn_init(&conn, &tracker.base);

        th_conn_destroy(&conn.base.base);

        TH_EXPECT(th_conn_tracker_count(&tracker) == 0);
        TH_EXPECT(conn.destroyed);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_tracks_multiple_conns)
    {
        th_fake_conn conn1, conn2;
        th_fake_conn_init(&conn1, &tracker.base);
        th_fake_conn_init(&conn2, &tracker.base);

        TH_EXPECT(th_conn_tracker_count(&tracker) == 2);

        th_conn_destroy(&conn1.base.base);
        TH_EXPECT(th_conn_tracker_count(&tracker) == 1);

        th_conn_destroy(&conn2.base.base);
        TH_EXPECT(th_conn_tracker_count(&tracker) == 0);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_async_wait_completes_task_on_any_destroy)
    {
        th_fake_conn conn1, conn2;
        th_fake_conn_init(&conn1, &tracker.base);
        th_fake_conn_init(&conn2, &tracker.base);

        th_recording_task task;
        th_recording_task_init(&task);
        th_conn_tracker_async_wait(&tracker, &task.base);

        th_conn_destroy(&conn1.base.base);

        TH_EXPECT(task.complete_count == 1);
        TH_EXPECT(th_conn_tracker_count(&tracker) == 1);

        th_conn_destroy(&conn2.base.base);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_async_wait_only_completes_task_once)
    {
        th_fake_conn conn1, conn2;
        th_fake_conn_init(&conn1, &tracker.base);
        th_fake_conn_init(&conn2, &tracker.base);

        th_recording_task task;
        th_recording_task_init(&task);
        th_conn_tracker_async_wait(&tracker, &task.base);

        th_conn_destroy(&conn1.base.base);
        th_conn_destroy(&conn2.base.base);

        TH_EXPECT(task.complete_count == 1);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_cancel_all_cancels_every_tracked_conn)
    {
        th_fake_conn conn1, conn2;
        th_fake_conn_init(&conn1, &tracker.base);
        th_fake_conn_init(&conn2, &tracker.base);

        th_conn_tracker_cancel_all(&tracker);

        TH_EXPECT(conn1.cancelled);
        TH_EXPECT(conn2.cancelled);

        th_conn_destroy(&conn1.base.base);
        th_conn_destroy(&conn2.base.base);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(conn_tracker_cancel_all_on_empty_tracker_is_a_noop)
    {
        th_conn_tracker_cancel_all(&tracker);
        th_conn_tracker_deinit(&tracker);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
