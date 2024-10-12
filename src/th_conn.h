#ifndef TH_CONN_H
#define TH_CONN_H

#include <th.h>

#include "th_allocator.h"
#include "th_config.h"
#include "th_request.h"
#include "th_response.h"
#include "th_router.h"
#include "th_ssl_socket.h"
#include "th_tcp_socket.h"

/* th_prot interface begin */

/* th_prot interface end */
/* th_conn interface begin */
typedef struct th_conn th_conn;
struct th_conn {
    th_socket* (*get_socket)(void* self);
    th_address* (*get_address)(void* self);
    void (*start)(void* self);
    void (*destroy)(void* self);
};

/** th_conn_init
 * @brief Initialize the client interface, this function should be called
 * by the parent client implementation on initialization.
 */
TH_INLINE(void)
th_conn_init(th_conn* client,
             th_socket* (*get_socket)(void* self),
             th_address* (*get_address)(void* self),
             void (*start)(void* self),
             void (*destroy)(void* self))
{
    client->get_socket = get_socket;
    client->get_address = get_address;
    client->start = start;
    client->destroy = destroy;
}

TH_INLINE(th_socket*)
th_conn_get_socket(th_conn* client)
{
    return client->get_socket(client);
}

TH_INLINE(th_address*)
th_conn_get_address(th_conn* client)
{
    return client->get_address(client);
}

TH_INLINE(void)
th_conn_start(th_conn* client)
{
    client->start(client);
}

TH_INLINE(void)
th_conn_destroy(th_conn* client)
{
    client->destroy(client);
}

/* th_conn interface end */
/* th_conn_upgrader interface begin */

/** th_conn_upgrader
 * @brief Implement this interface and pass it to `th_conn` to define
 * how a connection should be upgraded to a higher level protocol.
 */
typedef struct th_conn_upgrader {
    void (*upgrade)(void* self, th_conn* conn);
} th_conn_upgrader;

TH_INLINE(void)
th_conn_upgrader_init(th_conn_upgrader* upgrader, void (*upgrade)(void* self, th_conn* conn))
{
    upgrader->upgrade = upgrade;
}

TH_INLINE(void)
th_conn_upgrader_upgrade(th_conn_upgrader* upgrader, th_conn* conn)
{
    upgrader->upgrade(upgrader, conn);
}

/* th_conn_upgrader interface end */
/* th_conn_observable interface begin */

/** th_conn_observer
 * @brief Implement this interface to observe when a client is
 * initialized and destroyed.
 */
typedef struct th_conn_observable th_conn_observable;

typedef struct th_conn_observer th_conn_observer;
struct th_conn_observer {
    void (*on_init)(th_conn_observer* self, th_conn_observable* observable);
    void (*on_deinit)(th_conn_observer* self, th_conn_observable* observable);
};

TH_INLINE(void)
th_conn_observer_on_init(th_conn_observer* observer, th_conn_observable* observable)
{
    observer->on_init(observer, observable);
}

TH_INLINE(void)
th_conn_observer_on_deinit(th_conn_observer* observer, th_conn_observable* observable)
{
    observer->on_deinit(observer, observable);
}

struct th_conn_observable {
    th_conn base;
    void (*destroy)(void* self);
    th_conn_observer* observer;
    th_conn_observable *next, *prev;
};

/* th_conn_observable interface end */
/* th_tcp_conn declaration begin */

typedef struct th_tcp_conn th_tcp_conn;

struct th_tcp_conn {
    th_conn_observable base;
    th_tcp_socket socket;
    th_address addr;
    th_context* context;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_context* context,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

/* th_tcp_conn declaration end */
/* th_ssl_conn declaration begin */
#if TH_WITH_SSL
typedef struct th_ssl_conn th_ssl_conn;
typedef struct th_ssl_conn_io_handler {
    th_io_handler base;
    th_ssl_conn* conn;
} th_ssl_conn_io_handler;

struct th_ssl_conn {
    th_conn_observable base;
    th_ssl_conn_io_handler handshake_handler;
    th_ssl_conn_io_handler shutdown_handler;
    th_ssl_socket socket;
    th_address addr;
    th_context* context;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_context* context, th_ssl_context* ssl_context,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);
#endif
#endif
