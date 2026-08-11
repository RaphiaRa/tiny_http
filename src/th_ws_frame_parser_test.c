#include "th_system_error.h"
#include "th_test.h"
#include "th_ws_frame_parser.h"

#include <string.h>

/* Frame byte layout (RFC 6455 §5.2), all client frames below use mask key
 * 12 34 56 78 (arbitrary, fixed for reproducibility):
 *   byte 0: FIN(1) RSV(3)=0 opcode(4)
 *   byte 1: MASK(1)=1 payload-len(7)  [126/127 => 2/8 byte extended length]
 *   next 4 bytes: mask key
 *   remaining bytes: payload XORed with the mask key, repeating every 4 bytes
 *
 * Each array below was generated with a short throwaway Python script:
 *   MASK = bytes([0x12, 0x34, 0x56, 0x78])
 *   def frame(opcode, payload, fin=True):
 *       b0 = (0x80 if fin else 0) | opcode
 *       n = len(payload)
 *       hdr = bytes([b0, 0x80 | n])  # n < 126 for all frames here
 *       return hdr + MASK + bytes(b ^ MASK[i % 4] for i, b in enumerate(payload))
 */

// text "hello", FIN=1
static const unsigned char FRAME_TEXT_HELLO[] = {0x81, 0x85, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x51, 0x3a, 0x14, 0x7d};

// text "hi", FIN=1
static const unsigned char FRAME_TEXT_HI[] = {0x81, 0x82, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x5d};

// text "foo", FIN=0 (first fragment of a message)
static const unsigned char FRAME_TEXT_FOO_FIN0[] = {0x01, 0x83, 0x12, 0x34, 0x56, 0x78, 0x74, 0x5b, 0x39};

// continuation "bar", FIN=1 (final fragment)
static const unsigned char FRAME_CONT_BAR_FIN1[] = {0x80, 0x83, 0x12, 0x34, 0x56, 0x78, 0x70, 0x55, 0x24};

// ping "ping", FIN=1
static const unsigned char FRAME_PING[] = {0x89, 0x84, 0x12, 0x34, 0x56, 0x78, 0x62, 0x5d, 0x38, 0x1f};

// close, empty payload, FIN=1
static const unsigned char FRAME_CLOSE[] = {0x88, 0x80, 0x12, 0x34, 0x56, 0x78};

static const unsigned char TEST_MASK[4] = {0x12, 0x34, 0x56, 0x78};

