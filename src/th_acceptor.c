#include "th_acceptor.h"

#include "th_config.h"

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TH_LOCAL(th_err)
th_acceptor_ops_os_set_nonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_open(void* self, const char* addr, const char* port, int* out_fd)
{
    (void)self;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* res = NULL;
    if (getaddrinfo(addr, port, &hints, &res) != 0)
        return TH_ERR_SYSTEM(errno);

    th_err err = TH_ERR_OK;
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_addrinfo;
    }
#if TH_CONFIG_REUSE_ADDR
    {
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            err = TH_ERR_SYSTEM(errno);
            goto cleanup_fd;
        }
    }
#endif
#if TH_CONFIG_REUSE_PORT
    {
#if defined(SO_REUSEPORT)
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
            err = TH_ERR_SYSTEM(errno);
            goto cleanup_fd;
        }
#else
        TH_LOG_FATAL("SO_REUSEPORT is not supported on this platform");
        err = TH_ERR_NOSUPPORT;
        goto cleanup_fd;
#endif
    }
#endif
    if ((err = th_acceptor_ops_os_set_nonblocking(fd)) != TH_ERR_OK)
        goto cleanup_fd;
    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if (listen(fd, 1024) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    freeaddrinfo(res);
    *out_fd = fd;
    return TH_ERR_OK;
cleanup_fd:
    close(fd);
cleanup_addrinfo:
    freeaddrinfo(res);
    return err;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_accept(void* self, int fd, th_address* addr, int* out_fd)
{
    (void)self;
    int conn_fd = accept(fd, (struct sockaddr*)&addr->addr, &addr->addrlen);
    if (conn_fd < 0)
        return TH_ERR_SYSTEM(errno);
    th_err err = th_acceptor_ops_os_set_nonblocking(conn_fd);
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
        .open = th_acceptor_ops_os_open,
        .accept = th_acceptor_ops_os_accept,
    };
    return &ops;
}

#endif /* TH_CONFIG_OS_POSIX */

TH_PRIVATE(void)
th_acceptor_init(th_acceptor* acceptor, th_loop* loop, th_acceptor_ops* ops)
{
    acceptor->loop = loop;
    acceptor->handle = NULL;
    acceptor->ops = ops;
}

TH_PRIVATE(th_err)
th_acceptor_open(th_acceptor* acceptor, const char* addr, const char* port)
{
    int fd = -1;
    th_err err = acceptor->ops->open(acceptor->ops, addr, port, &fd);
    if (err != TH_ERR_OK)
        return err;
    th_acceptor_close(acceptor);
    err = th_reactor_create_handle(acceptor->loop->reactor, &acceptor->handle, fd);
    if (err != TH_ERR_OK) {
#if defined(TH_CONFIG_OS_POSIX)
        close(fd);
#endif
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
