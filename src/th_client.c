#include "th_client.h"

#include <th.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "th_allocator.h"
#include "th_log.h"
#include "th_router.h"
#include "th_socket.h"

/* th_client_observable begin */

TH_LOCAL(void)
th_client_observable_destroy(void* self)
{
    th_client_observable* observable = self;
    th_client_observer_on_deinit(observable->observer, observable);
    observable->destroy(observable);
}

TH_LOCAL(void)
th_client_observable_init(th_client_observable* observable,
                          th_socket* (*get_socket)(void* self),
                          th_address* (*get_address)(void* self),
                          th_err (*start)(void* self),
                          void (*set_mode)(void* self, th_exchange_mode mode),
                          void (*destroy)(void* self),
                          th_client_observer* observer)
{
    th_client_init(&observable->base, get_socket, get_address, start, set_mode, th_client_observable_destroy);
    th_client_observer_on_init(observer, observable);
    observable->destroy = destroy;
    observable->observer = observer;
}

/* th_client_observable end */
/* th_tcp_client begin */

#undef TH_LOG_TAG
#define TH_LOG_TAG "tcp_client"

TH_LOCAL(th_socket*)
th_tcp_client_get_socket(void* self);

TH_LOCAL(th_address*)
th_tcp_client_get_address(void* self);

TH_LOCAL(th_err)
th_tcp_client_start(void* self);

TH_LOCAL(void)
th_tcp_client_set_mode(void* self, th_exchange_mode mode);

TH_LOCAL(th_err)
th_tcp_client_exchange_next_msg(th_tcp_client* client);

TH_LOCAL(void)
th_tcp_client_destroy(void* client);

TH_LOCAL(th_tcp_client*)
th_tcp_client_ref(th_tcp_client* client);

TH_LOCAL(void)
th_tcp_client_unref(th_tcp_client* client);

TH_LOCAL(void)
th_tcp_client_msg_exchange_handler_fn(void* self, size_t close, th_err err)
{
    th_tcp_client_msg_exchange_handler* handler = self;
    th_tcp_client* client = handler->client;
    if (err != TH_ERR_OK && err != TH_ERR_EOF) {
        TH_LOG_ERROR("%p: %s", client, th_strerror(err));
        return;
    }
    if (err == TH_ERR_EOF) {
        TH_LOG_DEBUG("%p: Connection closed", client);
        return;
    }
    if (close) {
        TH_LOG_DEBUG("%p: Closing", client);
    } else {
        if ((err = th_tcp_client_exchange_next_msg(th_tcp_client_ref(client))) != TH_ERR_OK) {
            TH_LOG_ERROR("%p: Failed to initiate processing of next message: %s", client, th_strerror(err));
        }
    }
}

TH_LOCAL(void)
th_tcp_client_msg_exchange_handler_detroy(void* self)
{
    th_tcp_client_msg_exchange_handler* handler = self;
    th_tcp_client* client = handler->client;
    th_tcp_client_unref(client);
}

TH_LOCAL(void)
th_tcp_client_msg_exchange_handler_init(th_tcp_client_msg_exchange_handler* handler, th_tcp_client* client)
{
    th_io_handler_init(&handler->base, th_tcp_client_msg_exchange_handler_fn, th_tcp_client_msg_exchange_handler_detroy);
    handler->client = client;
}

TH_LOCAL(void)
th_tcp_client_init(th_tcp_client* client, th_context* context,
                   th_router* router, th_fcache* fcache,
                   th_client_observer* observer,
                   th_allocator* allocator)
{
    th_client_observable_init(&client->base, th_tcp_client_get_socket, th_tcp_client_get_address,
                              th_tcp_client_start, th_tcp_client_set_mode, th_tcp_client_destroy, observer);
    th_tcp_client_msg_exchange_handler_init(&client->msg_exchange_handler, client);
    client->context = context;
    client->allocator = allocator ? allocator : th_default_allocator_get();
    client->router = router;
    client->fcache = fcache;
    th_tcp_socket_init(&client->socket, context, client->allocator);
    th_address_init(&client->addr);
    client->mode = TH_EXCHANGE_MODE_NORMAL;
}

TH_PRIVATE(th_err)
th_tcp_client_create(th_client** out, th_context* context,
                     th_router* router, th_fcache* fcache,
                     th_client_observer* observer,
                     th_allocator* allocator)
{
    th_tcp_client* client = th_allocator_alloc(allocator, sizeof(th_tcp_client));
    if (!client)
        return TH_ERR_BAD_ALLOC;
    th_tcp_client_init(client, context, router, fcache, observer, allocator);
    *out = (th_client*)client;
    return TH_ERR_OK;
}

