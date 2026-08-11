#include "th_sha1.h"
#include "th_test.h"

#include <string.h>

static bool
digest_hex_eq(const unsigned char digest[TH_SHA1_DIGEST_LEN], const char* hex)
{
    char buf[TH_SHA1_DIGEST_LEN * 2 + 1];
    for (size_t i = 0; i < TH_SHA1_DIGEST_LEN; i++)
        snprintf(buf + i * 2, 3, "%02x", digest[i]);
    return strcmp(buf, hex) == 0;
}

TH_TEST_BEGIN(sha1)
{
    TH_TEST_CASE_BEGIN(sha1_empty)
    {
        unsigned char digest[TH_SHA1_DIGEST_LEN];
        th_sha1((th_buffer){"", 0}, digest);
        TH_EXPECT(digest_hex_eq(digest, "da39a3ee5e6b4b0d3255bfef95601890afd80709"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sha1_short_single_block)
    {
        unsigned char digest[TH_SHA1_DIGEST_LEN];
        th_sha1((th_buffer){"abc", 3}, digest);
        TH_EXPECT(digest_hex_eq(digest, "a9993e364706816aba3e25717850c26c9cd0d89d"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sha1_message_spanning_two_blocks)
    {
        // 56 bytes: no room for padding in the first block.
        const char* msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        unsigned char digest[TH_SHA1_DIGEST_LEN];
        th_sha1((th_buffer){msg, strlen(msg)}, digest);
        TH_EXPECT(digest_hex_eq(digest, "84983e441c3bd26ebaae4aa1f95129e5e54670f1"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(sha1_exactly_one_block)
    {
        // Full block, padding must spill into a second block.
        char msg[64];
        memset(msg, 'a', sizeof(msg));
        unsigned char digest[TH_SHA1_DIGEST_LEN];
        th_sha1((th_buffer){msg, sizeof(msg)}, digest);
        TH_EXPECT(digest_hex_eq(digest, "0098ba824b5c16427bd7a1122a5a442a25ec644d"));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
