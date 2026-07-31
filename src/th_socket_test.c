#include "th_socket.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

typedef struct th_fake_handle {
    th_handle base;
    int fd;
    bool timeout_enabled;
    bool cancelled;
    bool destroyed;
} th_fake_handle;

static void
th_fake_handle_cancel(void* self)
{
    th_fake_handle* handle = self;
    handle->cancelled = true;
}

static th_err
th_fake_handle_submit(void* self, th_op* op)
{
    (void)self;
    (void)op;
    return TH_ERR_OK;
}

static void
th_fake_handle_enable_timeout(void* self, bool enabled)
{
    th_fake_handle* handle = self;
    handle->timeout_enabled = enabled;
}

static int
th_fake_handle_get_fd(const void* self)
{
    const th_fake_handle* handle = self;
    return handle->fd;
}

static void
th_fake_handle_destroy(void* self)
{
    th_fake_handle* handle = self;
    handle->destroyed = true;
}

static const th_handle_methods th_fake_handle_methods = {
    .cancel = th_fake_handle_cancel,
    .submit = th_fake_handle_submit,
    .enable_timeout = th_fake_handle_enable_timeout,
    .get_fd = th_fake_handle_get_fd,
    .destroy = th_fake_handle_destroy,
};

typedef struct th_fake_reactor {
    th_reactor base;
    th_fake_handle handle;
    th_err create_handle_err;
} th_fake_reactor;

static th_err
th_fake_reactor_create_handle(void* self, th_handle** out, int fd)
{
    th_fake_reactor* reactor = self;
    if (reactor->create_handle_err != TH_ERR_OK)
        return reactor->create_handle_err;
    reactor->handle.base.methods = &th_fake_handle_methods;
    reactor->handle.fd = fd;
    reactor->handle.timeout_enabled = false;
    reactor->handle.cancelled = false;
    reactor->handle.destroyed = false;
    *out = &reactor->handle.base;
    return TH_ERR_OK;
}

static const th_reactor_methods th_fake_reactor_methods = {
    .run = NULL,
    .create_handle = th_fake_reactor_create_handle,
    .destroy = NULL,
};

static void
th_fake_reactor_init(th_fake_reactor* reactor)
{
    reactor->base.methods = &th_fake_reactor_methods;
    reactor->create_handle_err = TH_ERR_OK;
}

typedef struct th_fake_socket_ops {
    th_socket_ops base;
    th_err send_err;
    th_err sendvec_err;
    th_err recv_err;
    size_t send_result;
    size_t sendvec_result;
    size_t recv_result;
    int last_fd;
    size_t last_len;
} th_fake_socket_ops;

static th_err
th_fake_socket_send(void* self, int fd, const void* addr, size_t len, size_t* result)
{
    (void)addr;
    th_fake_socket_ops* ops = self;
    ops->last_fd = fd;
    ops->last_len = len;
    *result = ops->send_result;
    return ops->send_err;
}

static th_err
th_fake_socket_sendvec(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result)
{
    (void)iov;
    th_fake_socket_ops* ops = self;
    ops->last_fd = fd;
    ops->last_len = iovcnt;
    *result = ops->sendvec_result;
    return ops->sendvec_err;
}

static th_err
th_fake_socket_recv(void* self, int fd, void* addr, size_t len, size_t* result)
{
    (void)addr;
    th_fake_socket_ops* ops = self;
    ops->last_fd = fd;
    ops->last_len = len;
    *result = ops->recv_result;
    return ops->recv_err;
}

static void
th_fake_socket_ops_init(th_fake_socket_ops* ops)
{
    ops->base.send = th_fake_socket_send;
    ops->base.sendvec = th_fake_socket_sendvec;
    ops->base.recv = th_fake_socket_recv;
    ops->send_err = TH_ERR_OK;
    ops->sendvec_err = TH_ERR_OK;
    ops->recv_err = TH_ERR_OK;
    ops->send_result = 0;
    ops->sendvec_result = 0;
    ops->recv_result = 0;
    ops->last_fd = -1;
    ops->last_len = 0;
}

TH_TEST_BEGIN(socket)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);
    th_fake_socket_ops ops;
    th_fake_socket_ops_init(&ops);
    th_socket socket;
    th_socket_init(&socket, &loop, &ops.base);

    TH_TEST_CASE_BEGIN(socket_init_has_no_fd)
    {
        TH_EXPECT(th_socket_get_fd(&socket) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_set_fd_registers_handle_with_timeout)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 42) == TH_ERR_OK);
        TH_EXPECT(th_socket_get_fd(&socket) == 42);
        TH_EXPECT(reactor.handle.timeout_enabled);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_set_fd_propagates_reactor_error)
    {
        reactor.create_handle_err = TH_ERR_SYSTEM(TH_EBADF);
        TH_EXPECT(th_socket_set_fd(&socket, 42) == TH_ERR_SYSTEM(TH_EBADF));
        TH_EXPECT(th_socket_get_fd(&socket) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_set_fd_closes_previous_handle)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 1) == TH_ERR_OK);
        TH_EXPECT(th_socket_set_fd(&socket, 2) == TH_ERR_OK);
        TH_EXPECT(reactor.handle.destroyed == false);
        TH_EXPECT(th_socket_get_fd(&socket) == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_send_calls_ops_with_fd)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        ops.send_result = 5;
        size_t result = 0;
        TH_EXPECT(th_socket_send(&socket, "hello", 5, &result) == TH_ERR_OK);
        TH_EXPECT(ops.last_fd == 7);
        TH_EXPECT(ops.last_len == 5);
        TH_EXPECT(result == 5);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_send_propagates_eagain)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        ops.send_err = TH_ERR_SYSTEM(TH_EAGAIN);
        size_t result = 0;
        TH_EXPECT(th_socket_send(&socket, "hello", 5, &result) == TH_ERR_SYSTEM(TH_EAGAIN));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_sendvec_calls_ops)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        th_iov iov[2] = {{0}};
        ops.sendvec_result = 3;
        size_t result = 0;
        TH_EXPECT(th_socket_sendvec(&socket, iov, 2, &result) == TH_ERR_OK);
        TH_EXPECT(ops.last_len == 2);
        TH_EXPECT(result == 3);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_recv_calls_ops)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        char buf[8];
        ops.recv_result = 4;
        size_t result = 0;
        TH_EXPECT(th_socket_recv(&socket, buf, sizeof(buf), &result) == TH_ERR_OK);
        TH_EXPECT(ops.last_len == sizeof(buf));
        TH_EXPECT(result == 4);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_recv_propagates_eof)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        ops.recv_err = TH_ERR_EOF;
        char buf[8];
        size_t result = 0;
        TH_EXPECT(th_socket_recv(&socket, buf, sizeof(buf), &result) == TH_ERR_EOF);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_cancel_forwards_to_handle)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        th_socket_cancel(&socket);
        TH_EXPECT(reactor.handle.cancelled);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(socket_close_destroys_handle_and_clears_fd)
    {
        TH_EXPECT(th_socket_set_fd(&socket, 7) == TH_ERR_OK);
        th_socket_close(&socket);
        TH_EXPECT(reactor.handle.destroyed);
        TH_EXPECT(th_socket_get_fd(&socket) == -1);
    }
    TH_TEST_CASE_END

    th_socket_deinit(&socket);
    th_loop_deinit(&loop);
}
TH_TEST_END
