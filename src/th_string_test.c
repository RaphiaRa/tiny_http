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
    TH_TEST_CASE_BEGIN(string_resize_grows_and_shrinks_within_small)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        TH_EXPECT(th_string_set(&str, TH_STR("hi")) == TH_ERR_OK);

        TH_EXPECT(th_string_resize(&str, 5, 'x') == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == 5);
        TH_EXPECT(TH_STR_EQ(th_string_view(&str), "hixxx"));

        TH_EXPECT(th_string_resize(&str, 1, 'y') == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == 1);
        TH_EXPECT(TH_STR_EQ(th_string_view(&str), "h"));

        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_resize_promotes_small_to_large)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        TH_EXPECT(th_string_set(&str, TH_STR("hi")) == TH_ERR_OK);

        TH_EXPECT(th_string_resize(&str, 100, 'z') == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == 100);
        TH_EXPECT(th_string_data(&str)[0] == 'h');
        TH_EXPECT(th_string_data(&str)[1] == 'i');
        for (size_t i = 2; i < 100; i++) {
            TH_EXPECT(th_string_data(&str)[i] == 'z');
        }
        TH_EXPECT(th_string_data(&str)[100] == '\0');

        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_resize_grows_and_shrinks_within_large)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        TH_EXPECT(th_string_set(&str, TH_STR("Lorem ipsum dolor sit amet, consectetur adipiscing")) == TH_ERR_OK);
        size_t original_len = th_string_len(&str);

        TH_EXPECT(th_string_resize(&str, original_len + 50, 'w') == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == original_len + 50);
        for (size_t i = original_len; i < original_len + 50; i++) {
            TH_EXPECT(th_string_data(&str)[i] == 'w');
        }

        TH_EXPECT(th_string_resize(&str, 3, 'q') == TH_ERR_OK);
        TH_EXPECT(th_string_len(&str) == 3);
        TH_EXPECT(TH_STR_EQ(th_string_view(&str), "Lor"));

        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_eq_small_string)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        TH_EXPECT(th_string_set(&str, TH_STR("hello")) == TH_ERR_OK);

        TH_EXPECT(th_string_eq(&str, TH_STR("hello")));
        TH_EXPECT(!th_string_eq(&str, TH_STR("hellp")));
        TH_EXPECT(!th_string_eq(&str, TH_STR("hell")));

        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(string_eq_large_string)
    {
        th_string str;
        th_string_init(&str, th_default_allocator_get());
        th_str s = TH_STR("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        TH_EXPECT(th_string_set(&str, s) == TH_ERR_OK);

        TH_EXPECT(th_string_eq(&str, s));
        TH_EXPECT(!th_string_eq(&str, TH_STR("Lorem ipsum dolor sit amet, consectetur adipiscing elit!")));
        TH_EXPECT(!th_string_eq(&str, TH_STR("Lorem ipsum dolor sit amet")));

        th_string_deinit(&str);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
