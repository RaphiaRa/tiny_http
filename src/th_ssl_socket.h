#ifndef TH_SSL_SOCKET_H
#define TH_SSL_SOCKET_H

#include "th_config.h"

#if TH_WITH_SSL

#include "th_socket.h"
#include "th_ssl_context.h"
#include "th_tcp_socket.h"

#include <openssl/ssl.h>

/* th_ssl_socket begin */

typedef struct th_ssl_socket {
    th_socket base;
    th_tcp_socket tcp_socket;
    SSL* ssl;
    BIO* wbio; // ssl output buffer
    BIO* rbio; // ssl input buffer
} th_ssl_socket;

typedef enum th_ssl_socket_mode {
    TH_SSL_SOCKET_MODE_SERVER,
    TH_SSL_SOCKET_MODE_CLIENT
} th_ssl_socket_mode;

TH_PRIVATE(th_err)
th_ssl_socket_init(th_ssl_socket* socket, th_context* context, th_ssl_context* ssl_context, th_allocator* allocator);

/** ssl socket specific functions */

TH_PRIVATE(void)
th_ssl_socket_set_mode(th_ssl_socket* socket, th_ssl_socket_mode mode);

TH_PRIVATE(void)
th_ssl_socket_async_handshake(th_ssl_socket* socket, th_socket_handler* handler);

TH_PRIVATE(void)
th_ssl_socket_async_shutdown(th_ssl_socket* socket, th_socket_handler* handler);

/** th_socket_close
 * @brief Closes the underlying file descriptor of the socket.
 * while the socket object is still valid and can be reused.
 */
TH_PRIVATE(void)
th_ssl_socket_close(th_ssl_socket* socket);

TH_PRIVATE(void)
th_ssl_socket_deinit(th_ssl_socket* socket);

#endif
#endif
