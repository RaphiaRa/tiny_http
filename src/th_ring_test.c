#include "th_ring.h"
#include "th_system_error.h"
#include "th_test.h"

#include <string.h>

TH_LOCAL(th_err)
th_ring_write1(th_ring* rb, const char* data, size_t len)
{
    th_iov part = {(void*)data, len};
    return th_ring_write(rb, &part, 1);
}

TH_TEST_BEGIN(ring)
{
    th_ring rb;
    th_ring_init(&rb, NULL, 8, 64);

    TH_TEST_CASE_BEGIN(ring_write_and_peek)
    {
        TH_EXPECT(th_ring_write1(&rb, "abcd", 4) == TH_ERR_OK);

        th_iov iov[2];
        size_t n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 1);
        TH_EXPECT(iov[0].len == 4);
        TH_EXPECT(memcmp(iov[0].base, "abcd", 4) == 0);

        th_ring_consume(&rb, 4);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_consume_frees_space_in_place)
    {
        TH_EXPECT(th_ring_write1(&rb, "abcd", 4) == TH_ERR_OK);
        th_ring_consume(&rb, 4);

        th_iov iov[2];
        TH_EXPECT(th_ring_peek(&rb, iov) == 0);

        // still the same 8-byte chunk - fits without growing
        TH_EXPECT(th_ring_write1(&rb, "12345678", 8) == TH_ERR_OK);
        th_ring_consume(&rb, 8);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_wraps_within_a_chunk_and_peek_reports_two_iovs)
    {
        TH_EXPECT(th_ring_write1(&rb, "123456", 6) == TH_ERR_OK);
        th_ring_consume(&rb, 6);
        // head is now at 6; writing 4 bytes wraps around the 8-byte chunk
        TH_EXPECT(th_ring_write1(&rb, "abcd", 4) == TH_ERR_OK);

        th_iov iov[2];
        size_t n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 2);
        TH_EXPECT(iov[0].len == 2);
        TH_EXPECT(memcmp(iov[0].base, "ab", 2) == 0);
        TH_EXPECT(iov[1].len == 2);
        TH_EXPECT(memcmp(iov[1].base, "cd", 2) == 0);

        th_ring_consume(&rb, 4);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_grows_a_new_chunk_when_full)
    {
        TH_EXPECT(th_ring_write1(&rb, "12345678", 8) == TH_ERR_OK); // fills the 8-byte chunk
        TH_EXPECT(th_ring_write1(&rb, "xy", 2) == TH_ERR_OK);       // doesn't fit - grows a new chunk

        th_iov iov[2];
        size_t n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 1);
        TH_EXPECT(iov[0].len == 8);
        TH_EXPECT(memcmp(iov[0].base, "12345678", 8) == 0);

        th_ring_consume(&rb, 8); // frees the first chunk, head moves to the grown one
        n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 1);
        TH_EXPECT(iov[0].len == 2);
        TH_EXPECT(memcmp(iov[0].base, "xy", 2) == 0);

        th_ring_consume(&rb, 2);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_never_splits_a_message_across_chunks)
    {
        TH_EXPECT(th_ring_write1(&rb, "123456", 6) == TH_ERR_OK); // 2 bytes free in the 8-byte chunk
        TH_EXPECT(th_ring_write1(&rb, "abcd", 4) == TH_ERR_OK);   // doesn't fit - grows instead of splitting

        th_iov iov[2];
        size_t n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 1);
        TH_EXPECT(iov[0].len == 6);
        th_ring_consume(&rb, 6);

        n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 1);
        TH_EXPECT(iov[0].len == 4);
        TH_EXPECT(memcmp(iov[0].base, "abcd", 4) == 0);
        th_ring_consume(&rb, 4);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_grows_past_an_emptied_sole_chunk)
    {
        // drain the sole 8-byte chunk back to empty - it's kept alive
        // (not freed) since it's still both head and tail
        TH_EXPECT(th_ring_write1(&rb, "abcd", 4) == TH_ERR_OK);
        th_ring_consume(&rb, 4);

        // now write something bigger than that emptied chunk's capacity
        TH_EXPECT(th_ring_write1(&rb, "0123456789", 10) == TH_ERR_OK);

        th_iov iov[2];
        size_t n = th_ring_peek(&rb, iov);
        TH_EXPECT(n >= 1);
        TH_EXPECT(iov[0].len > 0); // the new data must be reachable from peek

        size_t total = 0;
        for (size_t i = 0; i < n; ++i)
            total += iov[i].len;
        TH_EXPECT(total == 10);
        th_ring_consume(&rb, 10);
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_rejects_message_over_max_len)
    {
        char big[65];
        memset(big, 'x', sizeof(big));
        TH_EXPECT(th_ring_write1(&rb, big, sizeof(big)) == TH_ERR_INVALID_ARG);

        th_iov iov[2];
        TH_EXPECT(th_ring_peek(&rb, iov) == 0); // nothing was queued
    }
    TH_TEST_CASE_END

    TH_TEST_CASE_BEGIN(ring_multipart_write_lands_in_one_chunk)
    {
        th_iov parts[2] = {{(void*)"ab", 2}, {(void*)"cd", 2}};
        TH_EXPECT(th_ring_write(&rb, parts, 2) == TH_ERR_OK);

        th_iov iov[2];
        size_t n = th_ring_peek(&rb, iov);
        TH_EXPECT(n == 1);
        TH_EXPECT(iov[0].len == 4);
        TH_EXPECT(memcmp(iov[0].base, "abcd", 4) == 0);

        th_ring_consume(&rb, 4);
    }
    TH_TEST_CASE_END

    th_ring_deinit(&rb);
}
TH_TEST_END
