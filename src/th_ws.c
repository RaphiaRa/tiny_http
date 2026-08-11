#include "th_ws.h"

#include "th_log.h"
#include "th_system_error.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "ws"

TH_PRIVATE(void)
th_ws_init(th_ws* ws, th_conn* conn, th_ws_handler handler, void* user_data, th_allocator* allocator)
{
    ws->conn = conn;
    ws->handler = handler;
    ws->user_data = user_data;
    ws->allocator = allocator ? allocator : th_default_allocator_get();
    ws->parser = (th_ws_frame_parser){0};
    th_buf_vec_init(&ws->payload, ws->allocator);
}

TH_PRIVATE(void)
th_ws_deinit(th_ws* ws)
{
    th_buf_vec_deinit(&ws->payload);
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
th_ws_close_and_destroy(th_ws* ws)
{
    (void)ws->handler(ws->user_data, ws, TH_WS_EVENT_CLOSE, (th_buffer){0});
    th_ws_destroy(ws);
}

TH_LOCAL(void)
th_ws_handle_recv(void* user_data, size_t len, th_err err);

TH_LOCAL(bool)
th_ws_consume(th_ws* ws, char* data, size_t len)
{
    while (len > 0) {
        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&ws->parser, data, len, &ws->payload, &parsed, &type);
        data += parsed;
        len -= parsed;
        if (err == TH_ERR_SYSTEM(TH_EAGAIN))
            return true;
        if (err != TH_ERR_OK) {
            TH_LOG_DEBUG("%p: Invalid frame: %s", (void*)ws, th_strerror(err));
            return false;
        }

        if (type == TH_WS_FRAME_CLOSE)
            return false;
        if (type == TH_WS_FRAME_PING || type == TH_WS_FRAME_PONG)
            continue;

        th_buffer message = {th_buf_vec_begin(&ws->payload), th_buf_vec_size(&ws->payload)};
        err = ws->handler(ws->user_data, ws, TH_WS_EVENT_DATA, message);
        th_buf_vec_clear(&ws->payload);
        if (err != TH_ERR_OK) {
            TH_LOG_DEBUG("%p: DATA handler returned %s, closing", (void*)ws, th_strerror(err));
            return false;
        }
    }
    return true;
}

TH_LOCAL(void)
th_ws_handle_recv(void* user_data, size_t len, th_err err)
{
    th_ws* ws = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: Connection closed: %s", (void*)ws, th_strerror(err));
        th_ws_close_and_destroy(ws);
        return;
    }
    if (!th_ws_consume(ws, ws->scratch, len)) {
        th_ws_close_and_destroy(ws);
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
