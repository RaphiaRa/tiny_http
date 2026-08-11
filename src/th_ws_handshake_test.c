#include "th_request.h"
#include "th_test.h"
#include "th_ws_handshake.h"

#include <string.h>

TH_TEST_BEGIN(ws_handshake)
{
    th_request request;
    th_request_init(&request, th_default_allocator_get());

    TH_TEST_CASE_BEGIN(ws_handshake_accept_key_matches_rfc6455_example)
    {
        th_string out;
        th_string_init(&out, th_default_allocator_get());
        TH_EXPECT(th_ws_handshake_accept_key(TH_STR("dGhlIHNhbXBsZSBub25jZQ=="), &out) == TH_ERR_OK);
        TH_EXPECT(th_str_eq(th_string_view(&out), TH_STR("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=")));
        th_string_deinit(&out);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_handshake_accept_key_rejects_oversized_key)
    {
        char oversized[64];
        memset(oversized, 'a', sizeof(oversized));
        th_string out;
        th_string_init(&out, th_default_allocator_get());
        TH_EXPECT(th_ws_handshake_accept_key(th_str_make(oversized, sizeof(oversized)), &out) == TH_ERR_INVALID_ARG);
        th_string_deinit(&out);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_accepts_valid_request)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("websocket")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("keep-alive, Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_accepts_case_insensitive_upgrade_value)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("WebSocket")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_rejects_non_get_method)
    {
        request.method = TH_METHOD_POST;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("websocket")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(!th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_rejects_missing_upgrade_header)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(!th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_rejects_wrong_upgrade_value)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("h2c")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(!th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_rejects_connection_header_without_upgrade_token)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("websocket")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("keep-alive")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(!th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_rejects_missing_key)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("websocket")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("13")) == TH_ERR_OK);
        TH_EXPECT(!th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_is_handshake_rejects_wrong_version)
    {
        request.method = TH_METHOD_GET;
        TH_EXPECT(th_request_add_header(&request, TH_STR("upgrade"), TH_STR("websocket")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("connection"), TH_STR("Upgrade")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-key"), TH_STR("dGhlIHNhbXBsZSBub25jZQ==")) == TH_ERR_OK);
        TH_EXPECT(th_request_add_header(&request, TH_STR("sec-websocket-version"), TH_STR("8")) == TH_ERR_OK);
        TH_EXPECT(!th_ws_is_handshake(&request));
    }
    TH_TEST_CASE_END

    th_request_deinit(&request);
}
TH_TEST_END
