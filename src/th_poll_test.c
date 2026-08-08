#include "th_poll.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

typedef struct th_fake_pollops {
    th_pollops base;
    short revents[8];
    int ret;
} th_fake_pollops;

static int
th_fake_poll(void* self, struct pollfd* fds, nfds_t nfds, int timeout_ms)
{
    (void)timeout_ms;
    th_fake_pollops* ops = self;
    for (nfds_t i = 0; i < nfds && i < TH_ARRAY_SIZE(ops->revents); ++i) {
        fds[i].revents = ops->revents[i];
    }
    return ops->ret;
}

static void
th_fake_pollops_init(th_fake_pollops* ops)
{
    ops->base.poll = th_fake_poll;
    memset(ops->revents, 0, sizeof(ops->revents));
    ops->ret = 0;
}

typedef struct th_fake_clock {
    th_clock base;
    time_t now;
} th_fake_clock;

static th_err
th_fake_clock_monotonic_now(void* self, time_t* out)
{
    th_fake_clock* clock = self;
    *out = clock->now;
    return TH_ERR_OK;
}

static void
th_fake_clock_init(th_fake_clock* clock, time_t now)
{
    clock->base.monotonic_now = th_fake_clock_monotonic_now;
    clock->now = now;
}

typedef struct th_test_op {
    th_op base;
    th_handle* handle;
    int runs;
    th_err aborted_with;
    bool aborted;
} th_test_op;

static void
th_test_op_fn(void* self)
{
    th_test_op* op = self;
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    if (--op->runs <= 0) {
        th_op_set_flags(&op->base, TH_OP_COMPLETED);
        return;
    }
    th_handle_submit(op->handle, &op->base);
}

static void
th_test_op_abort(void* self, th_err err)
{
    th_test_op* op = self;
    op->aborted = true;
    op->aborted_with = err;
}

static void
th_test_op_init(th_test_op* op, th_handle* handle, th_op_type type, int runs)
{
    th_op_init(&op->base, type, th_test_op_fn, th_test_op_abort);
    op->handle = handle;
    op->runs = runs;
    op->aborted = false;
    op->aborted_with = TH_ERR_OK;
}

TH_TEST_BEGIN(poll)
{
    th_fake_pollops ops = {0};
    th_fake_pollops_init(&ops);
    th_fake_clock clock = {0};
    th_fake_clock_init(&clock, 100);
    th_loop loop = {0};
    th_loop_init(&loop, NULL);
    th_reactor* reactor = NULL;
    TH_EXPECT(th_poll_create(&reactor, &loop, th_default_allocator_get(), &clock.base, &ops.base) == TH_ERR_OK);
    th_handle* handle = NULL;
    TH_EXPECT(th_reactor_create_handle(reactor, &handle, 42) == TH_ERR_OK);

    TH_TEST_CASE_BEGIN(poll_create_handle_reports_fd)
    {
        TH_EXPECT(th_handle_get_fd(handle) == 42);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_submit_read_runs_op_on_readiness)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 1;
        ops.revents[0] = POLLIN;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(th_op_get_flags(&op.base) & TH_OP_COMPLETED);
        TH_EXPECT(!op.aborted);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_submit_write_waits_for_pollout)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_WRITE, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 1;
        ops.revents[0] = POLLOUT;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(th_op_get_flags(&op.base) & TH_OP_COMPLETED);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_op_needing_three_runs_completes)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 3);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 1;
        ops.revents[0] = POLLIN;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(!(th_op_get_flags(&op.base) & TH_OP_COMPLETED));
        th_reactor_run(reactor, 1000);
        TH_EXPECT(th_op_get_flags(&op.base) & TH_OP_COMPLETED);
        TH_EXPECT(!op.aborted);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_no_readiness_reenqueues_op)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 0; /* poll() timed out, nothing ready */
        th_reactor_run(reactor, 1000);
        TH_EXPECT(!(th_op_get_flags(&op.base) & TH_OP_COMPLETED));
        TH_EXPECT(!op.aborted);
        ops.ret = 1;
        ops.revents[0] = POLLIN;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(th_op_get_flags(&op.base) & TH_OP_COMPLETED);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_pollhup_aborts_with_eof)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 1;
        ops.revents[0] = POLLHUP;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(op.aborted);
        TH_EXPECT(op.aborted_with == TH_ERR_EOF);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_pollerr_aborts_with_eio)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 1;
        ops.revents[0] = POLLERR;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(op.aborted);
        TH_EXPECT(op.aborted_with == TH_ERR_SYSTEM(TH_EIO));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_pollnval_aborts_with_ebadf)
    {
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 1;
        ops.revents[0] = POLLNVAL;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(op.aborted);
        TH_EXPECT(op.aborted_with == TH_ERR_SYSTEM(TH_EBADF));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_timeout_aborts_pending_op)
    {
        th_handle_enable_timeout(handle, true);
        th_test_op op;
        th_test_op_init(&op, handle, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(handle, &op.base) == TH_ERR_OK);
        ops.ret = 0; /* not ready */
        clock.now += TH_CONFIG_IO_TIMEOUT + 1;
        th_reactor_run(reactor, 1000);
        TH_EXPECT(op.aborted);
        TH_EXPECT(op.aborted_with == TH_ERR_SYSTEM(TH_ETIMEDOUT));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_cancel_aborts_pending_ops)
    {
        th_test_op read_op;
        th_test_op write_op;
        th_test_op_init(&read_op, handle, TH_OP_READ, 2);
        th_test_op_init(&write_op, handle, TH_OP_WRITE, 2);
        TH_EXPECT(th_handle_submit(handle, &read_op.base) == TH_ERR_OK);
        TH_EXPECT(th_handle_submit(handle, &write_op.base) == TH_ERR_OK);
        th_handle_cancel(handle);
        TH_EXPECT(read_op.aborted);
        TH_EXPECT(read_op.aborted_with == TH_ERR_SYSTEM(TH_ECANCELED));
        TH_EXPECT(write_op.aborted);
        TH_EXPECT(write_op.aborted_with == TH_ERR_SYSTEM(TH_ECANCELED));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(poll_removed_handle_fd_is_ignored)
    {
        th_handle* other = NULL;
        TH_EXPECT(th_reactor_create_handle(reactor, &other, 7) == TH_ERR_OK);
        th_test_op op;
        th_test_op_init(&op, other, TH_OP_READ, 2);
        TH_EXPECT(th_handle_submit(other, &op.base) == TH_ERR_OK);
        th_handle_destroy(other);
        ops.ret = 1;
        ops.revents[0] = POLLIN;
        th_reactor_run(reactor, 1000);
    }
    TH_TEST_CASE_END

    th_handle_destroy(handle);
    th_reactor_destroy(reactor);
    th_loop_deinit(&loop);
}
TH_TEST_END
