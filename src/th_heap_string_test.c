#include "th_heap_string.h"
#include "th_test.h"

TH_TEST_BEGIN(heap_string)
{
    TH_TEST_CASE_BEGIN(heap_string_init)
    {
        th_heap_string str;
        th_heap_string_init(&str, th_default_allocator_get());
        TH_EXPECT(th_heap_string_len(&str) == 0);
        TH_EXPECT(th_heap_string_data(&str) != NULL);
        th_heap_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(heap_string_set)
    {
        th_heap_string str;
        th_heap_string_init(&str, th_default_allocator_get());
        th_string s = TH_STRING("hello");
        TH_EXPECT(th_heap_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_heap_string_len(&str) == s.len);
        TH_EXPECT(th_string_eq(th_heap_string_view(&str), s));
        s = TH_STRING("Lorem ipsum dolor sit amet");
        TH_EXPECT(th_heap_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_heap_string_len(&str) == s.len);
        TH_EXPECT(th_string_eq(th_heap_string_view(&str), s));
        s = TH_STRING("");
        TH_EXPECT(th_heap_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_heap_string_len(&str) == s.len);
        TH_EXPECT(th_string_eq(th_heap_string_view(&str), s));
        s = TH_STRING("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas ullamcorper mi ut felis pulvinar tincidunt.");
        TH_EXPECT(th_heap_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_heap_string_len(&str) == s.len);
        TH_EXPECT(th_string_eq(th_heap_string_view(&str), s));
        s = TH_STRING("");
        TH_EXPECT(th_heap_string_set(&str, s) == TH_ERR_OK);
        TH_EXPECT(th_heap_string_len(&str) == s.len);
        TH_EXPECT(th_string_eq(th_heap_string_view(&str), s));
        th_heap_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(heap_string_append)
    {
        th_heap_string str;
        th_heap_string_init(&str, th_default_allocator_get());
        for (int i = 0; i < 100; ++i) {
            TH_EXPECT(th_heap_string_append(&str, TH_STRING("A")) == TH_ERR_OK);
            TH_EXPECT(th_heap_string_len(&str) == (size_t)(i + 1));
        }
        th_heap_string_deinit(&str);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
