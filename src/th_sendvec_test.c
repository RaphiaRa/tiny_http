#include "th_sendvec.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

typedef struct th_fake_handle {
    th_handle base;
    int fd;
} th_fake_handle;

/* Simulates a reactor that is always immediately ready, driving the
 * op's retry loop synchronously instead of waiting for a real event. */
static th_err
th_fake_handle_submit(void* self, th_op* op)
{
    (void)self;
    th_op_perform(op);
    return TH_ERR_OK;
}

static int
th_fake_handle_get_fd(const void* self)
{
    const th_fake_handle* handle = self;
    return handle->fd;
}

static void
th_fake_handle_enable_timeout(void* self, bool enabled)
{
    (void)self;
    (void)enabled;
}

static void
th_fake_handle_destroy(void* self)
{
    (void)self;
}

static const th_handle_methods th_fake_handle_methods = {
    .cancel = NULL,
    .submit = th_fake_handle_submit,
    .enable_timeout = th_fake_handle_enable_timeout,
    .get_fd = th_fake_handle_get_fd,
    .destroy = th_fake_handle_destroy,
};

typedef struct th_fake_reactor {
    th_reactor base;
    th_fake_handle handle;
} th_fake_reactor;

static th_err
th_fake_reactor_create_handle(void* self, th_handle** out, int fd)
{
    th_fake_reactor* reactor = self;
    reactor->handle.base.methods = &th_fake_handle_methods;
    reactor->handle.fd = fd;
    *out = &reactor->handle.base;
    return TH_ERR_OK;
}

static void
th_fake_reactor_run(void* self, int timeout_ms)
{
    (void)self;
    (void)timeout_ms;
}

static const th_reactor_methods th_fake_reactor_methods = {
    .run = th_fake_reactor_run,
    .create_handle = th_fake_reactor_create_handle,
    .destroy = NULL,
};

static void
th_fake_reactor_init(th_fake_reactor* reactor)
{
    reactor->base.methods = &th_fake_reactor_methods;
}

typedef struct th_fake_socket_ops {
    th_socket_ops base;
    char written[64];
    size_t written_len;
    size_t chunk_len; /* max bytes accepted per call across all iovs; 0 = unlimited */
    th_err err;       /* returned once, then reset to TH_ERR_OK */
} th_fake_socket_ops;

static th_err
th_fake_sendvec(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result)
{
    (void)fd;
    th_fake_socket_ops* ops = self;
    if (ops->err != TH_ERR_OK) {
        th_err err = ops->err;
        ops->err = TH_ERR_OK;
        return err;
    }
    size_t remaining = ops->chunk_len == 0 ? SIZE_MAX : ops->chunk_len;
    size_t total = 0;
    for (size_t i = 0; i < iovcnt && remaining > 0; ++i) {
        size_t n = iov[i].len < remaining ? iov[i].len : remaining;
        memcpy(ops->written + ops->written_len, iov[i].base, n);
        ops->written_len += n;
        remaining -= n;
        total += n;
    }
    *result = total;
    return TH_ERR_OK;
}

static void
th_fake_socket_ops_init(th_fake_socket_ops* ops)
{
    ops->base.send = NULL;
    ops->base.sendvec = th_fake_sendvec;
    ops->base.recv = NULL;
    ops->written_len = 0;
    ops->chunk_len = 0;
    ops->err = TH_ERR_OK;
}

TH_INLINE(void)
th_fake_socket_ops_reset(th_fake_socket_ops* ops)
{
    ops->written_len = 0;
    ops->chunk_len = 0;
    ops->err = TH_ERR_OK;
}

typedef struct th_recorded_result {
    bool called;
    size_t result;
    th_err err;
} th_recorded_result;

static void
th_recorded_result_cb(void* user_data, size_t size, th_err err)
{
    th_recorded_result* result = user_data;
    result->called = true;
    result->result = size;
    result->err = err;
}

static void
th_recorded_result_init(th_recorded_result* result)
{
    result->called = false;
    result->result = 0;
    result->err = TH_ERR_OK;
}

TH_TEST_BEGIN(sendvec)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);
    th_fake_socket_ops ops;
    th_fake_socket_ops_init(&ops);
    th_socket socket;
    th_socket_init(&socket, &loop, &ops.base);
    th_socket_set_fd(&socket, 5);

    TH_TEST_CASE_BEGIN(sendvec_writes_all_buffers_in_one_call)
    {
        th_fake_socket_ops_reset(&ops);

        th_iov iov[2] = {
            {(void*)"hello ", 6},
            {(void*)"world", 5},
        };
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_sendvec_op op;
        th_sendvec_op_init(&op, &socket, iov, 2, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 11);
        TH_EXPECT(memcmp(ops.written, "hello world", 11) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sendvec_retries_across_buffer_boundary)
    {
        th_fake_socket_ops_reset(&ops);
        ops.chunk_len = 4;

        th_iov iov[2] = {
            {(void*)"hello ", 6},
            {(void*)"world", 5},
        };
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_sendvec_op op;
        th_sendvec_op_init(&op, &socket, iov, 2, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 11);
        TH_EXPECT(memcmp(ops.written, "hello world", 11) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sendvec_eagain_submits_and_retries)
    {
        th_fake_socket_ops_reset(&ops);
        ops.err = TH_ERR_SYSTEM(TH_EAGAIN);

        th_iov iov[1] = {{(void*)"hi", 2}};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_sendvec_op op;
        th_sendvec_op_init(&op, &socket, iov, 1, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_OK);
        TH_EXPECT(result.result == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sendvec_error_completes_with_error)
    {
        th_fake_socket_ops_reset(&ops);
        ops.err = TH_ERR_SYSTEM(TH_EIO);

        th_iov iov[1] = {{(void*)"hi", 2}};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_sendvec_op op;
        th_sendvec_op_init(&op, &socket, iov, 1, th_recorded_result_cb, &result);
        th_op_perform(&op.base);
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SYSTEM(TH_EIO));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sendvec_abort_completes_with_given_error)
    {
        th_fake_socket_ops_reset(&ops);

        th_iov iov[1] = {{(void*)"hi", 2}};
        th_recorded_result result;
        th_recorded_result_init(&result);
        th_sendvec_op op;
        th_sendvec_op_init(&op, &socket, iov, 1, th_recorded_result_cb, &result);
        th_op_abort(&op.base, TH_ERR_SYSTEM(TH_ECANCELED));
        th_loop_run(&loop);

        TH_EXPECT(result.called);
        TH_EXPECT(result.err == TH_ERR_SYSTEM(TH_ECANCELED));
    }
    TH_TEST_CASE_END

    th_loop_deinit(&loop);
    th_socket_deinit(&socket);
}
TH_TEST_END
