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
 * after this call). The returned conn has no fd yet; call th_conn's
 * methods only after th_tcp_conn_set_fd.
 */
TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_socket* socket,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

/** th_tcp_conn_set_fd
 * @brief Registers fd with conn's underlying socket. conn must have been
 * created by th_tcp_conn_create.
 */
TH_PRIVATE(th_err)
th_tcp_conn_set_fd(th_conn* conn, int fd);

#endif
