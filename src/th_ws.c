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
    th_ring_init(&ws->send_ring, ws->allocator, TH_CONFIG_WS_SEND_RING_LEN, TH_CONFIG_WS_SEND_MAX_LEN);
    ws->sending = false;
    ws->closing = false;
}

TH_PRIVATE(void)
th_ws_deinit(th_ws* ws)
{
    th_buf_vec_deinit(&ws->payload);
    th_ring_deinit(&ws->send_ring);
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
    (void)ws->handler(ws->user_data, ws, TH_WS_EVENT_CLOSE, (th_buffer){0}, TH_WS_TEXT);
    th_ws_destroy(ws);
}

TH_LOCAL(void)
th_ws_handle_recv(void* user_data, size_t len, th_err err);

TH_LOCAL(th_err)
th_ws_queue_frame(th_ws* ws, th_ws_frame_type frame_type, th_buffer data);

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

        if (type == TH_WS_FRAME_CLOSE) {
            if (!ws->closing) {
                ws->closing = true;
                th_ws_queue_frame(ws, TH_WS_FRAME_CLOSE, (th_buffer){0});
            }
            return false;
        }
        if (type == TH_WS_FRAME_PING || type == TH_WS_FRAME_PONG)
            continue;

        th_buffer message = {th_buf_vec_begin(&ws->payload), th_buf_vec_size(&ws->payload)};
        th_ws_type msg_type = type == TH_WS_FRAME_TEXT ? TH_WS_TEXT : TH_WS_BINARY;
        err = ws->handler(ws->user_data, ws, TH_WS_EVENT_DATA, message, msg_type);
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
        // if closing, a CLOSE frame is now queued/in flight - the send
        // path destroys once it drains, so as not to cut it off mid-send
        if (!ws->closing)
            th_ws_close_and_destroy(ws);
        return;
    }
    th_conn_recv(ws->conn, ws->scratch, sizeof(ws->scratch), false, th_ws_handle_recv, ws);
}

TH_PRIVATE(void)
th_ws_start(th_ws* ws)
{
    th_err err = ws->handler(ws->user_data, ws, TH_WS_EVENT_OPEN, (th_buffer){0}, TH_WS_TEXT);
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: OPEN handler returned %s, closing", (void*)ws, th_strerror(err));
        th_ws_destroy(ws);
        return;
    }
    th_conn_recv(ws->conn, ws->scratch, sizeof(ws->scratch), false, th_ws_handle_recv, ws);
}

TH_LOCAL(void)
th_ws_handle_send(void* user_data, size_t len, th_err err);

TH_LOCAL(void)
th_ws_send_drain(th_ws* ws)
{
    size_t iovcnt = th_ring_peek(&ws->send_ring, ws->send_iov);
    if (iovcnt == 0) {
        ws->sending = false;
        if (ws->closing)
            th_ws_close_and_destroy(ws);
        return;
    }
    ws->sending = true;
    th_conn_send(ws->conn, ws->send_iov, iovcnt, NULL, 0, 0, th_ws_handle_send, ws);
}

TH_LOCAL(void)
th_ws_handle_send(void* user_data, size_t len, th_err err)
{
    th_ws* ws = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: Send error: %s, closing", (void*)ws, th_strerror(err));
        th_ws_close_and_destroy(ws);
        return;
    }
    th_ring_consume(&ws->send_ring, len);
    th_ws_send_drain(ws);
}

TH_LOCAL(th_err)
th_ws_queue_frame(th_ws* ws, th_ws_frame_type frame_type, th_buffer data)
{
    unsigned char header[TH_WS_FRAME_HEADER_MAX_LEN];
    size_t header_len = th_ws_frame_header_write(header, frame_type, data.len);

    th_iov parts[2] = {
        {header, header_len},
        {(void*)data.ptr, data.len},
    };
    th_err err = th_ring_write(&ws->send_ring, parts, 2);
    if (err != TH_ERR_OK)
        return err;

    if (!ws->sending)
        th_ws_send_drain(ws);
    return TH_ERR_OK;
}

TH_PUBLIC(th_err)
th_ws_send(th_ws* ws, th_buffer data, th_ws_type type)
{
    if (ws->closing)
        return TH_ERR_INVALID_ARG;
    th_ws_frame_type frame_type = type == TH_WS_TEXT ? TH_WS_FRAME_TEXT : TH_WS_FRAME_BINARY;
    return th_ws_queue_frame(ws, frame_type, data);
}

TH_PUBLIC(th_err)
th_ws_close(th_ws* ws)
{
    if (ws->closing)
        return TH_ERR_INVALID_ARG;
    ws->closing = true;
    return th_ws_queue_frame(ws, TH_WS_FRAME_CLOSE, (th_buffer){0});
}