TH_LOCAL(th_socket*)
th_tcp_client_get_socket(void* self)
{
    th_tcp_client* client = (th_tcp_client*)self;
    return &client->socket.base;
}

TH_LOCAL(th_address*)
th_tcp_client_get_address(void* self)
{
    th_tcp_client* client = (th_tcp_client*)self;
    return &client->addr;
}

TH_LOCAL(th_err)
th_tcp_client_start(void* self)
{
    th_tcp_client* client = (th_tcp_client*)self;
    TH_LOG_TRACE("%p: Starting", client);
    return th_tcp_client_exchange_next_msg(client);
}

TH_LOCAL(void)
th_tcp_client_set_mode(void* self, th_exchange_mode mode)
{
    th_tcp_client* client = (th_tcp_client*)self;
    client->mode = mode;
}

TH_LOCAL(th_err)
th_tcp_client_exchange_next_msg(th_tcp_client* client)
{
    th_exchange* exchange = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_exchange_create(&exchange, th_tcp_client_get_socket(client),
                                  client->router, client->fcache,
                                  client->allocator, &client->msg_exchange_handler.base))
        != TH_ERR_OK) {
        return err;
    }
    th_exchange_start(exchange, client->mode);
    return err;
}

TH_LOCAL(void)
th_tcp_client_destroy(void* self)
{
    th_tcp_client* client = self;
    TH_LOG_TRACE("%p: Destroying", client);
    th_tcp_socket_deinit(&client->socket);
    th_allocator_free(client->allocator, client);
}

TH_LOCAL(th_tcp_client*)
th_tcp_client_ref(th_tcp_client* client)
{
    return (th_tcp_client*)th_client_ref((th_client*)client);
}

TH_LOCAL(void)
th_tcp_client_unref(th_tcp_client* client)
{
    th_client_unref((th_client*)client);
}

/* th_tcp_client end */
/* th_ssl_client begin */
#if TH_WITH_SSL

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_client"

TH_LOCAL(th_socket*)
th_ssl_client_get_socket(void* self);

TH_LOCAL(th_address*)
th_ssl_client_get_address(void* self);

TH_LOCAL(th_err)
th_ssl_client_start(void* self);

TH_LOCAL(void)
th_ssl_client_set_mode(void* self, th_exchange_mode mode);

TH_LOCAL(th_err)
th_ssl_client_exchange_next_msg(th_ssl_client* client);

TH_LOCAL(void)
th_ssl_client_destroy(void* self);

TH_LOCAL(th_ssl_client*)
th_ssl_client_ref(th_ssl_client* client);

TH_LOCAL(void)
th_ssl_client_unref(th_ssl_client* client);

TH_LOCAL(void)
th_ssl_client_msg_exchange_handler_fn(void* self, size_t close, th_err err)
{
    th_ssl_client_io_handler* handler = self;
    th_ssl_client* client = handler->client;
    if (err != TH_ERR_OK && err != TH_ERR_EOF) {
        TH_LOG_DEBUG("%p: %s", client, th_strerror(err));
        return;
    }
    if (err == TH_ERR_EOF) {
        TH_LOG_DEBUG("%p: Connection closed", client);
        return;
    }
    if (close) {
        TH_LOG_DEBUG("%p: Closing", client);
    } else {
        if ((err = th_ssl_client_exchange_next_msg(th_ssl_client_ref(client))) != TH_ERR_OK) {
            TH_LOG_ERROR("%p: Failed to initiate processing of next message: %s", client, th_strerror(err));
        }
    }
}

TH_LOCAL(void)
th_ssl_client_msg_exchange_handler_detroy(void* self)
{
    th_ssl_client_io_handler* handler = self;
    th_ssl_client* client = handler->client;
    th_ssl_client_unref(client);
}

TH_LOCAL(void)
th_ssl_client_handshake_handler_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_ssl_client_io_handler* handler = self;
    th_ssl_client* client = handler->client;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p Handshake error: %s", client, th_strerror(err));
        return;
    }
    TH_LOG_TRACE("%p Handshake complete", client);
    if ((err = th_ssl_client_exchange_next_msg((th_ssl_client*)th_client_ref((th_client*)client))) != TH_ERR_OK) {
        TH_LOG_ERROR("Object; %p Failed to initiate processing of next message: %s", client, th_strerror(err));
    }
}

TH_LOCAL(void)
th_ssl_client_handshake_handler_detroy(void* self)
{
    th_ssl_client_io_handler* handler = self;
    th_ssl_client* client = handler->client;
    th_client_unref((th_client*)client);
}

TH_LOCAL(void)
th_ssl_client_shutdown_handler_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_ssl_client_io_handler* handler = self;
    th_ssl_client* client = handler->client;
    (void)client;
    // Whatever the result, we should finish the client
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p Shutdown error: %s", client, th_strerror(err));
        return;
    }
    TH_LOG_DEBUG("%p Shutdown complete", client);
}

