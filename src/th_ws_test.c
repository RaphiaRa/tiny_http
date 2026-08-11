#include "th_system_error.h"
#include "th_test.h"
#include "th_ws.h"

typedef struct th_fake_conn {
    th_conn base;
    bool destroyed;
    void (*callback)(void* user_data, size_t size, th_err err);
    void* user_data;
    th_err next_recv_err;
} th_fake_conn;

static th_address*
th_fake_conn_get_address(void* self)
{
    (void)self;
    return NULL;
}

static void
th_fake_conn_start(void* self)
{
    (void)self;
}

static void
th_fake_conn_recv(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    (void)addr;
    (void)len;
    (void)exact;
    th_fake_conn* conn = self;
    TH_ASSERT(conn->callback == NULL);
    conn->callback = callback;
    conn->user_data = user_data;
}

static void
th_fake_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    (void)self;
    (void)iov;
    (void)iovcnt;
    (void)file;
    (void)offset;
    (void)len;
    (void)callback;
    (void)user_data;
    TH_ASSERT(0 && "not expected to be called in this slice");
}

static void
th_fake_conn_cancel(void* self)
{
    (void)self;
}

static void
th_fake_conn_destroy(void* self)
{
    th_fake_conn* conn = self;
    conn->destroyed = true;
}

static const th_conn_methods th_fake_conn_methods = {
    .get_address = th_fake_conn_get_address,
    .start = th_fake_conn_start,
    .recv = th_fake_conn_recv,
    .send = th_fake_conn_send,
    .cancel = th_fake_conn_cancel,
    .destroy = th_fake_conn_destroy,
};

static void
th_fake_conn_init(th_fake_conn* conn)
{
    conn->base.methods = &th_fake_conn_methods;
    conn->destroyed = false;
    conn->callback = NULL;
    conn->user_data = NULL;
    conn->next_recv_err = TH_ERR_EOF;
}

static void
th_fake_conn_run(th_fake_conn* conn)
{
    TH_ASSERT(conn->callback != NULL);
    void (*callback)(void*, size_t, th_err) = conn->callback;
    void* user_data = conn->user_data;
    conn->callback = NULL;
    conn->user_data = NULL;
    callback(user_data, 0, conn->next_recv_err);
}

struct handler_calls {
    int open_count;
    int close_count;
    th_err return_on_open;
};

static th_err
th_test_ws_handler(void* userp, th_ws* ws, th_ws_event ev, th_buffer data)
{
    (void)ws;
    (void)data;
    struct handler_calls* calls = userp;
    switch (ev) {
    case TH_WS_EVENT_OPEN:
        calls->open_count++;
        return calls->return_on_open;
    case TH_WS_EVENT_CLOSE:
        calls->close_count++;
        return TH_ERR_OK;
    default:
        return TH_ERR_OK;
    }
}

TH_TEST_BEGIN(ws)
{
    th_fake_conn conn;
    th_fake_conn_init(&conn);
    struct handler_calls calls = {0, 0, TH_ERR_OK};

    TH_TEST_CASE_BEGIN(ws_start_fires_open)
    {
        th_ws* ws = NULL;
        TH_EXPECT(th_ws_create(&ws, &conn.base, th_test_ws_handler, &calls, NULL) == TH_ERR_OK);
        th_ws_start(ws);
        TH_EXPECT(calls.open_count == 1);
        TH_EXPECT(calls.close_count == 0);
        TH_EXPECT(!conn.destroyed);

        conn.next_recv_err = TH_ERR_EOF;
        th_fake_conn_run(&conn);
        TH_EXPECT(calls.close_count == 1);
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_open_error_closes_without_recv)
    {
        calls.return_on_open = TH_ERR_INVALID_ARG;
        th_ws* ws = NULL;
        TH_EXPECT(th_ws_create(&ws, &conn.base, th_test_ws_handler, &calls, NULL) == TH_ERR_OK);
        th_ws_start(ws);
        TH_EXPECT(calls.open_count == 1);
        TH_EXPECT(calls.close_count == 0); // CLOSE isn't fired for a handler-rejected OPEN
        TH_EXPECT(conn.destroyed);
        TH_EXPECT(conn.callback == NULL); // never issued a recv
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_recv_error_fires_close_and_destroys)
    {
        th_ws* ws = NULL;
        TH_EXPECT(th_ws_create(&ws, &conn.base, th_test_ws_handler, &calls, NULL) == TH_ERR_OK);
        th_ws_start(ws);

        conn.next_recv_err = TH_ERR_SYSTEM(TH_EIO);
        th_fake_conn_run(&conn);
        TH_EXPECT(calls.close_count == 1);
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_recv_success_keeps_reading)
    {
        th_ws* ws = NULL;
        TH_EXPECT(th_ws_create(&ws, &conn.base, th_test_ws_handler, &calls, NULL) == TH_ERR_OK);
        th_ws_start(ws);

        conn.next_recv_err = TH_ERR_OK;
        th_fake_conn_run(&conn); // successful recv, discarded, issues another recv
        TH_EXPECT(calls.close_count == 0);
        TH_EXPECT(!conn.destroyed);
        TH_EXPECT(conn.callback != NULL);

        conn.next_recv_err = TH_ERR_EOF;
        th_fake_conn_run(&conn);
        TH_EXPECT(calls.close_count == 1);
        TH_EXPECT(conn.destroyed);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_send_returns_nosupport)
    {
        th_ws* ws = NULL;
        TH_EXPECT(th_ws_create(&ws, &conn.base, th_test_ws_handler, &calls, NULL) == TH_ERR_OK);
        th_ws_start(ws);
        TH_EXPECT(th_ws_send(ws, (th_buffer){"hi", 2}, TH_WS_MSG_TEXT) == TH_ERR_NOSUPPORT);

        conn.next_recv_err = TH_ERR_EOF;
        th_fake_conn_run(&conn);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_close_returns_nosupport)
    {
        th_ws* ws = NULL;
        TH_EXPECT(th_ws_create(&ws, &conn.base, th_test_ws_handler, &calls, NULL) == TH_ERR_OK);
        th_ws_start(ws);
        TH_EXPECT(th_ws_close(ws) == TH_ERR_NOSUPPORT);

        conn.next_recv_err = TH_ERR_EOF;
        th_fake_conn_run(&conn);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
