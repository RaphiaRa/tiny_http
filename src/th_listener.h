#ifndef TH_LISTENER_H
#define TH_LISTENER_H

#include "th_acceptor.h"
#include "th_client.h"
#include "th_client_tracker.h"
#include "th_fcache.h"
#include "th_heap_string.h"
#include "th_io_service.h"
#include "th_router.h"
#include "th_socket.h"

typedef struct th_listener th_listener;

typedef struct th_listener_accept_handler {
    th_io_handler base;
    th_listener* listener;
} th_listener_accept_handler;

typedef struct th_listener_client_destroy_handler {
    th_task base;
    th_listener* listener;
} th_listener_client_destroy_handler;

struct th_listener {
    th_acceptor acceptor;
    th_listener* next;
    th_context* context;

    /** The router that will be associated with the clients. */
    th_router* router;

    /** The file cache that will be used to cache the file objects. */
    th_fcache* fcache;

    /** The client that will be used to handle the incoming connections. */
    th_client* client;

    /** Used to keep track of all the clients that are currently active. */
    th_client_tracker client_tracker;

    /** Used to react to the destruction of a client. */
    th_listener_client_destroy_handler client_destroy_handler;

#if TH_WITH_SSL
    /** Ssl context that will be used to create the ssl socket. */
    th_ssl_context ssl_context;
#endif /* TH_WITH_SSL */

    /** Flag that indicates if ssl is enabled. */
    bool ssl_enabled;

    /** The accept handler that will be used to handle the completion
     * of the accept operation.
     */
    th_listener_accept_handler accept_handler;

    /** As long as the listener keeps accepting new connections,
     * this flag will be set to 1.
     */
    bool running;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_context* context,
                   const char* host, const char* port,
                   th_router* router, th_fcache* fcache,
                   th_bind_opt* opt, th_allocator* allocator);

TH_PRIVATE(th_err)
th_listener_start(th_listener* listener);

TH_PRIVATE(void)
th_listener_stop(th_listener* listener);

TH_PRIVATE(void)
th_listener_destroy(th_listener* listener);

#endif
