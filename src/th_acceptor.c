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
#elif defined(TH_CONFIG_OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(TH_CONFIG_OS_MOCK)
#include "th_mock_syscall.h"
#endif

TH_PRIVATE(th_err)
th_acceptor_init(th_acceptor* acceptor, th_context* context,
                 th_allocator* allocator,
                 const char* addr, const char* port)
{
    acceptor->handle = NULL;
    acceptor->context = context;
    acceptor->allocator = allocator;
#if defined(TH_CONFIG_OS_POSIX)
    th_err err = TH_ERR_OK;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* res = NULL;
    if (getaddrinfo(addr, port, &hints, &res) != 0) {
        return TH_ERR_SYSTEM(errno);
    }
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
    // Set the socket to non-blocking mode
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if (listen(fd, 1024) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if ((err = th_context_create_handle(context, &acceptor->handle, fd)) != TH_ERR_OK)
        goto cleanup_fd;
    freeaddrinfo(res);
    return TH_ERR_OK;
cleanup_fd:
    close(fd);
cleanup_addrinfo:
    freeaddrinfo(res);
    return err;
#elif defined(TH_CONFIG_OS_WIN)
    th_err err = TH_ERR_OK;
    const ADDRINFOA hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    ADDRINFOA* res = NULL;
    if (getaddrinfo(addr, port, &hints, &res) != 0) {
        return TH_ERR_SYSTEM(WSAGetLastError());
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == INVALID_SOCKET) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_addrinfo;
    }
#if TH_CONFIG_REUSE_ADDR
    {
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
            err = TH_ERR_SYSTEM(WSAGetLastError());
            goto cleanup_fd;
        }
    }
#endif
#if TH_CONFIG_REUSE_PORT
    {
        TH_LOG_FATAL("SO_REUSEPORT is not supported on this platform");
        err = TH_ERR_NOSUPPORT;
        goto cleanup_fd;
    }
#endif
    // Set the socket to non-blocking mode
    u_long mode = 1;
    if (ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_fd;
    }
    if (bind(fd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_fd;
    }
    if (listen(fd, 1024) == SOCKET_ERROR) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_fd;
    }
    if ((err = th_context_create_handle(context, &acceptor->handle, fd)) != TH_ERR_OK)
        goto cleanup_fd;
    freeaddrinfo(res);
    return TH_ERR_OK;
cleanup_fd:
    closesocket(fd);
cleanup_addrinfo:
    freeaddrinfo(res);
    return err;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)addr;
    (void)port;
    int fd = th_mock_open();
    if (fd < 0)
        return TH_ERR_SYSTEM(-fd);
    th_err err = TH_ERR_OK;
    if ((err = th_context_create_handle(context, &acceptor->handle, fd)) != TH_ERR_OK)
        th_mock_close();
    return err;
#endif
}

TH_PRIVATE(void)
th_acceptor_async_accept(th_acceptor* acceptor, th_address* addr, th_io_handler* on_complete)
{
    th_address_init(addr);
    th_io_task* iot = th_io_task_create(acceptor->allocator);
    if (!iot) {
        th_context_dispatch_handler(acceptor->context, on_complete, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_accept(iot, th_io_handle_get_fd(acceptor->handle), &addr->addr, &addr->addrlen, on_complete);
    th_io_handle_submit(acceptor->handle, iot);
}

TH_PRIVATE(void)
th_acceptor_cancel(th_acceptor* acceptor)
{
    th_io_handle_cancel(acceptor->handle);
}

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor)
{
    th_io_handle_destroy(acceptor->handle);
}

/* th_acceptor functions end */
