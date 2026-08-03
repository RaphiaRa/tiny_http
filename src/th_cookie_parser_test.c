#include "th_cookie_parser.h"
#include "th_test.h"

/* Properties checked below, per RFC 6265 section 4.1.1/4.2.1:
 *
 * - cookie-string = cookie-pair *( ";" SP cookie-pair )
 * - cookie-pair   = cookie-name "=" cookie-value
 * - cookie-name   = token (no '=', ';', space, or CTLs)
 * - cookie-value  = *cookie-octet / ( DQUOTE *cookie-octet DQUOTE )
 * - cookie-octet  = %x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E
 *                   (printable ASCII minus space, DQUOTE, comma, semicolon, backslash)
 * - cookie-value may be empty.
 * - Duplicate cookie-name is not disallowed by the grammar - the parser
 *   surfaces every pair, deduplication (if any) is the caller's job.
 * - The formal separator is exactly "; ", but real senders deviate (bare
 *   ';', extra spaces) - the parser tolerates OWS around pairs and '='.
 * - A cookie-value's surrounding DQUOTE pair (if present and matched) is
 *   stripped rather than treated as part of the value (project decision -
 *   RFC 6265 itself does not mandate stripping).
 * - A pair with no '=' is malformed: TH_ERR_HTTP(TH_CODE_BAD_REQUEST).
 * - A cookie-value containing a byte outside cookie-octet (space, DQUOTE
 *   mid-value, comma, backslash, or a control character) is malformed:
 *   TH_ERR_HTTP(TH_CODE_BAD_REQUEST).
 * - A cookie-name containing a byte outside token (a separator like SP,
 *   '(', ')', '<', '>', '@', ',', ';', ':', '\', '"', '/', '[', ']', '?',
 *   '=', '{', '}', or a control character) is malformed:
 *   TH_ERR_HTTP(TH_CODE_BAD_REQUEST).
 */

TH_TEST_BEGIN(cookie_parser)
{
    TH_TEST_CASE_BEGIN(parses_a_single_pair)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("session_id=abc123"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "session_id"));
        TH_EXPECT(TH_STR_EQ(value, "abc123"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parses_multiple_pairs_separated_by_semicolon_space)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("a=1; b=2; c=3"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "1"));
        TH_EXPECT(!th_cookie_parser_done(&parser));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "b") && TH_STR_EQ(value, "2"));
        TH_EXPECT(!th_cookie_parser_done(&parser));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "c") && TH_STR_EQ(value, "3"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tolerates_bare_semicolon_with_no_space)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("a=1;b=2"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "1"));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "b") && TH_STR_EQ(value, "2"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(tolerates_extra_whitespace_around_pairs_and_equals)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("  a = 1  ;   b=2   "));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "1"));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "b") && TH_STR_EQ(value, "2"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(empty_cookie_value_is_valid)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name="));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "name"));
        TH_EXPECT(th_str_empty(value));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(quoted_cookie_value_has_quotes_stripped)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=\"quoted value\""));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "name"));
        TH_EXPECT(TH_STR_EQ(value, "quoted value"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(quoted_empty_cookie_value_strips_to_empty)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=\"\""));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "name"));
        TH_EXPECT(th_str_empty(value));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(unmatched_leading_quote_is_a_bad_request)
    {
        // A DQUOTE is only legal as a matched wrapping pair (quoted cookie-value);
        // a lone/unmatched DQUOTE is not a valid cookie-octet.
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=\"abc"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_value_may_contain_full_octet_range)
    {
        // cookie-octet excludes only space, DQUOTE, comma, semicolon, backslash -
        // everything else in printable ASCII (and beyond DQUOTE/backslash) is allowed.
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=abc!#$%&'()*+-./:<=>?@[]^_`{|}~"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "name"));
        TH_EXPECT(TH_STR_EQ(value, "abc!#$%&'()*+-./:<=>?@[]^_`{|}~"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_value_containing_a_comma_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=has,a,comma"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_value_containing_a_backslash_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=has\\backslash"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_value_containing_a_control_character_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, th_str_make("name=has\x01ctl", 12));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(unquoted_cookie_value_containing_a_mid_value_quote_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("name=abc\"def"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_name_containing_a_space_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("bad name=1"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_name_containing_a_separator_is_a_bad_request)
    {
        // '(' is a token separator per RFC 2616 section 2.2.
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("bad(name=1"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(cookie_name_containing_a_control_character_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, th_str_make("na\x01me=1", 6));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(later_cookie_name_containing_a_separator_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("a=1; bad@name=2"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "1"));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(duplicate_cookie_names_are_both_surfaced)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("a=1; a=2"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "1"));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "2"));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(pair_missing_equals_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("not_a_valid_cookie"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(later_pair_missing_equals_is_a_bad_request)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR("a=1; not_valid"));

        th_str key, value;
        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(key, "a") && TH_STR_EQ(value, "1"));

        TH_EXPECT(th_cookie_parser_next(&parser, &key, &value) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(empty_header_is_immediately_done)
    {
        th_cookie_parser parser;
        th_cookie_parser_init(&parser, TH_STR(""));
        TH_EXPECT(th_cookie_parser_done(&parser));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
