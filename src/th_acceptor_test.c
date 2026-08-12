#include "th_acceptor.h"
#include "th_system_error.h"
#include "th_test.h"

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

typedef struct th_fake_acceptor_ops {
    th_acceptor_ops base;
    th_err socket_err;
    int socket_fd;
    th_err setsockopt_err;
    th_err set_nonblocking_err;
    th_err bind_err;
    th_err listen_err;
    bool closed;
    th_err accept_err;
    int accept_fd;
    int last_fd;
} th_fake_acceptor_ops;

static th_err
th_fake_acceptor_socket(void* self, int domain, int type, int protocol, int* out_fd)
{
    (void)domain;
    (void)type;
    (void)protocol;
    th_fake_acceptor_ops* ops = self;
    if (ops->socket_err != TH_ERR_OK)
        return ops->socket_err;
    *out_fd = ops->socket_fd;
    return TH_ERR_OK;
}

static th_err
th_fake_acceptor_setsockopt(void* self, int fd, int level, int optname, const void* optval, socklen_t optlen)
{
    (void)fd;
    (void)level;
    (void)optname;
    (void)optval;
    (void)optlen;
    th_fake_acceptor_ops* ops = self;
    return ops->setsockopt_err;
}

static th_err
th_fake_acceptor_set_nonblocking(void* self, int fd)
{
    (void)fd;
    th_fake_acceptor_ops* ops = self;
    return ops->set_nonblocking_err;
}

static th_err
th_fake_acceptor_bind(void* self, int fd, const struct sockaddr* addr, socklen_t addrlen)
{
    (void)fd;
    (void)addr;
    (void)addrlen;
    th_fake_acceptor_ops* ops = self;
    return ops->bind_err;
}

static th_err
th_fake_acceptor_listen(void* self, int fd, int backlog)
{
    (void)fd;
    (void)backlog;
    th_fake_acceptor_ops* ops = self;
    return ops->listen_err;
}

static void
th_fake_acceptor_close(void* self, int fd)
{
    (void)fd;
    th_fake_acceptor_ops* ops = self;
    ops->closed = true;
}

static th_err
th_fake_acceptor_accept(void* self, int fd, th_address* addr, int* out_fd)
{
    (void)addr;
    th_fake_acceptor_ops* ops = self;
    ops->last_fd = fd;
    if (ops->accept_err != TH_ERR_OK)
        return ops->accept_err;
    *out_fd = ops->accept_fd;
    return TH_ERR_OK;
}

static void
th_fake_acceptor_ops_init(th_fake_acceptor_ops* ops)
{
    ops->base.socket = th_fake_acceptor_socket;
    ops->base.setsockopt = th_fake_acceptor_setsockopt;
    ops->base.set_nonblocking = th_fake_acceptor_set_nonblocking;
    ops->base.bind = th_fake_acceptor_bind;
    ops->base.listen = th_fake_acceptor_listen;
    ops->base.close = th_fake_acceptor_close;
    ops->base.accept = th_fake_acceptor_accept;
    ops->socket_err = TH_ERR_OK;
    ops->socket_fd = 9;
    ops->setsockopt_err = TH_ERR_OK;
    ops->set_nonblocking_err = TH_ERR_OK;
    ops->bind_err = TH_ERR_OK;
    ops->listen_err = TH_ERR_OK;
    ops->closed = false;
    ops->accept_err = TH_ERR_OK;
    ops->accept_fd = -1;
    ops->last_fd = -1;
}

TH_TEST_BEGIN(acceptor)
{
    th_fake_reactor reactor;
    th_fake_reactor_init(&reactor);
    th_loop loop;
    th_loop_init(&loop, &reactor.base);
    th_fake_acceptor_ops ops;
    th_fake_acceptor_ops_init(&ops);
    th_acceptor acceptor;
    th_acceptor_init(&acceptor, &loop, &ops.base);
    th_addrinfo info = {0};

    TH_TEST_CASE_BEGIN(acceptor_init_has_no_fd)
    {
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_registers_handle_without_timeout)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == 9);
        TH_EXPECT(reactor.handle.timeout_enabled == false);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_propagates_socket_error)
    {
        ops.socket_err = TH_ERR_SYSTEM(TH_EIO);
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_SYSTEM(TH_EIO));
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_propagates_bind_error_and_closes_fd)
    {
        ops.bind_err = TH_ERR_SYSTEM(TH_EADDRINUSE);
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_SYSTEM(TH_EADDRINUSE));
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
        TH_EXPECT(ops.closed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_propagates_reactor_error)
    {
        reactor.create_handle_err = TH_ERR_SYSTEM(TH_EBADF);
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_SYSTEM(TH_EBADF));
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_open_closes_previous_handle)
    {
        ops.socket_fd = 1;
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        ops.socket_fd = 2;
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == 2);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_accept_calls_ops_with_fd)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        ops.accept_fd = 42;
        th_address addr;
        th_socket socket;
        th_socket_init(&socket, &loop, NULL);
        TH_EXPECT(th_acceptor_accept(&acceptor, &addr, &socket) == TH_ERR_OK);
        TH_EXPECT(ops.last_fd == 9);
        TH_EXPECT(th_socket_get_fd(&socket) == 42);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_accept_propagates_eagain)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        ops.accept_err = TH_ERR_SYSTEM(TH_EAGAIN);
        th_address addr;
        th_socket socket;
        th_socket_init(&socket, &loop, NULL);
        TH_EXPECT(th_acceptor_accept(&acceptor, &addr, &socket) == TH_ERR_SYSTEM(TH_EAGAIN));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_cancel_forwards_to_handle)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        th_acceptor_cancel(&acceptor);
        TH_EXPECT(reactor.handle.cancelled);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(acceptor_close_destroys_handle_and_clears_fd)
    {
        TH_EXPECT(th_acceptor_open(&acceptor, &info) == TH_ERR_OK);
        th_acceptor_close(&acceptor);
        TH_EXPECT(reactor.handle.destroyed);
        TH_EXPECT(th_acceptor_get_fd(&acceptor) == -1);
    }
    TH_TEST_CASE_END

    th_acceptor_deinit(&acceptor);
    th_loop_deinit(&loop);
}
TH_TEST_END
