#ifndef TH_WS_H
#define TH_WS_H

#include <th.h>

#include "th_conn.h"
#include "th_iov.h"
#include "th_ring.h"
#include "th_vec.h"
#include "th_ws_frame_parser.h"

#define TH_WS_SCRATCH_RECV_LEN 8192

struct th_ws {
    th_conn* conn;
    th_ws_handler handler;
    void* user_data;
    th_allocator* allocator;
    th_ws_frame_parser parser;
    th_buf_vec payload; // accumulates a message's payload across fragments/calls
    char scratch[TH_WS_SCRATCH_RECV_LEN];

    th_ring send_ring;
    th_iov send_iov[2];
    bool sending;
    bool closing; // a CLOSE frame is queued/in flight - destroy once send_ring drains
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
