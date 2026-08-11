#include "th_ws_frame.h"

TH_LOCAL(unsigned char)
th_ws_frame_opcode(th_ws_frame_type type)
{
    switch (type) {
    case TH_WS_FRAME_TEXT:
        return 0x1;
    case TH_WS_FRAME_BINARY:
        return 0x2;
    case TH_WS_FRAME_CLOSE:
        return 0x8;
    case TH_WS_FRAME_PING:
        return 0x9;
    default:
        return 0xA; // TH_WS_FRAME_PONG
    }
}

TH_PRIVATE(size_t)
th_ws_frame_header_write(unsigned char* header, th_ws_frame_type type, size_t len)
{
    header[0] = 0x80 | th_ws_frame_opcode(type); // FIN=1, no fragmentation on send
    if (len < 126) {
        header[1] = (unsigned char)len;
        return 2;
    }
    if (len <= 0xFFFF) {
        header[1] = 126;
        header[2] = (unsigned char)(len >> 8);
        header[3] = (unsigned char)len;
        return 4;
    }
    header[1] = 127;
    for (int i = 0; i < 8; ++i)
        header[2 + i] = (unsigned char)(len >> (8 * (7 - i)));
    return 10;
}
