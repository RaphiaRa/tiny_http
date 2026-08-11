#include "th_ws.h"

#include "th_log.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "ws"

TH_PRIVATE(void)
th_ws_init(th_ws* ws, th_conn* conn, th_ws_handler handler, void* user_data, th_allocator* allocator)
{
    ws->conn = conn;
    ws->handler = handler;
    ws->user_data = user_data;
    ws->allocator = allocator ? allocator : th_default_allocator_get();
}

TH_PRIVATE(void)
th_ws_deinit(th_ws* ws)
{
    th_conn_destroy(ws->conn);
}

TH_PRIVATE(th_err)
th_ws_create(th_ws** out, th_conn* conn, th_ws_handler handler, void* user_data, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_ws* ws = th_allocator_alloc(allocator, sizeof(th_ws));
    if (!ws)
        return TH_ERR_BAD_ALLOC;
    th_ws_init(ws, conn, handler, user_data, allocator);
    *out = ws;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ws_destroy(th_ws* ws)
{
    th_allocator* allocator = ws->allocator;
    th_ws_deinit(ws);
    th_allocator_free(allocator, ws);
}

TH_LOCAL(void)
th_ws_handle_recv(void* user_data, size_t len, th_err err)
{
    th_ws* ws = user_data;
    (void)len;
    // Frame parsing isn't implemented yet: any successfully received
    // bytes are discarded, we only care about detecting EOF/errors.
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: Connection closed: %s", (void*)ws, th_strerror(err));
        (void)ws->handler(ws->user_data, ws, TH_WS_EVENT_CLOSE, (th_buffer){0});
        th_ws_destroy(ws);
        return;
    }
    th_conn_recv(ws->conn, ws->scratch, sizeof(ws->scratch), false, th_ws_handle_recv, ws);
}

TH_PRIVATE(void)
th_ws_start(th_ws* ws)
{
    th_err err = ws->handler(ws->user_data, ws, TH_WS_EVENT_OPEN, (th_buffer){0});
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: OPEN handler returned %s, closing", (void*)ws, th_strerror(err));
        th_ws_destroy(ws);
        return;
    }
    th_conn_recv(ws->conn, ws->scratch, sizeof(ws->scratch), false, th_ws_handle_recv, ws);
}

TH_PUBLIC(th_err)
th_ws_send(th_ws* ws, th_buffer data, th_ws_msg_type type)
{
    (void)ws;
    (void)data;
    (void)type;
    TH_LOG_ERROR("WebSocket frame sending is not implemented yet.");
    return TH_ERR_NOSUPPORT;
}

TH_PUBLIC(th_err)
th_ws_close(th_ws* ws)
{
    (void)ws;
    TH_LOG_ERROR("WebSocket close handshake is not implemented yet.");
    return TH_ERR_NOSUPPORT;
}
