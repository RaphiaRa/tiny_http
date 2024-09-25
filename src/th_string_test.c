#include "th_string.h"
#include "th_test.h"

TH_TEST_BEGIN(string)
{
    TH_TEST_CASE_BEGIN(string_make_literal)
    {
        th_string str = TH_STRING("TEST");
        TH_EXPECT(str.len == 4);
        TH_EXPECT(str.ptr[0] == 'T');
        TH_EXPECT(str.ptr[1] == 'E');
        TH_EXPECT(str.ptr[2] == 'S');
        TH_EXPECT(str.ptr[3] == 'T');
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_make)
    {
        const char* test_string = "Test String";
        th_string str = th_string_make(test_string, 4);
        TH_EXPECT(str.len == 4);
        TH_EXPECT(str.ptr[0] == 'T');
        TH_EXPECT(str.ptr[1] == 'e');
        TH_EXPECT(str.ptr[2] == 's');
        TH_EXPECT(str.ptr[3] == 't');
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_trim)
    {
        th_string str = TH_STRING("  Test String  ");
        th_string trimmed = th_string_trim(str);
        TH_EXPECT(TH_STRING_EQ(trimmed, "Test String"));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
