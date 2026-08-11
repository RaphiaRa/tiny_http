#ifndef TH_CONN_H
#define TH_CONN_H

#include <th.h>

#include "th_address.h"
#include "th_iov.h"
#include "th_op.h"
#include "th_recv.h"
#include "th_send.h"
#include "th_socket.h"

/* th_conn interface begin */

/** th_conn_methods
 * @brief A connection: an accepted socket plus the send/recv operations
 * needed to shuttle an HTTP request/response over it. th_response/th_http
 * call these directly instead of reaching through to a socket type, so
 * that e.g. th_ssl_conn can do handshake/BIO shuttling internally without
 * callers needing to know the connection is encrypted.
 */
typedef struct th_conn_methods {
    th_address* (*get_address)(void* self);
    th_socket* (*get_socket)(void* self);
    void (*start)(void* self);

    /** recv
     * @brief Reads into addr. If exact is false, completes as soon as
     * any bytes arrive (0 bytes => TH_ERR_EOF); if true, retries until
     * exactly len bytes have been read or an error/EOF occurs.
     */
    void (*recv)(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data);

    /** send
     * @brief Writes iov (mutated in place as buffers are consumed),
     * retrying until every byte has been written or an error occurs.
     * If file is NULL, only iov is sent. If file is non-NULL, iov is
     * sent as a header followed by len bytes of file starting at
     * offset (offset/len are ignored when file is NULL).
     */
    void (*send)(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data);

    void (*cancel)(void* self);
    void (*destroy)(void* self);
} th_conn_methods;

typedef struct th_conn {
    const th_conn_methods* methods;
} th_conn;

TH_INLINE(th_address*)
th_conn_get_address(th_conn* conn)
{
    return conn->methods->get_address(conn);
}

TH_INLINE(th_socket*)
th_conn_get_socket(th_conn* conn)
{
    return conn->methods->get_socket(conn);
}

TH_INLINE(void)
th_conn_start(th_conn* conn)
{
    conn->methods->start(conn);
}

TH_INLINE(void)
th_conn_recv(th_conn* conn, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    conn->methods->recv(conn, addr, len, exact, callback, user_data);
}

TH_INLINE(void)
th_conn_send(th_conn* conn, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    conn->methods->send(conn, iov, iovcnt, file, offset, len, callback, user_data);
}

TH_INLINE(void)
th_conn_cancel(th_conn* conn)
{
    conn->methods->cancel(conn);
}

TH_INLINE(void)
th_conn_destroy(th_conn* conn)
{
    if (conn)
        conn->methods->destroy(conn);
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

TH_PRIVATE(void)
th_conn_observable_init(th_conn_observable* observable, const th_conn_methods* methods,
                        void (*destroy)(void* self), th_conn_observer* observer);

/** th_conn_observable_destroy
 * @brief The destroy every concrete conn type's th_conn_methods table
 * must point at: notifies the observer, then calls the type's real
 * destructor (the destroy passed to th_conn_observable_init).
 */
TH_PRIVATE(void)
th_conn_observable_destroy(void* self);

/* th_conn_observable interface end */

#endif
