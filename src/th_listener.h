#ifndef TH_LISTENER_H
#define TH_LISTENER_H

#include "th_acceptor.h"
#include "th_client.h"
#include "th_client_acceptor.h"
#include "th_fcache.h"
#include "th_heap_string.h"
#include "th_io_service.h"
#include "th_router.h"
#include "th_socket.h"

typedef struct th_listener th_listener;
struct th_listener {
    th_acceptor acceptor;
    th_client_acceptor client_acceptor;
    th_router* router;
    th_fcache* fcache;
    th_listener* next;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_context* context,
                   const char* host, const char* port,
                   th_router* router, th_fcache* fcache,
                   th_listener_opt* opt, th_allocator* allocator);

TH_PRIVATE(th_err)
th_listener_start(th_listener* listener);

TH_PRIVATE(void)
th_listener_stop(th_listener* listener);

TH_PRIVATE(void)
th_listener_destroy(th_listener* listener);

#endif
