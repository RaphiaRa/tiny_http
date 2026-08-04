#include "th_multipart_parser.h"
#include "th_test.h"

#include <stdlib.h>
#include <string.h>

/* RFC 7578 (multipart/form-data), built on RFC 2046 section 5.1.1:
 * - body = dash-boundary CRLF part *( CRLF dash-boundary CRLF part )
 *          CRLF close-delimiter epilogue
 * - boundary param on Content-Type; missing/empty is malformed.
 * - Content-Disposition + "name" mandatory per part; filename/Content-Type
 *   optional (filename absent => form field, present => file upload).
 * - Content runs until the next boundary, via explicit Content-Length or a
 *   scan for the boundary.
 * - Deviation from RFC 2046: no epilogue tolerance, trailing bytes after
 *   the close-delimiter are rejected.
 */

TH_TEST_BEGIN(multipart_parser)
{
    TH_TEST_CASE_BEGIN(boundary_extracted_from_content_type)
    {
        th_str boundary;
        TH_EXPECT(th_multipart_parser_boundary(TH_STR("multipart/form-data; boundary=abc123"), &boundary)
                  == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(boundary, "abc123"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(boundary_extracted_from_quoted_form)
    {
        th_str boundary;
        TH_EXPECT(th_multipart_parser_boundary(TH_STR("multipart/form-data; boundary=\"abc 123\""), &boundary)
                  == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(boundary, "abc 123"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(boundary_missing_is_a_bad_request)
    {
        th_str boundary;
        TH_EXPECT(th_multipart_parser_boundary(TH_STR("multipart/form-data"), &boundary)
                  == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(boundary_empty_value_is_a_bad_request)
    {
        th_str boundary;
        TH_EXPECT(th_multipart_parser_boundary(TH_STR("multipart/form-data; boundary="), &boundary)
                  == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parses_a_single_form_field)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"field1\"\r\n\r\n"
                             "value1\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);
        TH_EXPECT(!th_multipart_parser_done(&parser));

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.name, "field1"));
        TH_EXPECT(TH_STR_EQ(part.content, "value1"));
        TH_EXPECT(th_str_empty(part.filename));
        TH_EXPECT(th_str_empty(part.content_type));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parses_multiple_parts_in_order)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                             "1\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"b\"\r\n\r\n"
                             "2\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.name, "a") && TH_STR_EQ(part.content, "1"));
        TH_EXPECT(!th_multipart_parser_done(&parser));

        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.name, "b") && TH_STR_EQ(part.content, "2"));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parses_a_file_part_with_filename_and_content_type)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"file\"; filename=\"a.txt\"\r\n"
                             "Content-Type: text/plain\r\n\r\n"
                             "hello\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.name, "file"));
        TH_EXPECT(TH_STR_EQ(part.filename, "a.txt"));
        TH_EXPECT(TH_STR_EQ(part.content_type, "text/plain"));
        TH_EXPECT(TH_STR_EQ(part.content, "hello"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(parses_a_part_using_explicit_content_length)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n"
                             "Content-Length: 6\r\n\r\n"
                             "he\r\nlo\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.content, "he\r\nlo"));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(empty_part_body_is_valid)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                             "\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(th_str_empty(part.content));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(missing_content_disposition_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Type: text/plain\r\n\r\n"
                             "value\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(missing_name_parameter_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data\r\n\r\n"
                             "value\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(header_line_without_crlf_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(unterminated_body_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                             "value with no closing boundary");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(body_ending_in_a_lone_cr_is_a_bad_request)
    {
        /* regression: an EOL scan must not read past the buffer when a
         * trailing '\r' has no following byte to check for '\n'. Uses a
         * heap buffer sized exactly to the content (no NUL terminator, so
         * ASan/Valgrind can catch a one-byte overread that a string
         * literal's implicit '\0' would silently hide). */
        const char* content = "--boundary\r\n"
                              "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                              "value with no closing boundary\r";
        size_t len = strlen(content);
        char* buf = malloc(len);
        memcpy(buf, content, len);
        th_str body = th_str_make(buf, len);

        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));

        free(buf);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(content_length_not_followed_by_boundary_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n"
                             "Content-Length: 5\r\n\r\n"
                             "hello garbage instead of a boundary\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(missing_opening_boundary_is_a_bad_request)
    {
        th_str body = TH_STR("not a boundary line at all\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(body_with_only_the_closing_delimiter_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(trailing_data_after_closing_delimiter_is_a_bad_request)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                             "value\r\n"
                             "--boundary--\r\n"
                             "epilogue garbage");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(duplicate_field_names_are_both_surfaced)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                             "1\r\n"
                             "--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n\r\n"
                             "2\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.name, "a") && TH_STR_EQ(part.content, "1"));

        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.name, "a") && TH_STR_EQ(part.content, "2"));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(empty_filename_is_treated_as_a_form_field)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"file\"; filename=\"\"\r\n\r\n"
                             "\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(th_str_empty(part.filename));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(path_like_filename_is_kept_opaque)
    {
        // RFC 7578 4.2: filename is opaque, not a filesystem path.
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"file\"; filename=\"../../etc/passwd\"\r\n\r\n"
                             "data\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.filename, "../../etc/passwd"));
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(boundary_like_content_inside_content_length_is_not_treated_as_boundary)
    {
        th_str body = TH_STR("--boundary\r\n"
                             "Content-Disposition: form-data; name=\"a\"\r\n"
                             "Content-Length: 12\r\n\r\n"
                             "--boundaryXX\r\n"
                             "--boundary--\r\n");
        th_multipart_parser parser;
        TH_EXPECT(th_multipart_parser_init(&parser, body, TH_STR("boundary")) == TH_ERR_OK);

        th_multipart_part part;
        TH_EXPECT(th_multipart_parser_next(&parser, &part) == TH_ERR_OK);
        TH_EXPECT(TH_STR_EQ(part.content, "--boundaryXX"));
        TH_EXPECT(th_multipart_parser_done(&parser));
    }
    TH_TEST_CASE_END
}
TH_TEST_END
