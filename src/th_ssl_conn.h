#ifndef TH_SSL_CONN_H
#define TH_SSL_CONN_H

#include "th_config.h"

#if TH_WITH_SSL
#include <th.h>

#include "th_conn.h"
#include "th_loop.h"
#include "th_socket.h"
#include "th_ssl_context.h"
#include "th_ssl_ops.h"

/** th_ssl_conn_create
 * @brief Allocates and initializes an SSL th_conn, taking ownership of
 * socket by value (the caller's th_socket is moved in, not referenced —
 * construct it with th_socket_init and don't use it again after this
 * call). The returned conn has no fd yet; set one via
 * th_socket_set_fd(th_conn_get_socket(conn), fd) before use. The SSL
 * handshake only runs once th_conn_start is called, not at creation.
 */
TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_socket* socket, th_ssl_context* ssl_context, th_ssl_ops* ssl_ops,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

#endif
#endif
