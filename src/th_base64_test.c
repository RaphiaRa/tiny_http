#include "th_base64.h"
#include "th_test.h"

#include <string.h>

static bool
encodes_to(const char* input, const char* expected)
{
    th_string out;
    th_string_init(&out, th_default_allocator_get());
    bool ok = th_base64_encode(th_str_from_cstr(input), &out) == TH_ERR_OK
              && th_str_eq(th_string_view(&out), th_str_from_cstr(expected));
    th_string_deinit(&out);
    return ok;
}

TH_TEST_BEGIN(base64)
{
    TH_TEST_CASE_BEGIN(base64_empty)
    {
        TH_EXPECT(encodes_to("", ""));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(base64_one_byte_needs_two_padding_chars)
    {
        TH_EXPECT(encodes_to("f", "Zg=="));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(base64_two_bytes_needs_one_padding_char)
    {
        TH_EXPECT(encodes_to("fo", "Zm8="));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(base64_three_bytes_needs_no_padding)
    {
        TH_EXPECT(encodes_to("foo", "Zm9v"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(base64_multiple_groups_with_padding)
    {
        TH_EXPECT(encodes_to("foobar", "Zm9vYmFy"));
        TH_EXPECT(encodes_to("foob", "Zm9vYg=="));
        TH_EXPECT(encodes_to("fooba", "Zm9vYmE="));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(base64_sha1_digest_length)
    {
        // Sec-WebSocket-Accept encodes a 20-byte SHA-1 digest into 28 chars.
        unsigned char digest[20] = {0};
        th_string out;
        th_string_init(&out, th_default_allocator_get());
        TH_EXPECT(th_base64_encode(th_str_make((const char*)digest, sizeof(digest)), &out) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&out) == 28);
        th_string_deinit(&out);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(base64_reuses_existing_output_string)
    {
        th_string out;
        th_string_init(&out, th_default_allocator_get());
        TH_EXPECT(th_string_set(&out, TH_STR("leftover content")) == TH_ERR_OK);
        TH_EXPECT(th_base64_encode(th_str_from_cstr("foo"), &out) == TH_ERR_OK);
        TH_EXPECT(th_str_eq(th_string_view(&out), TH_STR("Zm9v")));
        th_string_deinit(&out);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
