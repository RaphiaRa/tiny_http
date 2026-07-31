#include <th.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "th_allocator.h"
#include "th_listener.h"
#include "th_log.h"
#include "th_tcp_conn.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "listener"

TH_LOCAL(th_err)
th_listener_enable_ssl(th_listener* listener, const char* key_file, const char* cert_file)
{
#if TH_WITH_SSL
    (void)listener;
    (void)key_file;
    (void)cert_file;
    TH_LOG_ERROR("SSL support is being reworked onto th_conn/th_socket, not available yet.");
    return TH_ERR_NOSUPPORT;
#else
    (void)listener;
    (void)key_file;
    (void)cert_file;
    TH_LOG_ERROR("SSL is not enabled in this build.");
    return TH_ERR_NOSUPPORT;
#endif
}

TH_LOCAL(th_err)
th_listener_init(th_listener* listener, th_loop* loop,
                 const char* host, const char* port,
                 th_router* router, th_fcache* fcache,
                 th_bind_opt* opt, th_allocator* allocator)
{
    listener->loop = loop;
    listener->running = 0;
    listener->allocator = allocator ? allocator : th_default_allocator_get();
    th_err err = TH_ERR_OK;
    th_acceptor_init(&listener->acceptor, loop, th_acceptor_ops_os());
    if ((err = th_acceptor_open(&listener->acceptor, host, port)) != TH_ERR_OK)
        return err;
    if (opt && opt->key_file && opt->cert_file) {
        if ((err = th_listener_enable_ssl(listener, opt->key_file, opt->cert_file)) != TH_ERR_OK)
            goto cleanup_acceptor;
    }
    th_conn_tracker_init(&listener->conn_tracker);
    th_http_upgrader_init(&listener->upgrader, &listener->conn_tracker, router, fcache, allocator);
    TH_LOG_INFO("Created listener on %s:%s", host, port);
    return TH_ERR_OK;
cleanup_acceptor:
    th_acceptor_deinit(&listener->acceptor);
    return err;
}

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_loop* loop,
                   const char* host, const char* port,
                   th_router* router, th_fcache* fcache,
                   th_bind_opt* opt, th_allocator* allocator)
{
    th_listener* listener = th_allocator_alloc(allocator, sizeof(th_listener));
    if (!listener)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_init(listener, loop, host, port, router, fcache, opt, allocator)) != TH_ERR_OK)
        goto cleanup;
    *out = listener;
    return TH_ERR_OK;
cleanup:
    th_allocator_free(allocator, listener);
    return err;
}

TH_LOCAL(void)
th_listener_accept_complete(void* user_data, int fd, th_err err);

TH_LOCAL(th_err)
th_listener_async_accept(th_listener* listener)
{
    th_socket socket;
    th_socket_init(&socket, listener->loop, th_socket_ops_os());
    th_err err = TH_ERR_OK;
    if ((err = th_tcp_conn_create(&listener->conn, &socket,
                                  &listener->upgrader.base,
                                  (th_conn_observer*)&listener->conn_tracker,
                                  listener->allocator))
        != TH_ERR_OK) {
        return err;
    }
    th_accept_op_init(&listener->accept_op, &listener->acceptor, &listener->accept_addr,
                      th_listener_accept_complete, listener);
    th_op_perform(&listener->accept_op.base);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_listener_client_destroy_handler_fn(void* self)
{
    th_listener* listener = self;
    if (!listener->running)
        return;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_async_accept(listener)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_conn_tracker_async_wait(&listener->conn_tracker, &listener->client_destroy_handler.base);
    }
}

TH_LOCAL(void)
th_listener_accept_complete(void* user_data, int fd, th_err err)
{
    th_listener* listener = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("Accept failed: %s", th_strerror(err));
        th_conn_destroy(TH_MOVE_PTR(listener->conn));
    } else {
        th_tcp_conn_set_fd(listener->conn, fd);
        th_conn_start(listener->conn);
    }
    if (!listener->running) {
        return;
    }
    if ((err = th_listener_async_accept(listener)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_conn_tracker_async_wait(&listener->conn_tracker, &listener->client_destroy_handler.base);
    }
}

TH_PRIVATE(th_err)
th_listener_start(th_listener* listener)
{
    // Client destroy handler
    listener->client_destroy_handler.listener = listener;
    th_task_init(&listener->client_destroy_handler.base, th_listener_client_destroy_handler_fn, NULL);
    listener->running = 1;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_async_accept(listener)) != TH_ERR_OK)
        return err;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_listener_stop(th_listener* listener)
{
    listener->running = 0;
    th_acceptor_cancel(&listener->acceptor);
    th_conn_tracker_cancel_all(&listener->conn_tracker);
}

TH_LOCAL(void)
th_listener_deinit(th_listener* listener)
{
    th_acceptor_deinit(&listener->acceptor);
    th_conn_tracker_deinit(&listener->conn_tracker);
}

TH_PRIVATE(void)
th_listener_destroy(th_listener* listener)
{
    th_listener_deinit(listener);
    th_allocator_free(listener->allocator, listener);
}
