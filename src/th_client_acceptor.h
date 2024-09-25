#ifndef TH_CLIENT_ACCEPTOR_H
#define TH_CLIENT_ACCEPTOR_H

#include "th_acceptor.h"
#include "th_allocator.h"
#include "th_client.h"
#include "th_client_tracker.h"
#include "th_config.h"
#include "th_fcache.h"
#include "th_router.h"
#include "th_ssl_context.h"

typedef struct th_client_acceptor th_client_acceptor;
typedef struct th_client_acceptor_accept_handler {
    th_io_handler base;
    th_client_acceptor* client_acceptor;
} th_client_acceptor_accept_handler;

typedef struct th_client_acceptor_client_destroy_handler {
    th_task base;
    th_client_acceptor* client_acceptor;
} th_client_acceptor_client_destroy_handler;

struct th_client_acceptor {
    th_context* context;

    /** The router that will be associated with the clients. */
    th_router* router;

    /** The file cache that will be used to cache the file objects. */
    th_fcache* fcache;

    /** The acceptor that will accept the incoming connections. */
    th_acceptor* acceptor;

    /** The client that will be used to handle the incoming connections. */
    th_client* client;

    /** Used to keep track of all the clients that are currently active. */
    th_client_tracker client_tracker;

    /** Used to react to the destruction of a client. */
    th_client_acceptor_client_destroy_handler client_destroy_handler;

#if TH_WITH_SSL
    /** Ssl context that will be used to create the ssl socket. */
    th_ssl_context ssl_context;
#endif /* TH_WITH_SSL */

    /** Flag that indicates if ssl is enabled. */
    int ssl_enabled;

    /** The accept handler that will be used to handle the completion
     * of the accept operation.
     */
    th_client_acceptor_accept_handler accept_handler;

    /** As long as the acceptor keeps accepting new connections,
     * this flag will be set to 1.
     */
    int running;

    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_client_acceptor_init(th_client_acceptor* client_acceptor,
                        th_context* context,
                        th_router* router, th_fcache* fcache,
                        th_acceptor* acceptor, th_allocator* allocator);

/** th_client_acceptor_enable_ssl
 * @brief Enable ssl for the client acceptor. This function should be called before
 * starting the client acceptor.
 */
TH_PRIVATE(th_err)
th_client_acceptor_enable_ssl(th_client_acceptor* client_acceptor, const char* key_file, const char* cert_file);

/** th_client_acceptor_start
 * @brief Asynchronously start the client acceptor. The acceptor will keep accepting new connections
 * until th_client_acceptor_stop is called.
 */
TH_PRIVATE(th_err)
th_client_acceptor_start(th_client_acceptor* client_acceptor);

TH_PRIVATE(void)
th_client_acceptor_stop(th_client_acceptor* client_acceptor);

TH_PRIVATE(void)
th_client_acceptor_deinit(th_client_acceptor* client_acceptor);

#endif
