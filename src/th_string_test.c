#include "th_string.h"
#include "th_test.h"

TH_TEST_BEGIN(string)
{
    TH_TEST_CASE_BEGIN(string_init)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        TH_EXPECT(th_string_len(&str) == 0);
        TH_EXPECT(th_string_data(&str) != NULL);
        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_set)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        th_str s = TH_STR("hello");
        TH_EXPECT(th_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == s.len);
        TH_EXPECT(th_str_eq(th_string_view(&str), s));
        s = TH_STR("Lorem ipsum dolor sit amet");
        TH_EXPECT(th_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == s.len);
        TH_EXPECT(th_str_eq(th_string_view(&str), s));
        s = TH_STR("");
        TH_EXPECT(th_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == s.len);
        TH_EXPECT(th_str_eq(th_string_view(&str), s));
        s = TH_STR("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas ullamcorper mi ut felis pulvinar tincidunt.");
        TH_EXPECT(th_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == s.len);
        TH_EXPECT(th_str_eq(th_string_view(&str), s));
        s = TH_STR("");
        TH_EXPECT(th_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == s.len);
        TH_EXPECT(th_str_eq(th_string_view(&str), s));
        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_append)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        for (int i = 0; i < 100; ++i) {
            TH_EXPECT(th_string_append(&str, TH_STR("A")) == TH_ERR_OK);
            TH_EXPECT(th_string_len(&str) == (size_t)(i + 1));
        }
        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
