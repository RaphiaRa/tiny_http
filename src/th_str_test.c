#include "th_str.h"
#include "th_test.h"

TH_TEST_BEGIN(str)
{
    TH_TEST_CASE_BEGIN(str_make_literal)
    {
        th_str str = TH_STR("TEST");
        TH_EXPECT(str.len == 4);
        TH_EXPECT(str.ptr[0] == 'T');
        TH_EXPECT(str.ptr[1] == 'E');
        TH_EXPECT(str.ptr[2] == 'S');
        TH_EXPECT(str.ptr[3] == 'T');
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_make)
    {
        const char* test_str = "Test String";
        th_str str = th_str_make(test_str, 4);
        TH_EXPECT(str.len == 4);
        TH_EXPECT(str.ptr[0] == 'T');
        TH_EXPECT(str.ptr[1] == 'e');
        TH_EXPECT(str.ptr[2] == 's');
        TH_EXPECT(str.ptr[3] == 't');
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_trim)
    {
        th_str str = TH_STR("  Test String  ");
        th_str trimmed = th_str_trim(str);
        TH_EXPECT(TH_STR_EQ(trimmed, "Test String"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_eq_identical_strings_are_equal)
    {
        TH_EXPECT(th_str_eq(TH_STR("hello"), TH_STR("hello")));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_eq_different_lengths_are_not_equal)
    {
        TH_EXPECT(!th_str_eq(TH_STR("hello"), TH_STR("hello!")));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_eq_same_length_different_content_are_not_equal)
    {
        TH_EXPECT(!th_str_eq(TH_STR("hello"), TH_STR("hellp")));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_eq_differ_in_last_byte_are_not_equal)
    {
        TH_EXPECT(!th_str_eq(TH_STR("aaaaaaaaaa"), TH_STR("aaaaaaaaab")));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_eq_empty_strings_are_equal)
    {
        TH_EXPECT(th_str_eq(th_str_make_empty(), th_str_make_empty()));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(str_eq_empty_and_non_empty_are_not_equal)
    {
        TH_EXPECT(!th_str_eq(th_str_make_empty(), TH_STR("a")));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
