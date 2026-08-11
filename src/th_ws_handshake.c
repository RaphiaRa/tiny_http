#include "th_ws_handshake.h"

#include "th_base64.h"
#include "th_sha1.h"

#include <string.h>

// Max length of a base64-encoded 16-byte nonce, per RFC 6455.
#define TH_WS_HANDSHAKE_KEY_MAX_LEN 24
#define TH_WS_HANDSHAKE_GUID_LEN 36
#define TH_WS_HANDSHAKE_GUID TH_STR("258EAFA5-E914-47DA-95CA-C5AB0DC85B11")

TH_LOCAL(bool)
th_ws_connection_has_upgrade_token(th_str value)
{
    size_t pos = 0;
    while (pos <= value.len) {
        size_t comma = th_str_find_first(value, pos, ',');
        size_t end = comma == th_str_npos ? value.len : comma;
        th_str token = th_str_trim(th_str_substr(value, pos, end - pos));
        if (th_str_ieq(token, TH_STR("upgrade")))
            return true;
        if (comma == th_str_npos)
            break;
        pos = comma + 1;
    }
    return false;
}

TH_PRIVATE(bool)
th_ws_is_handshake(th_request* request)
{
    if (request->method != TH_METHOD_GET)
        return false;
    if (!th_str_ieq(th_request_get_header(request, TH_STR("upgrade")), TH_STR("websocket")))
        return false;
    if (!th_ws_connection_has_upgrade_token(th_request_get_header(request, TH_STR("connection"))))
        return false;
    if (th_str_empty(th_request_get_header(request, TH_STR("sec-websocket-key"))))
        return false;
    if (!th_str_eq(th_request_get_header(request, TH_STR("sec-websocket-version")), TH_STR("13")))
        return false;
    return true;
}

TH_PRIVATE(th_err)
th_ws_handshake_accept_key(th_str key, th_string* out)
{
    if (key.len > TH_WS_HANDSHAKE_KEY_MAX_LEN)
        return TH_ERR_INVALID_ARG;
    th_str guid = TH_WS_HANDSHAKE_GUID;
    char concat[TH_WS_HANDSHAKE_KEY_MAX_LEN + TH_WS_HANDSHAKE_GUID_LEN];
    memcpy(concat, key.ptr, key.len);
    memcpy(concat + key.len, guid.ptr, guid.len);

    unsigned char digest[TH_SHA1_DIGEST_LEN];
    th_sha1((th_buffer){concat, key.len + guid.len}, digest);

    return th_base64_encode(th_str_make((const char*)digest, TH_SHA1_DIGEST_LEN), out);
}
