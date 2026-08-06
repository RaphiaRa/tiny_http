#include "th_filepath.h"
#include "th_test.h"

#include <string.h>

TH_TEST_BEGIN(filepath)
{
    TH_TEST_CASE_BEGIN(filepath_simple_name)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo.txt")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo.txt") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_nested_path)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/bar/baz.txt")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo/bar/baz.txt") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_dot_inside_name)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("a.b")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "a.b") == 0);
        TH_EXPECT(th_filepath_init(&path, TH_STR("..foo")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "..foo") == 0);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo..")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo..") == 0);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo...bar")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo...bar") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_empty)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("")) == TH_ERR_INVALID_ARG);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_absolute_rejected)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("/etc/passwd")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("/")) == TH_ERR_INVALID_ARG);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_dot_component_is_skipped)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR(".")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("./foo")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo") == 0);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/.")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo") == 0);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/./bar")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo/bar") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_dotdot_component_rejected)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("..")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("../foo")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/..")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/../bar")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/bar/../../../etc/passwd")) == TH_ERR_INVALID_ARG);
        TH_EXPECT(th_filepath_init(&path, TH_STR("/../etc/passwd")) == TH_ERR_INVALID_ARG);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_double_slash_collapses)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo//bar")) == TH_ERR_OK);
        TH_EXPECT(strcmp(th_filepath_cstr(&path), "foo/bar") == 0);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_trailing_slash_rejected)
    {
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, TH_STR("foo/")) == TH_ERR_INVALID_ARG);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_embedded_nul)
    {
        // No ".." anywhere - this only fails if the NUL itself is
        // detected, not as a side effect of the traversal check.
        char buffer[] = "foo\0bar";
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, th_str_make(buffer, sizeof(buffer) - 1)) == TH_ERR_INVALID_ARG);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(filepath_too_long)
    {
        char buffer[TH_CONFIG_MAX_PATH_LEN + 2];
        memset(buffer, 'a', sizeof(buffer));
        th_filepath path;
        TH_EXPECT(th_filepath_init(&path, th_str_make(buffer, sizeof(buffer))) == TH_ERR_INVALID_ARG);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
