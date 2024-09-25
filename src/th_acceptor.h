#ifndef TH_ACCEPTOR_H
#define TH_ACCEPTOR_H

#include <th.h>

#include "th_allocator.h"
#include "th_config.h"
#include "th_context.h"
#include "th_socket.h"

typedef struct th_acceptor {
    th_context* context;
    th_allocator* allocator;
    th_io_handle* handle;
} th_acceptor;

typedef struct th_acceptor_opt {
    bool reuse_addr;
    bool reuse_port;
} th_acceptor_opt;

TH_PRIVATE(th_err)
th_acceptor_init(th_acceptor* acceptor, th_context* context,
                 th_allocator* allocator,
                 const char* addr, const char* port);

/** th_acceptor_async_accept
 * @brief Asynchronously accept a new connection. And call the handler when the operation is complete.
 * Both addr and sock must point to valid memory locations until the handler is called.
 * @param acceptor The acceptor that will accept the new connection.
 * @param addr Pointer to the address that will be filled with the address of the new connection.
 * @param sock Pointer to the socket that will be filled with the new connection.
 */
TH_PRIVATE(void)
th_acceptor_async_accept(th_acceptor* acceptor, th_address* addr, th_io_handler* handler);

TH_PRIVATE(void)
th_acceptor_cancel(th_acceptor* acceptor);

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor);

#endif