TH_LOCAL(void)
th_ssl_client_shutdown_handler_detroy(void* self)
{
    th_ssl_client_io_handler* handler = self;
    th_ssl_client* client = handler->client;
    th_client_unref((th_client*)client);
}

TH_LOCAL(void)
th_ssl_client_io_handler_init(th_ssl_client_io_handler* handler, th_ssl_client* client,
                              void (*fn)(void* self, size_t len, th_err err), void (*destroy)(void* self))
{
    th_io_handler_init(&handler->base, fn, destroy);
    handler->client = client;
}

TH_LOCAL(void)
th_ssl_client_init(th_ssl_client* client, th_context* context, th_ssl_context* ssl_context,
                   th_router* router, th_fcache* fcache,
                   th_client_observer* observer,
                   th_allocator* allocator)
{
    th_client_observable_init(&client->base, th_ssl_client_get_socket, th_ssl_client_get_address,
                              th_ssl_client_start, th_ssl_client_set_mode, th_ssl_client_destroy, observer);
    th_ssl_client_io_handler_init(&client->msg_exchange_handler, client,
                                  th_ssl_client_msg_exchange_handler_fn, th_ssl_client_msg_exchange_handler_detroy);
    th_ssl_client_io_handler_init(&client->handshake_handler, client,
                                  th_ssl_client_handshake_handler_fn, th_ssl_client_handshake_handler_detroy);
    th_ssl_client_io_handler_init(&client->shutdown_handler, client,
                                  th_ssl_client_shutdown_handler_fn, th_ssl_client_shutdown_handler_detroy);
    client->context = context;
    client->allocator = allocator;
    client->router = router;
    client->fcache = fcache;
    th_ssl_socket_init(&client->socket, context, ssl_context, client->allocator);
    th_address_init(&client->addr);
    client->mode = TH_EXCHANGE_MODE_NORMAL;
}

TH_PRIVATE(th_err)
th_ssl_client_create(th_client** out, th_context* context, th_ssl_context* ssl_context,
                     th_router* router, th_fcache* fcache,
                     th_client_observer* observer,
                     th_allocator* allocator)
{
    th_ssl_client* client = th_allocator_alloc(allocator, sizeof(th_ssl_client));
    if (!client)
        return TH_ERR_BAD_ALLOC;
    th_ssl_client_init(client, context, ssl_context, router, fcache, observer, allocator);
    *out = (th_client*)client;
    return TH_ERR_OK;
}

TH_LOCAL(th_socket*)
th_ssl_client_get_socket(void* self)
{
    th_ssl_client* client = (th_ssl_client*)self;
    return (th_socket*)&client->socket;
}

TH_LOCAL(th_address*)
th_ssl_client_get_address(void* self)
{
    th_ssl_client* client = (th_ssl_client*)self;
    return &client->addr;
}

TH_LOCAL(void)
th_ssl_client_start_handshake(th_ssl_client* client)
{
    th_ssl_socket_async_handshake(&client->socket, &client->handshake_handler.base);
}

TH_LOCAL(th_err)
th_ssl_client_start(void* self)
{
    th_ssl_client* client = (th_ssl_client*)self;
    TH_LOG_TRACE("%p: Starting", client);
    th_ssl_socket_set_mode(&client->socket, TH_SSL_SOCKET_MODE_SERVER);
    th_ssl_client_start_handshake(client);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_client_set_mode(void* self, th_exchange_mode mode)
{
    th_ssl_client* client = (th_ssl_client*)self;
    client->mode = mode;
}

TH_LOCAL(th_err)
th_ssl_client_exchange_next_msg(th_ssl_client* client)
{
    th_exchange* exchange = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_exchange_create(&exchange, th_ssl_client_get_socket(client),
                                  client->router, client->fcache,
                                  client->allocator, &client->msg_exchange_handler.base))
        != TH_ERR_OK) {
        return err;
    }
    th_exchange_start(exchange, client->mode);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_client_destroy(void* self)
{
    th_ssl_client* client = self;
    TH_LOG_TRACE("%p Destroying", client);
    th_ssl_socket_deinit(&client->socket);
    th_allocator_free(client->allocator, client);
}

TH_LOCAL(th_ssl_client*)
th_ssl_client_ref(th_ssl_client* client)
{
    return (th_ssl_client*)th_client_ref((th_client*)client);
}

TH_LOCAL(void)
th_ssl_client_unref(th_ssl_client* client)
{
    th_client_unref((th_client*)client);
}

#endif /* TH_WITH_SSL */
/* th_ssl_client end */
