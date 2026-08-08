#ifndef TH_TCP_CONN_H
#define TH_TCP_CONN_H

#include <th.h>

#include "th_conn.h"
#include "th_loop.h"
#include "th_socket.h"

/** th_tcp_conn_create
 * @brief Allocates and initializes a plain (non-SSL) th_conn, taking
 * ownership of socket by value (the caller's th_socket is moved in, not
 * referenced — construct it with th_socket_init and don't use it again
 * after this call). The returned conn has no fd yet; set one via
 * th_socket_set_fd(th_conn_get_socket(conn), fd) before use.
 */
TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_socket* socket,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

#endif
