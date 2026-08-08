#include "th_config.h"

#if TH_WITH_SSL

#include "th_ssl_smem_bio.h"
#include "th_test.h"

#include <string.h>

TH_TEST_BEGIN(ssl_smem_bio)
{
    th_ssl_context context;
    context.ctx = NULL;
    context.smem_method = NULL;
    context.ops = NULL;

    TH_TEST_CASE_BEGIN(smem_bio_returns_same_method_across_calls)
    {
        BIO_METHOD* a = th_smem_bio(&context);
        BIO_METHOD* b = th_smem_bio(&context);
        TH_EXPECT(a != NULL);
        TH_EXPECT(a == b);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_new_and_free_via_real_bio_api)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        TH_EXPECT(bio != NULL);
        th_smem_bio_setup_buf(bio, NULL, 64);
        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_write_then_read_round_trips_data)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);

        TH_EXPECT(BIO_write(bio, "hello", 5) == 5);
        TH_EXPECT(BIO_pending(bio) == 5);

        char out[16] = {0};
        TH_EXPECT(BIO_read(bio, out, sizeof(out)) == 5);
        TH_EXPECT(memcmp(out, "hello", 5) == 0);
        TH_EXPECT(BIO_pending(bio) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_read_when_empty_and_not_eof_requests_retry)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);

        char out[16];
        TH_EXPECT(BIO_read(bio, out, sizeof(out)) == -1);
        TH_EXPECT(BIO_should_retry(bio));

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_read_when_empty_and_eof_returns_zero)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        th_smem_bio_set_eof(bio);

        char out[16];
        TH_EXPECT(BIO_read(bio, out, sizeof(out)) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_read_partial_when_less_available_than_requested)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(BIO_write(bio, "hi", 2) == 2);

        char out[16] = {0};
        TH_EXPECT(BIO_read(bio, out, sizeof(out)) == 2);
        TH_EXPECT(memcmp(out, "hi", 2) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_write_after_eof_returns_zero)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        th_smem_bio_set_eof(bio);

        TH_EXPECT(BIO_write(bio, "x", 1) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_write_grows_buffer_up_to_max_len)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 8);

        TH_EXPECT(BIO_write(bio, "01234567", 8) == 8);
        TH_EXPECT(BIO_pending(bio) == 8);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_write_past_max_len_short_writes_and_requests_retry)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 4);

        /* First write fills the 4-byte cap exactly. */
        TH_EXPECT(BIO_write(bio, "0123", 4) == 4);
        /* Second write has no room left: buffer is capped at max_len. */
        int ret = (int)BIO_write(bio, "4567", 4);
        TH_EXPECT(ret == -1);
        TH_EXPECT(BIO_should_retry(bio));

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_inc_read_pos_resets_positions_once_fully_drained)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(th_smem_ensure_buf_size(bio, 64) == 64);
        TH_EXPECT(BIO_write(bio, "hello", 5) == 5);

        th_iov iov;
        th_smem_bio_get_rdata(bio, &iov);
        TH_EXPECT(iov.len == 5);

        th_smem_bio_inc_read_pos(bio, 5);
        TH_EXPECT(BIO_pending(bio) == 0);

        /* Positions wrapped back to 0, so the buffer's whole 64-byte
         * capacity is available again instead of only the 59 bytes past
         * an ever-advancing write_pos. */
        th_iov wbuf;
        th_smem_bio_get_wbuf(bio, &wbuf);
        TH_EXPECT(wbuf.len == 64);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_get_wbuf_reflects_write_position)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(th_smem_ensure_buf_size(bio, 64) == 64);
        TH_EXPECT(BIO_write(bio, "hi", 2) == 2);

        th_iov iov;
        th_smem_bio_get_wbuf(bio, &iov);
        TH_EXPECT(iov.len == 62);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_fed_ciphertext_style_write_via_inc_write_pos)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(th_smem_ensure_buf_size(bio, 64) == 64);

        th_iov wbuf;
        th_smem_bio_get_wbuf(bio, &wbuf);
        TH_EXPECT(wbuf.len == 64);
        memcpy(wbuf.base, "abc", 3);
        th_smem_bio_inc_write_pos(bio, 3);

        TH_EXPECT(BIO_pending(bio) == 3);
        char out[8] = {0};
        TH_EXPECT(BIO_read(bio, out, sizeof(out)) == 3);
        TH_EXPECT(memcmp(out, "abc", 3) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_ctrl_reset_clears_positions_and_eof)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(BIO_write(bio, "hi", 2) == 2);
        th_smem_bio_set_eof(bio);

        TH_EXPECT(BIO_reset(bio) == 1);
        TH_EXPECT(BIO_pending(bio) == 0);
        TH_EXPECT(!BIO_eof(bio));

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_ctrl_eof_true_only_when_eof_set_and_drained)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(BIO_write(bio, "hi", 2) == 2);
        th_smem_bio_set_eof(bio);

        /* eof flag set, but unread data remains: not yet at eof. */
        TH_EXPECT(!BIO_eof(bio));

        char out[8] = {0};
        TH_EXPECT(BIO_read(bio, out, sizeof(out)) == 2);
        TH_EXPECT(BIO_eof(bio));

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_ctrl_info_returns_buffer_size_and_pointer)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(th_smem_ensure_buf_size(bio, 64) == 64);
        TH_EXPECT(BIO_write(bio, "hi", 2) == 2);

        char* data = NULL;
        long size = BIO_get_mem_data(bio, &data);
        TH_EXPECT(size == 64);
        TH_EXPECT(data != NULL);
        TH_EXPECT(memcmp(data, "hi", 2) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_ctrl_wpending_is_always_zero)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(BIO_write(bio, "hi", 2) == 2);

        TH_EXPECT(BIO_wpending(bio) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_ctrl_dup_and_flush_report_success)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);

        TH_EXPECT(BIO_flush(bio) == 1);
        int dup_ret = 0;
        TH_EXPECT(BIO_dup_state(bio, &dup_ret) == 1);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_bio_ctrl_unknown_command_returns_zero)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);

        TH_EXPECT(BIO_ctrl(bio, 12345, 0, NULL) == 0);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_ensure_buf_size_caps_at_max_len)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 16);

        TH_EXPECT(th_smem_ensure_buf_size(bio, 1024) == 16);

        char* data = NULL;
        long size = BIO_get_mem_data(bio, &data);
        TH_EXPECT(size == 16);

        BIO_free(bio);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(smem_ensure_buf_size_does_not_shrink_existing_buffer)
    {
        BIO* bio = BIO_new(th_smem_bio(&context));
        th_smem_bio_setup_buf(bio, NULL, 64);
        TH_EXPECT(th_smem_ensure_buf_size(bio, 32) == 32);

        TH_EXPECT(th_smem_ensure_buf_size(bio, 8) == 32);

        BIO_free(bio);
    }
    TH_TEST_CASE_END

    th_ssl_context_deinit(&context);
}
TH_TEST_END

#endif
