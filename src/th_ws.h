#ifndef TH_WS_H
#define TH_WS_H

#include <th.h>

#include "th_conn.h"

// Frame parsing isn't implemented yet; received bytes are discarded here.
#define TH_WS_SCRATCH_RECV_LEN 8192

struct th_ws {
    th_conn* conn;
    th_ws_handler handler;
    void* user_data;
    th_allocator* allocator;
    char scratch[TH_WS_SCRATCH_RECV_LEN];
};

TH_PRIVATE(void)
th_ws_init(th_ws* ws, th_conn* conn, th_ws_handler handler, void* user_data, th_allocator* allocator);

TH_PRIVATE(void)
th_ws_deinit(th_ws* ws);

TH_PRIVATE(th_err)
th_ws_create(th_ws** out, th_conn* conn, th_ws_handler handler, void* user_data, th_allocator* allocator);

/** th_ws_start
 * @brief Fires TH_WS_EVENT_OPEN. If the handler returns an error, the
 * connection is torn down immediately without ever reading a frame.
 */
TH_PRIVATE(void)
th_ws_start(th_ws* ws);

#endif
