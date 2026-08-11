#ifndef TH_WS_FRAME_H
#define TH_WS_FRAME_H

#include <th.h>

#include "th_config.h"

#include <stddef.h>

typedef enum th_ws_frame_type {
    TH_WS_FRAME_TEXT,
    TH_WS_FRAME_BINARY,
    TH_WS_FRAME_PING,
    TH_WS_FRAME_PONG,
    TH_WS_FRAME_CLOSE,
} th_ws_frame_type;

// 2 base bytes + 8 byte extended length (server frames are never masked).
#define TH_WS_FRAME_HEADER_MAX_LEN 10

/** th_ws_frame_header_write
 * @brief Encodes a FIN=1, unmasked frame header for len bytes of payload.
 * @return Bytes written to header (2, 4, or 10).
 */
TH_PRIVATE(size_t)
th_ws_frame_header_write(unsigned char* header, th_ws_frame_type type, size_t len);

#endif
