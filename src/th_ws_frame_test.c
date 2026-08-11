#include "th_test.h"
#include "th_ws_frame.h"

#include <string.h>

TH_TEST_BEGIN(ws_frame)
{
    TH_TEST_CASE_BEGIN(ws_frame_header_write_text_small_payload)
    {
        unsigned char header[TH_WS_FRAME_HEADER_MAX_LEN];
        size_t n = th_ws_frame_header_write(header, TH_WS_FRAME_TEXT, 5);
        static const unsigned char expected[] = {0x81, 0x05};
        TH_EXPECT(n == sizeof(expected));
        TH_EXPECT(memcmp(header, expected, sizeof(expected)) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_frame_header_write_binary_16bit_length)
    {
        unsigned char header[TH_WS_FRAME_HEADER_MAX_LEN];
        size_t n = th_ws_frame_header_write(header, TH_WS_FRAME_BINARY, 300);
        static const unsigned char expected[] = {0x82, 126, 0x01, 0x2c};
        TH_EXPECT(n == sizeof(expected));
        TH_EXPECT(memcmp(header, expected, sizeof(expected)) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_frame_header_write_64bit_length)
    {
        unsigned char header[TH_WS_FRAME_HEADER_MAX_LEN];
        size_t n = th_ws_frame_header_write(header, TH_WS_FRAME_BINARY, 70000);
        static const unsigned char expected[] = {0x82, 127, 0, 0, 0, 0, 0, 1, 0x11, 0x70};
        TH_EXPECT(n == sizeof(expected));
        TH_EXPECT(memcmp(header, expected, sizeof(expected)) == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(ws_frame_header_write_close_ping_pong_opcodes)
    {
        unsigned char header[TH_WS_FRAME_HEADER_MAX_LEN];
        TH_EXPECT(th_ws_frame_header_write(header, TH_WS_FRAME_CLOSE, 0) == 2);
        TH_EXPECT(header[0] == 0x88);
        TH_EXPECT(th_ws_frame_header_write(header, TH_WS_FRAME_PING, 0) == 2);
        TH_EXPECT(header[0] == 0x89);
        TH_EXPECT(th_ws_frame_header_write(header, TH_WS_FRAME_PONG, 0) == 2);
        TH_EXPECT(header[0] == 0x8a);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
