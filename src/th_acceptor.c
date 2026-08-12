#include "th_acceptor.h"

#include "th_config.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TH_LOCAL(th_err)
th_acceptor_ops_os_socket(void* self, int domain, int type, int protocol, int* out_fd)
{
    (void)self;
    int fd = socket(domain, type, protocol);
    if (fd < 0)
        return TH_ERR_SYSTEM(errno);
    *out_fd = fd;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_setsockopt(void* self, int fd, int level, int optname, const void* optval, socklen_t optlen)
{
    (void)self;
    if (setsockopt(fd, level, optname, optval, optlen) < 0)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_set_nonblocking(void* self, int fd)
{
    (void)self;
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_bind(void* self, int fd, const struct sockaddr* addr, socklen_t addrlen)
{
    (void)self;
    if (bind(fd, addr, addrlen) < 0)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_listen(void* self, int fd, int backlog)
{
    (void)self;
    if (listen(fd, backlog) < 0)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_acceptor_ops_os_close(void* self, int fd)
{
    (void)self;
    close(fd);
}

TH_LOCAL(th_err)
th_acceptor_ops_os_accept(void* self, int fd, th_address* addr, int* out_fd)
{
    (void)self;
    int conn_fd = accept(fd, (struct sockaddr*)&addr->addr, &addr->addrlen);
    if (conn_fd < 0)
        return TH_ERR_SYSTEM(errno);
    th_err err = th_acceptor_ops_os_set_nonblocking(self, conn_fd);
    if (err != TH_ERR_OK) {
        close(conn_fd);
        return err;
    }
    *out_fd = conn_fd;
    return TH_ERR_OK;
}

TH_PRIVATE(th_acceptor_ops*)
th_acceptor_ops_os(void)
{
    static th_acceptor_ops ops = {
        .socket = th_acceptor_ops_os_socket,
        .setsockopt = th_acceptor_ops_os_setsockopt,
        .set_nonblocking = th_acceptor_ops_os_set_nonblocking,
        .bind = th_acceptor_ops_os_bind,
        .listen = th_acceptor_ops_os_listen,
        .close = th_acceptor_ops_os_close,
        .accept = th_acceptor_ops_os_accept,
    };
    return &ops;
}

#endif /* TH_CONFIG_OS_POSIX */

TH_LOCAL(th_err)
th_acceptor_open_socket(th_acceptor* acceptor, const th_addrinfo* info, int* out_fd)
{
    th_acceptor_ops* ops = acceptor->ops;
    int fd = -1;
    th_err err = ops->socket(ops, info->family, info->socktype, info->protocol, &fd);
    if (err != TH_ERR_OK)
        return err;

#if TH_CONFIG_REUSE_ADDR
    {
        int optval = 1;
        if ((err = ops->setsockopt(ops, fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) != TH_ERR_OK)
            goto cleanup_fd;
    }
#endif
#if TH_CONFIG_REUSE_PORT
    {
#if defined(SO_REUSEPORT)
        int optval = 1;
        if ((err = ops->setsockopt(ops, fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval))) != TH_ERR_OK)
            goto cleanup_fd;
#else
        TH_LOG_FATAL("SO_REUSEPORT is not supported on this platform");
        err = TH_ERR_NOSUPPORT;
        goto cleanup_fd;
#endif
    }
#endif
    if ((err = ops->set_nonblocking(ops, fd)) != TH_ERR_OK)
        goto cleanup_fd;
    if ((err = ops->bind(ops, fd, (const struct sockaddr*)&info->addr.addr, info->addr.addrlen)) != TH_ERR_OK)
        goto cleanup_fd;
    if ((err = ops->listen(ops, fd, 1024)) != TH_ERR_OK)
        goto cleanup_fd;

    *out_fd = fd;
    return TH_ERR_OK;
cleanup_fd:
    ops->close(ops, fd);
    return err;
}

TH_PRIVATE(void)
th_acceptor_init(th_acceptor* acceptor, th_loop* loop, th_acceptor_ops* ops)
{
    acceptor->loop = loop;
    acceptor->handle = NULL;
    acceptor->ops = ops;
}

TH_PRIVATE(th_err)
th_acceptor_open(th_acceptor* acceptor, const th_addrinfo* info)
{
    int fd = -1;
    th_err err = th_acceptor_open_socket(acceptor, info, &fd);
    if (err != TH_ERR_OK)
        return err;
    th_acceptor_close(acceptor);
    err = th_reactor_create_handle(acceptor->loop->reactor, &acceptor->handle, fd);
    if (err != TH_ERR_OK) {
        acceptor->ops->close(acceptor->ops, fd);
        return err;
    }
    th_handle_enable_timeout(acceptor->handle, false);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_acceptor_close(th_acceptor* acceptor)
{
    if (acceptor->handle) {
        th_handle_destroy(acceptor->handle);
        acceptor->handle = NULL;
    }
}

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor)
{
    th_acceptor_close(acceptor);
}

TH_PRIVATE(th_err)
th_acceptor_accept(th_acceptor* acceptor, th_address* addr, th_socket* out_socket)
{
    int fd = -1;
    th_err err = acceptor->ops->accept(acceptor->ops, th_acceptor_get_fd(acceptor), addr, &fd);
    if (err != TH_ERR_OK)
        return err;
    return th_socket_set_fd(out_socket, fd);
}