TH_TEST_BEGIN(ws_frame_parser)
{
    th_ws_frame_parser parser = {0};
    th_buf_vec payload;
    th_buf_vec_init(&payload, NULL);

    TH_TEST_CASE_BEGIN(ws_frame_parser_single_text_frame)
    {
        unsigned char buf[sizeof(FRAME_TEXT_HELLO)];
        memcpy(buf, FRAME_TEXT_HELLO, sizeof(buf));

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(parsed == sizeof(buf));
        TH_EXPECT(type == TH_WS_FRAME_DATA);
        TH_EXPECT(th_buf_vec_size(&payload) == 5);
        TH_EXPECT(memcmp(th_buf_vec_begin(&payload), "hello", 5) == 0);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_reports_eagain_until_whole_frame_present)
    {
        // Feed one more byte per attempt: each call consumes everything it's
        // given (staged internally) and reports EAGAIN until the frame completes.
        th_err err = TH_ERR_OK;
        for (size_t n = 1; n <= sizeof(FRAME_TEXT_HI); ++n) {
            unsigned char b = FRAME_TEXT_HI[n - 1];
            size_t parsed = 0;
            th_ws_frame_type type;
            err = th_ws_frame_parser_parse(&parser, (char*)&b, 1, &payload, &parsed, &type);
            TH_EXPECT(parsed == 1);
            if (n < sizeof(FRAME_TEXT_HI))
                TH_EXPECT(err == TH_ERR_SYSTEM(TH_EAGAIN));
        }
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(th_buf_vec_size(&payload) == 2);
        TH_EXPECT(memcmp(th_buf_vec_begin(&payload), "hi", 2) == 0);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_reassembles_fragmented_message)
    {
        unsigned char buf[sizeof(FRAME_TEXT_FOO_FIN0) + sizeof(FRAME_CONT_BAR_FIN1)];
        memcpy(buf, FRAME_TEXT_FOO_FIN0, sizeof(FRAME_TEXT_FOO_FIN0));
        memcpy(buf + sizeof(FRAME_TEXT_FOO_FIN0), FRAME_CONT_BAR_FIN1, sizeof(FRAME_CONT_BAR_FIN1));

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(parsed == sizeof(buf));
        TH_EXPECT(type == TH_WS_FRAME_DATA);
        TH_EXPECT(th_buf_vec_size(&payload) == 6);
        TH_EXPECT(memcmp(th_buf_vec_begin(&payload), "foobar", 6) == 0);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_reassembles_fragment_split_across_calls)
    {
        size_t parsed = 0;
        th_ws_frame_type type;
        unsigned char first[sizeof(FRAME_TEXT_FOO_FIN0)];
        memcpy(first, FRAME_TEXT_FOO_FIN0, sizeof(first));
        th_err err = th_ws_frame_parser_parse(&parser, (char*)first, sizeof(first), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_SYSTEM(TH_EAGAIN)); // only the first fragment is here, no message yet
        TH_EXPECT(parsed == sizeof(first));

        unsigned char second[sizeof(FRAME_CONT_BAR_FIN1)];
        memcpy(second, FRAME_CONT_BAR_FIN1, sizeof(second));
        parsed = 0;
        err = th_ws_frame_parser_parse(&parser, (char*)second, sizeof(second), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(parsed == sizeof(second));
        TH_EXPECT(th_buf_vec_size(&payload) == 6);
        TH_EXPECT(memcmp(th_buf_vec_begin(&payload), "foobar", 6) == 0);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_ping_is_skipped_not_appended)
    {
        unsigned char buf[sizeof(FRAME_PING) + sizeof(FRAME_TEXT_HI)];
        memcpy(buf, FRAME_PING, sizeof(FRAME_PING));
        memcpy(buf + sizeof(FRAME_PING), FRAME_TEXT_HI, sizeof(FRAME_TEXT_HI));

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(FRAME_PING), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(parsed == sizeof(FRAME_PING));
        TH_EXPECT(type == TH_WS_FRAME_PING);
        TH_EXPECT(th_buf_vec_size(&payload) == 0);

        parsed = 0;
        err = th_ws_frame_parser_parse(&parser, (char*)buf + sizeof(FRAME_PING), sizeof(FRAME_TEXT_HI), &payload,
                                        &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(type == TH_WS_FRAME_DATA);
        TH_EXPECT(th_buf_vec_size(&payload) == 2);
        TH_EXPECT(memcmp(th_buf_vec_begin(&payload), "hi", 2) == 0);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_close_frame_reports_type)
    {
        unsigned char buf[sizeof(FRAME_CLOSE)];
        memcpy(buf, FRAME_CLOSE, sizeof(buf));

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(parsed == sizeof(buf));
        TH_EXPECT(type == TH_WS_FRAME_CLOSE);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_extended_16_bit_length)
    {
        // binary, FIN=1, payload_len=512 (encoded as 0x7E + 16-bit 0x0200)
        // payload is 0..255 repeated twice, masked with TEST_MASK.
        unsigned char src[512];
        for (size_t i = 0; i < sizeof(src); ++i)
            src[i] = (unsigned char)i;
        unsigned char buf[8 + sizeof(src)];
        buf[0] = 0x82;
        buf[1] = 0x80 | 126;
        buf[2] = 0x02;
        buf[3] = 0x00;
        memcpy(buf + 4, TEST_MASK, 4);
        for (size_t i = 0; i < sizeof(src); ++i)
            buf[8 + i] = (unsigned char)(src[i] ^ TEST_MASK[i % 4]);

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_OK);
        TH_EXPECT(parsed == sizeof(buf));
        TH_EXPECT(th_buf_vec_size(&payload) == sizeof(src));
        TH_EXPECT(memcmp(th_buf_vec_begin(&payload), src, sizeof(src)) == 0);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_rejects_unmasked_frame)
    {
        // text "hello", FIN=1, MASK bit cleared (byte 1 = 0x05, not 0x85)
        unsigned char buf[] = {0x81, 0x05, 'h', 'e', 'l', 'l', 'o'};

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_SYSTEM(TH_EPROTO));
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_rejects_reserved_bits)
    {
        // text "hello", FIN=1, RSV1 set (byte 0 = 0xC1, not 0x81)
        unsigned char buf[] = {0xc1, 0x85, 0x12, 0x34, 0x56, 0x78, 0x7a, 0x51, 0x3a, 0x14, 0x7d};

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_SYSTEM(TH_EPROTO));
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_rejects_continuation_without_message)
    {
        // continuation opcode as the *first* frame - nothing to continue
        unsigned char buf[sizeof(FRAME_CONT_BAR_FIN1)];
        memcpy(buf, FRAME_CONT_BAR_FIN1, sizeof(buf));

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_SYSTEM(TH_EPROTO));
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ws_frame_parser_rejects_oversized_message)
    {
        // binary, FIN=1, 64-bit extended length one byte past the configured max.
        unsigned char buf[14];
        buf[0] = 0x82;
        buf[1] = 0x80 | 127;
        uint64_t huge = (uint64_t)TH_CONFIG_WS_MAX_MESSAGE_LEN + 1;
        for (int shift = 56, i = 2; shift >= 0; shift -= 8, ++i)
            buf[i] = (unsigned char)(huge >> shift);
        memcpy(buf + 10, TEST_MASK, 4);

        size_t parsed = 0;
        th_ws_frame_type type;
        th_err err = th_ws_frame_parser_parse(&parser, (char*)buf, sizeof(buf), &payload, &parsed, &type);
        TH_EXPECT(err == TH_ERR_SYSTEM(TH_EPROTO));
    }
    TH_TEST_CASE_END

    th_buf_vec_deinit(&payload);
}
TH_TEST_END
