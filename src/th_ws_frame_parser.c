#include "th_ws_frame_parser.h"

#include "th_system_error.h"

#include <string.h>

#define TH_WS_OPCODE_CONTINUATION 0x0
#define TH_WS_OPCODE_TEXT 0x1
#define TH_WS_OPCODE_BINARY 0x2
#define TH_WS_OPCODE_CLOSE 0x8
#define TH_WS_OPCODE_PING 0x9
#define TH_WS_OPCODE_PONG 0xA

TH_LOCAL(bool)
th_ws_opcode_is_control(unsigned char opcode)
{
    return opcode >= TH_WS_OPCODE_CLOSE;
}

TH_LOCAL(size_t)
th_ws_frame_parser_fill_header(th_ws_frame_parser* parser, const char* data, size_t len, size_t needed)
{
    size_t remaining = needed - parser->header_len;
    size_t n = len < remaining ? len : remaining;
    memcpy(parser->header_buf + parser->header_len, data, n);
    parser->header_len += n;
    return n;
}

TH_LOCAL(th_err)
th_ws_frame_parser_validate_base_header(th_ws_frame_parser* parser, size_t* header_len)
{
    unsigned char byte0 = parser->header_buf[0];
    unsigned char byte1 = parser->header_buf[1];
    if ((byte0 & 0x70) != 0) // RSV1-3 must be 0, no extensions negotiated
        return TH_ERR_SYSTEM(TH_EPROTO);
    if ((byte1 & 0x80) == 0) // client frames must be masked
        return TH_ERR_SYSTEM(TH_EPROTO);

    unsigned char opcode = byte0 & 0x0F;
    bool fin = (byte0 & 0x80) != 0;
    unsigned char len7 = byte1 & 0x7F;

    bool known_opcode = opcode == TH_WS_OPCODE_CONTINUATION || opcode == TH_WS_OPCODE_TEXT
        || opcode == TH_WS_OPCODE_BINARY || opcode == TH_WS_OPCODE_CLOSE || opcode == TH_WS_OPCODE_PING
        || opcode == TH_WS_OPCODE_PONG;
    if (!known_opcode)
        return TH_ERR_SYSTEM(TH_EPROTO);
    if (th_ws_opcode_is_control(opcode) && !fin) // control frames can't be fragmented
        return TH_ERR_SYSTEM(TH_EPROTO);
    if (th_ws_opcode_is_control(opcode) && len7 > 125) // RFC 6455 5.5
        return TH_ERR_SYSTEM(TH_EPROTO);
    if (opcode == TH_WS_OPCODE_CONTINUATION && parser->message_opcode == 0) // nothing to continue
        return TH_ERR_SYSTEM(TH_EPROTO);
    if ((opcode == TH_WS_OPCODE_TEXT || opcode == TH_WS_OPCODE_BINARY) && parser->message_opcode != 0)
        return TH_ERR_SYSTEM(TH_EPROTO); // data frame while a fragmented message is still in progress

    size_t ext_len_bytes = len7 == 126 ? 2 : len7 == 127 ? 8 : 0;
    *header_len = 2 + ext_len_bytes + 4;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_ws_frame_parser_finish_header(th_ws_frame_parser* parser, th_buf_vec* payload)
{
    unsigned char byte1 = parser->header_buf[1];
    unsigned char len7 = byte1 & 0x7F;
    size_t ext_len_bytes = len7 == 126 ? 2 : len7 == 127 ? 8 : 0;

    parser->fin = (parser->header_buf[0] & 0x80) != 0;
    parser->opcode = parser->header_buf[0] & 0x0F;

    uint64_t payload_len = len7;
    if (ext_len_bytes > 0) {
        payload_len = 0;
        for (size_t i = 0; i < ext_len_bytes; ++i)
            payload_len = (payload_len << 8) | parser->header_buf[2 + i];
    }
    if (!th_ws_opcode_is_control(parser->opcode) && th_buf_vec_size(payload) + payload_len > TH_CONFIG_WS_MAX_MESSAGE_LEN)
        return TH_ERR_SYSTEM(TH_EPROTO);

    memcpy(parser->mask_key, parser->header_buf + 2 + ext_len_bytes, 4);
    parser->payload_len = payload_len;
    parser->payload_read = 0;
    parser->state = TH_WS_FRAME_PARSER_STATE_PAYLOAD;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_ws_frame_parser_do_header(th_ws_frame_parser* parser, th_buf_vec* payload, const char* data, size_t len,
                             size_t* parsed)
{
    *parsed = th_ws_frame_parser_fill_header(parser, data, len, 2);
    if (parser->header_len < 2)
        return TH_ERR_OK;

    size_t header_len = 0;
    th_err err = th_ws_frame_parser_validate_base_header(parser, &header_len);
    if (err != TH_ERR_OK)
        return err;

    *parsed += th_ws_frame_parser_fill_header(parser, data + *parsed, len - *parsed, header_len);
    if (parser->header_len < header_len)
        return TH_ERR_OK;

    return th_ws_frame_parser_finish_header(parser, payload);
}

TH_LOCAL(th_err)
th_ws_frame_parser_append_payload(th_buf_vec* payload, const unsigned char* data, size_t len)
{
    if (len == 0)
        return TH_ERR_OK;
    size_t start = th_buf_vec_size(payload);
    th_err err = th_buf_vec_resize(payload, start + len);
    if (err != TH_ERR_OK)
        return err;
    memcpy(th_buf_vec_at(payload, start), data, len);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ws_frame_parser_frame_done(th_ws_frame_parser* parser, bool* message_done, th_ws_frame_type* type)
{
    if (th_ws_opcode_is_control(parser->opcode)) {
        *type = parser->opcode == TH_WS_OPCODE_CLOSE ? TH_WS_FRAME_CLOSE
            : parser->opcode == TH_WS_OPCODE_PING    ? TH_WS_FRAME_PING
                                                      : TH_WS_FRAME_PONG;
        *message_done = true;
    } else {
        if (parser->opcode != TH_WS_OPCODE_CONTINUATION)
            parser->message_opcode = parser->opcode;
        if (parser->fin) {
            *type = parser->message_opcode == TH_WS_OPCODE_TEXT ? TH_WS_FRAME_TEXT : TH_WS_FRAME_BINARY;
            parser->message_opcode = 0;
            *message_done = true;
        }
    }
    parser->state = TH_WS_FRAME_PARSER_STATE_HEADER;
    parser->header_len = 0;
}

// mask_key indexing uses payload_read so it stays correct across chunks.
TH_LOCAL(th_err)
th_ws_frame_parser_do_payload(th_ws_frame_parser* parser, char* data, size_t len, th_buf_vec* payload, size_t* parsed,
                              bool* message_done, th_ws_frame_type* type)
{
    uint64_t remaining = parser->payload_len - parser->payload_read;
    size_t n = (uint64_t)len < remaining ? len : (size_t)remaining;
    *parsed = n;

    for (size_t i = 0; i < n; ++i)
        data[i] = (char)((unsigned char)data[i] ^ parser->mask_key[(parser->payload_read + i) % 4]);

    th_err err = TH_ERR_OK;
    if (!th_ws_opcode_is_control(parser->opcode))
        err = th_ws_frame_parser_append_payload(payload, (const unsigned char*)data, n);
    parser->payload_read += n;
    if (err != TH_ERR_OK)
        return err;

    *message_done = false;
    if (parser->payload_read == parser->payload_len)
        th_ws_frame_parser_frame_done(parser, message_done, type);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_ws_frame_parser_parse_next(th_ws_frame_parser* parser, char* data, size_t len, th_buf_vec* payload, size_t* parsed,
                              bool* message_done, th_ws_frame_type* type)
{
    switch (parser->state) {
    case TH_WS_FRAME_PARSER_STATE_HEADER:
        *message_done = false;
        return th_ws_frame_parser_do_header(parser, payload, data, len, parsed);
    case TH_WS_FRAME_PARSER_STATE_PAYLOAD:
        return th_ws_frame_parser_do_payload(parser, data, len, payload, parsed, message_done, type);
    default:
        *parsed = 0;
        *message_done = false;
        return TH_ERR_OK;
    }
}

TH_PRIVATE(th_err)
th_ws_frame_parser_parse(th_ws_frame_parser* parser, char* data, size_t len, th_buf_vec* payload, size_t* parsed,
                          th_ws_frame_type* type)
{
    th_err err = TH_ERR_OK;
    *parsed = 0;
    for (;;) {
        size_t p = 0;
        bool message_done = false;
        if ((err = th_ws_frame_parser_parse_next(parser, data, len, payload, &p, &message_done, type)) != TH_ERR_OK) {
            *parsed += p;
            return err;
        }
        data += p;
        len -= p;
        *parsed += p;
        if (message_done)
            return TH_ERR_OK;
        // check message_done first: a zero-length payload also has p == 0
        if (p == 0 && len == 0)
            return TH_ERR_SYSTEM(TH_EAGAIN);
    }
}
