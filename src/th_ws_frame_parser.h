#ifndef TH_WS_FRAME_PARSER_H
#define TH_WS_FRAME_PARSER_H

#include <th.h>

#include "th_config.h"
#include "th_vec.h"
#include "th_ws_frame.h"

#include <stdint.h>

typedef enum th_ws_frame_parser_state {
    TH_WS_FRAME_PARSER_STATE_HEADER,
    TH_WS_FRAME_PARSER_STATE_PAYLOAD,
} th_ws_frame_parser_state;

// Largest a header can be: 2 fixed bytes + 8 byte extended length + 4 byte mask key.
#define TH_WS_FRAME_PARSER_HEADER_MAX_LEN 14

typedef struct th_ws_frame_parser {
    th_ws_frame_parser_state state;

    // Header bytes seen so far, for a header split across recv() calls.
    unsigned char header_buf[TH_WS_FRAME_PARSER_HEADER_MAX_LEN];
    size_t header_len;

    // Current frame, once its header is fully parsed.
    bool fin;
    unsigned char opcode;
    unsigned char mask_key[4];
    uint64_t payload_len;
    uint64_t payload_read; // bytes of this frame's payload consumed so far

    unsigned char message_opcode; // opcode of a fragmented message still in progress, 0 if none
} th_ws_frame_parser;

// data must be mutable - payloads are unmasked in place.
// *type is only set when this returns TH_ERR_OK.
//
// - TH_ERR_OK: one full message is in payload (empty for TH_WS_FRAME_CLOSE)
// - TH_ERR_SYSTEM(TH_EAGAIN): need more data; *parsed still reflects bytes
//   consumed so far - keep the remainder and retry once more bytes arrive
// - TH_ERR_SYSTEM(TH_EPROTO): protocol violation
TH_PRIVATE(th_err)
th_ws_frame_parser_parse(th_ws_frame_parser* parser, char* data, size_t len, th_buf_vec* payload, size_t* parsed,
                          th_ws_frame_type* type);

#endif
