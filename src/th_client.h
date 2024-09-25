#ifndef TH_CLIENT_H
#define TH_CLIENT_H

#include <th.h>

#include "th_allocator.h"
#include "th_config.h"
#include "th_exchange.h"
#include "th_refcounted.h"
#include "th_request.h"
#include "th_response.h"
#include "th_router.h"
#include "th_ssl_socket.h"
#include "th_tcp_socket.h"

/* th_client interface begin */
typedef struct th_client th_client;
struct th_client {
    th_refcounted base;
    th_socket* (*get_socket)(void* self);
    th_address* (*get_address)(void* self);
    th_err (*start)(void* self);
};

/** th_client_init
 * @brief Initialize the client interface, this function should be called
 * by the parent client implementation on initialization.
 */
TH_INLINE(void)
th_client_init(th_client* client,
               th_socket* (*get_socket)(void* self),
               th_address* (*get_address)(void* self),
               th_err (*start)(void* self),
               void (*destroy)(void* self))
{
    th_refcounted_init(&client->base, destroy);
    client->get_socket = get_socket;
    client->get_address = get_address;
    client->start = start;
}

TH_INLINE(th_socket*)
th_client_get_socket(th_client* client)
{
    return client->get_socket(client);
}

TH_INLINE(th_address*)
th_client_get_address(th_client* client)
{
    return client->get_address(client);
}

TH_INLINE(th_err)
th_client_start(th_client* client)
{
    return client->start(client);
}

TH_INLINE(th_client*)
th_client_ref(th_client* client) TH_MAYBE_UNUSED;

TH_INLINE(th_client*)
th_client_ref(th_client* client)
{
    return (th_client*)th_refcounted_ref(&client->base);
}

TH_INLINE(void)
th_client_unref(th_client* client)
{
    th_refcounted_unref(&client->base);
}

/* th_client interface end */
/* th_client_observable interface begin */

/** th_client_observer
 * @brief Implement this interface to observe when a client is
 * initialized and destroyed.
 */
typedef struct th_client_observable th_client_observable;

typedef struct th_client_observer th_client_observer;
struct th_client_observer {
    void (*on_init)(th_client_observer* self, th_client_observable* observable);
    void (*on_deinit)(th_client_observer* self, th_client_observable* observable);
};

TH_INLINE(void)
th_client_observer_on_init(th_client_observer* observer, th_client_observable* observable)
{
    observer->on_init(observer, observable);
}

TH_INLINE(void)
th_client_observer_on_deinit(th_client_observer* observer, th_client_observable* observable)
{
    observer->on_deinit(observer, observable);
}

struct th_client_observable {
    th_client base;
    void (*destroy)(void* self);
    th_client_observer* observer;
    th_client_observable *next, *prev;
};

/* th_client_observable interface end */
/* th_tcp_client declaration begin */

typedef struct th_tcp_client th_tcp_client;
typedef struct th_tcp_client_msg_exchange_handler {
    th_io_handler base;
    th_tcp_client* client;
} th_tcp_client_msg_exchange_handler;

struct th_tcp_client {
    th_client_observable base;
    th_tcp_socket socket;
    th_address addr;
    th_context* context;
    th_allocator* allocator;
    th_router* router;
    th_fcache* fcache;
    th_tcp_client_msg_exchange_handler msg_exchange_handler;
};

TH_PRIVATE(th_err)
th_tcp_client_create(th_client** out, th_context* context,
                     th_router* router, th_fcache* fcache,
                     th_client_observer* observer,
                     th_allocator* allocator);

/* th_tcp_client declaration end */
/* th_ssl_client declaration begin */
#if TH_WITH_SSL
typedef struct th_ssl_client th_ssl_client;
typedef struct th_ssl_client_io_handler {
    th_io_handler base;
    th_ssl_client* client;
} th_ssl_client_io_handler;

struct th_ssl_client {
    th_client_observable base;
    th_ssl_socket socket;
    th_address addr;
    th_context* context;
    th_allocator* allocator;
    th_router* router;
    th_fcache* fcache;
    th_ssl_client_io_handler msg_exchange_handler;
    th_ssl_client_io_handler handshake_handler;
    th_ssl_client_io_handler shutdown_handler;
};

TH_PRIVATE(th_err)
th_ssl_client_create(th_client** out, th_context* context, th_ssl_context* ssl_context,
                     th_router* router, th_fcache* fcache, th_client_observer* observer,
                     th_allocator* allocator);
#endif
#endif
