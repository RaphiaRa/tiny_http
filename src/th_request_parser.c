#include "th_request_parser.h"

#include "th_header_id.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "request_parser"

TH_PRIVATE(void)
th_request_parser_init(th_request_parser* parser)
{
    parser->state = TH_REQUEST_PARSER_STATE_METHOD;
    parser->content_len = 0;
    parser->body_encoding = TH_REQUEST_BODY_ENCODING_NONE;
}

TH_PRIVATE(void)
th_request_parser_reset(th_request_parser* parser)
{
    parser->state = TH_REQUEST_PARSER_STATE_METHOD;
    parser->content_len = 0;
    parser->body_encoding = TH_REQUEST_BODY_ENCODING_NONE;
}

TH_PRIVATE(size_t)
th_request_parser_content_len(th_request_parser* parser)
{
    return parser->content_len;
}

TH_LOCAL(th_err)
th_request_parser_do_cookie(th_request* request, th_string cookie)
{
    size_t eq = th_string_find_first(cookie, 0, '=');
    if (eq == th_string_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    th_string key = th_string_trim(th_string_substr(cookie, 0, eq));
    th_string value = th_string_trim(th_string_substr(cookie, eq + 1, cookie.len));
    th_err err = TH_ERR_OK;
    if ((err = th_request_add_cookie(request, key, value)) != TH_ERR_OK) {
        return err;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_cookie_list(th_request* request, th_string cookie_list)
{
    size_t start = 0;
    size_t pos = 0;
    while (pos != th_string_npos) {
        pos = th_string_find_first(cookie_list, start, ';');
        th_string cookie = th_string_trim(th_string_substr(cookie_list, start, pos - start));
        th_err err = th_request_parser_do_cookie(request, cookie);
        if (err != TH_ERR_OK) {
            return err;
        }
        start = pos + 1;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_next_queryvar(th_string string, size_t* pos, th_string* key, th_string* value)
{
    size_t eq = th_string_find_first(string, *pos, '=');
    if (eq == th_string_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    *key = th_string_trim(th_string_substr(string, *pos, eq - *pos));
    *pos = th_string_find_first(string, eq + 1, '&');
    if (*pos != th_string_npos) {
        *value = th_string_trim(th_string_substr(string, eq + 1, *pos - eq - 1));
        (*pos)++;
        return TH_ERR_OK;
    } else {
        *value = th_string_trim(th_string_substr(string, eq + 1, *pos));
        return TH_ERR_OK;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_bodyvars(th_request* request, th_string body)
{
    th_err err = TH_ERR_OK;
    size_t pos = 0;
    while (pos != th_string_npos) {
        th_string key;
        th_string value;
        err = th_request_parser_do_next_queryvar(body, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if ((err = th_request_add_formvar(request, key, value)) != TH_ERR_OK) {
            return err;
        }
    }
    return err;
}

/* Get the next HTTP token from the buffer, stopping at the given character */
TH_LOCAL(th_err)
th_request_parser_next_token(th_string buffer, th_string* token, char until, size_t* parsed)
{
    static const int token_char[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 32-47 (don't allow space, ")
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 48-57 (0-9)
        1, 1, 0, 1, 0, 1, 1,                            // 58-64 (don't allow <,>)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 65-80 (A-P)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 81-90 (Q-Z)
        0, 0, 0, 1, 1, 1,                               // 91-96 (don't allow [, \, ])
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 97-112 (a-p)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 113-122 (q-z)
        0, 1, 0, 1, 0,                                  // 123-127 (don't allow {, }, DEL)
        // implicitely set to 0 for 128-255
    };
    size_t i = 0;
    while (i < buffer.len && buffer.ptr[i] != until) {
        if (token_char[(unsigned char)buffer.ptr[i]] == 0)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        i++;
    }
    if (i == buffer.len)
        return TH_ERR_OK;
    if (i == 0)
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *token = th_string_substr(buffer, 0, i);
    *parsed = i + 1;
    return TH_ERR_OK;
}

TH_LOCAL(bool)
th_request_parser_is_printable_string(th_string input)
{
    for (size_t i = 0; i < input.len; i++) {
        if (input.ptr[i] < 32 || input.ptr[i] > 126) {
            return false;
        }
    }
    return true;
}

TH_LOCAL(th_err)
th_request_parser_do_method(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed_out)
{
    th_string method;
    size_t parsed = 0;
    th_err err = th_request_parser_next_token(buffer, &method, ' ', &parsed);
    if (err != TH_ERR_OK || parsed == 0) {
        return err;
    }
    struct th_method_mapping* mm = th_method_mapping_find(method.ptr, method.len);
    if (!mm) {
        return TH_ERR_HTTP(TH_CODE_NOT_IMPLEMENTED);
    }
    th_request_set_method(request, mm->method);
    *parsed_out = parsed;
    parser->state = TH_REQUEST_PARSER_STATE_PATH;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_uri_query(th_request* request, th_string path)
{
    size_t pos = 0;
    while (pos != th_string_npos) {
        th_string key;
        th_string value;
        th_err err = th_request_parser_do_next_queryvar(path, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if (th_request_add_queryvar(request, key, value) != TH_ERR_OK) {
            return TH_ERR_BAD_ALLOC;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_next_path_segment(th_string buffer, th_string* segment, size_t* parsed)
{
    static const int uri_char[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 32-47 (don't allow space, ")
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 48-57 (0-9)
        1, 1, 0, 1, 0, 1, 1,                            // 58-64 (don't allow <,>)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 65-80 (A-P)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 81-90 (Q-Z)
        0, 0, 0, 0, 1, 0,                               // 91-96 (don't allow [, \, ], ^, `)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 97-112 (a-p)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 113-122 (q-z)
        0, 0, 0, 1, 0,                                  // 123-127 (don't allow {, |, }, DEL)
        // implicitely set to 0 for 128-255
    };
    size_t i = 0;
    while (i < buffer.len && buffer.ptr[i] != ' ' && buffer.ptr[i] != '?') {
        if (uri_char[(unsigned char)buffer.ptr[i]] == 0)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        i++;
    }
    if (i == buffer.len)
        return TH_ERR_OK;
    *segment = th_string_substr(buffer, 0, i);
    *parsed = i + 1;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_path(th_request_parser* parser, th_request* request, th_string path, size_t* parsed)
{
    th_string segment;
    size_t uri_parsed = 0;
    th_err err = th_request_parser_next_path_segment(path, &segment, &uri_parsed);
    if (err != TH_ERR_OK || uri_parsed == 0)
        return err;
    if ((err = th_request_set_uri_path(request, segment)) != TH_ERR_OK)
        return err;
    if (segment.ptr[segment.len] == '?') { // got a query
        size_t query_parsed = 0;
        err = th_request_parser_next_path_segment(th_string_substr(path, uri_parsed, th_string_npos), &segment, &query_parsed);
        if (err != TH_ERR_OK || query_parsed == 0)
            return err;
        if ((err = th_request_set_uri_query(request, segment)) != TH_ERR_OK)
            return err;
        if ((err = th_request_parser_do_uri_query(request, segment)) != TH_ERR_OK) {
            // If we can't parse the query, that's ok, we just ignore it
            // restore the original state and continue
            th_request_clear_queryvars(request);
        }
        uri_parsed += query_parsed;
    } else {
        if ((err = th_request_set_uri_query(request, TH_STRING(""))) != TH_ERR_OK)
            return err;
    }
    *parsed = uri_parsed;
    parser->state = TH_REQUEST_PARSER_STATE_VERSION;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_version(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    size_t n = th_string_find_first(buffer, 0, '\r');
    if (n == th_string_npos || n + 1 == buffer.len)
        return TH_ERR_OK;
    if (buffer.ptr[n + 1] != '\n')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string version = th_string_substr(buffer, 0, n);
    if (version.len < 8)
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[0] != 'H')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[1] != 'T')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[2] != 'T')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[3] != 'P')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[4] != '/')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[5] != '1')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[6] != '.')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[7] < '0' || version.ptr[7] > '9')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_request_set_version(request, version.ptr[7] - '0');
    *parsed = n + 2;
    parser->state = TH_REQUEST_PARSER_STATE_HEADERS;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parse_handle_header(th_request_parser* parser, th_request* request, th_string name, th_string value)
{
    char arena[1024] = {0};
    th_arena_allocator arena_allocator;
    th_arena_allocator_init(&arena_allocator, arena, sizeof(arena), NULL);
    th_heap_string normalized_name;
    th_heap_string_init(&normalized_name, &arena_allocator.base);
    if (th_heap_string_set(&normalized_name, name) != TH_ERR_OK) {
        // This can only happen if the name is too long
        return TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    th_heap_string_to_lower(&normalized_name);
    th_header_id id = th_header_id_from_string(th_heap_string_data(&normalized_name), th_heap_string_len(&normalized_name));
    switch (id) {
    case TH_HEADER_ID_COOKIE:
        return th_request_parser_do_cookie_list(request, value);
    case TH_HEADER_ID_CONTENT_LENGTH:
        return th_string_to_uint(value, (unsigned*)&parser->content_len);
    case TH_HEADER_ID_CONNECTION:
        if (th_string_eq(value, TH_STRING("close"))) {
            request->close = true;
        } else if (th_string_eq(value, TH_STRING("keep-alive"))) {
            request->close = false;
        }
        return TH_ERR_OK;
    case TH_HEADER_ID_CONTENT_TYPE:
        if (th_string_eq(value, TH_STRING("application/x-www-form-urlencoded"))) {
            parser->body_encoding = TH_REQUEST_BODY_ENCODING_FORM_URL_ENCODED;
        } else if (th_string_eq(th_string_substr(value, 0, 19), TH_STRING("multipart/form-data"))) {
            parser->body_encoding = TH_REQUEST_BODY_ENCODING_MULTIPART_FORM_DATA;
        }
        break;
    default:
        break;
    }
    return th_request_add_header(request, th_heap_string_view(&normalized_name), value);
}

TH_LOCAL(th_err)
th_request_parser_parse_header_line(th_string line, th_string* out_name, th_string* out_value)
{
    th_err err = TH_ERR_OK;
    size_t parsed = 0;
    if ((err = th_request_parser_next_token(line, out_name, ':', &parsed)) != TH_ERR_OK)
        return err;
    if (parsed == 0)
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string header_value = th_string_substr(line, parsed, th_string_npos);
    if (!th_request_parser_is_printable_string(header_value))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *out_value = th_string_trim(header_value);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_header(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    size_t n = th_string_find_first(buffer, 0, '\r');
    if (n == th_string_npos || n + 1 == buffer.len)
        return TH_ERR_OK;
    if (buffer.ptr[n + 1] != '\n')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (n == 0) {
        *parsed = 2;
        if (parser->content_len == 0) {
            th_request_set_body(request, th_string_make(&buffer.ptr[2], 0));
            parser->state = TH_REQUEST_PARSER_STATE_DONE;
        } else {
            if (request->method == TH_METHOD_GET || request->method == TH_METHOD_HEAD)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            parser->state = TH_REQUEST_PARSER_STATE_BODY;
        }
        return TH_ERR_OK;
    }
    size_t key_parsed = 0;
    th_string key;
    th_err err = TH_ERR_OK;
    if ((err = th_request_parser_next_token(buffer, &key, ':', &key_parsed)) != TH_ERR_OK
        || key_parsed == 0)
        return err;
    th_string value = th_string_substr(buffer, key_parsed, n - key_parsed);
    if (!th_request_parser_is_printable_string(value))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if ((err = th_request_parse_handle_header(parser, request, th_string_trim(key), th_string_trim(value)))
        != TH_ERR_OK)
        return err;
    *parsed = n + 2;
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_request_parser_multipart_find_eol(th_string buffer, size_t start)
{
    size_t pos = start;
    while (pos + 1 < buffer.len) {
        if (buffer.ptr[pos] == '\r' && buffer.ptr[pos + 1] == '\n')
            return pos;
        pos++;
    }
    return th_string_npos;
}

TH_LOCAL(bool)
th_request_parser_multipart_is_boundary_line(th_string line, th_string boundary, bool* last)
{
    *last = false;
    if (line.len < boundary.len + 2)
        return false;
    if (line.ptr[0] != '-' || line.ptr[1] != '-')
        return false;
    if (th_string_eq(th_string_substr(line, 2, boundary.len), boundary)) {
        if (line.len == boundary.len + 2)
            return true;
        if (line.ptr[boundary.len + 2] == '-' && line.ptr[boundary.len + 3] == '-') {
            *last = true;
            return true;
        }
    }
    return false;
}

TH_LOCAL(th_err)
th_request_parser_multipart_next_header_param(th_string buffer, th_string* out_name, th_string* out_value, size_t* out_parsed)
{
    // skip leading spaces
    buffer = th_string_substr(buffer, th_string_find_first_not(buffer, 0, ' '), th_string_npos);
    size_t eq = th_string_find_first_of(buffer, 0, "=; ");
    if (eq == th_string_npos || buffer.ptr[eq] == ';') {
        *out_name = th_string_substr(buffer, 0, eq);
        *out_value = th_string_make_empty();
        *out_parsed = eq == th_string_npos ? buffer.len : eq + 1;
        return TH_ERR_OK;
    }
    if (buffer.ptr[eq] == ' ') // spaces are not allowed
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *out_name = th_string_substr(buffer, 0, eq);
    size_t parsed = eq + 1;
    buffer = th_string_substr(buffer, eq + 1, th_string_npos);
    if (th_string_empty(buffer)) // equals sign must be followed by a value
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (buffer.ptr[0] == '"') {
        size_t end = th_string_find_first(buffer, 1, '"');
        if (end == th_string_npos) // no closing quote
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *out_value = th_string_substr(buffer, 1, end - 1);
        parsed += (end == th_string_npos ? buffer.len : end + 1);
    } else {
        size_t end = th_string_find_first_of(buffer, 0, "; ");
        if (end != th_string_npos && buffer.ptr[end] == ' ')
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *out_value = th_string_substr(buffer, 0, end);
        parsed += (end == th_string_npos ? buffer.len : end + 1);
    }
    *out_parsed = parsed;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_multipart_content_disposition(th_string header_value, th_string* out_name, th_string* out_filename)
{
    // skip heading
    header_value = th_string_substr(header_value, th_string_find_first(header_value, 0, ';') + 1, th_string_npos);
    // parse the parameters
    while (!th_string_empty(header_value)) {
        th_err err = TH_ERR_OK;
        th_string name, value = th_string_make_empty();
        size_t parsed = 0;
        if ((err = th_request_parser_multipart_next_header_param(header_value, &name, &value, &parsed)) != TH_ERR_OK)
            return err;
        header_value = th_string_substr(header_value, parsed, th_string_npos);
        if (th_string_eq(name, TH_STRING("name"))) {
            *out_name = value;
        } else if (th_string_eq(name, TH_STRING("filename"))) {
            *out_filename = value;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_request_parser_multipart_find_boundary(th_string buffer, th_string boundary, bool* last, size_t* length)
{
    TH_ASSERT(length && "lenght pointer must not be NULL");
    size_t pos = 0;
    while (1) {
        size_t eol = th_request_parser_multipart_find_eol(buffer, pos);
        if (eol == th_string_npos)
            return th_string_npos;
        th_string line = th_string_substr(buffer, pos, eol - pos);
        if (th_request_parser_multipart_is_boundary_line(line, boundary, last)) {
            *length = line.len;
            break;
        }
        pos = eol + 2;
    }
    return pos;
}

TH_LOCAL(th_err)
th_request_parser_multipart_do_next(th_request* request, th_string buffer, th_string boundary, size_t* out_parsed)
{
    th_string content_disposition, content_type;
    content_disposition = content_type = th_string_make_empty();
    size_t content_len = th_string_npos;
    size_t original_len = buffer.len;
    // parse the headers
    while (1) {
        if (th_string_empty(buffer))
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        size_t line_length = th_request_parser_multipart_find_eol(buffer, 0);
        if (line_length == th_string_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        th_string line = th_string_substr(buffer, 0, line_length);
        if (th_string_empty(line)) {
            buffer = th_string_substr(buffer, line_length + 2, th_string_npos);
            break; // end of headers
        }
        th_string header_name;
        th_string header_value;
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_parse_header_line(line, &header_name, &header_value)) != TH_ERR_OK)
            return err;
        if (th_string_eq(header_name, TH_STRING("Content-Disposition"))) {
            content_disposition = header_value;
        } else if (th_string_eq(header_name, TH_STRING("Content-Length"))) {
            if (th_string_to_uint(header_value, (unsigned*)&content_len) != TH_ERR_OK)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        } else if (th_string_eq(header_name, TH_STRING("Content-Type"))) {
            content_type = header_value;
        }
        buffer = th_string_substr(buffer, line_length + 2, th_string_npos);
    }
    if (th_string_empty(content_disposition))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string name, filename;
    name = filename = th_string_make_empty();
    th_err err = TH_ERR_OK;
    if ((err = th_request_parser_multipart_content_disposition(content_disposition, &name, &filename)) != TH_ERR_OK)
        return err;
    if (th_string_empty(name))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    bool last = false;
    th_string content = th_string_make_empty();
    if (content_len != th_string_npos) {
        content = th_string_substr(buffer, 0, content_len);
        buffer = th_string_substr(buffer, content_len, th_string_npos);
        // check the boundary
        if (buffer.ptr[0] != '\r' || buffer.ptr[1] != '\n')
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        th_string line = th_string_substr(buffer, 2, th_request_parser_multipart_find_eol(buffer, 0));
        if (!th_request_parser_multipart_is_boundary_line(line, boundary, &last))
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        buffer = th_string_substr(buffer, content_len + boundary.len + 2, th_string_npos);
    } else {
        // we don't have the content length, so we need to find the boundary
        size_t boundary_length = 0;
        size_t pos = th_request_parser_multipart_find_boundary(buffer, boundary, &last, &boundary_length);
        if (pos == th_string_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        content = th_string_substr(buffer, 0, pos - 2); // -2 to remove the \r\n
        buffer = th_string_substr(buffer, pos + boundary_length + 2, th_string_npos);
    }
    if (last && !th_string_empty(buffer)) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    if (th_string_empty(filename)) {
        if (th_request_add_formvar(request, name, content) != TH_ERR_OK)
            return TH_ERR_BAD_ALLOC;
    } else {
        if (th_request_add_upload(request, content, name, filename, content_type) != TH_ERR_OK)
            return TH_ERR_BAD_ALLOC;
    }
    *out_parsed = original_len - buffer.len;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parsed_multipart_parse_content_type(th_string content_type, th_string* boundary)
{
    // skip heading
    content_type = th_string_substr(content_type, th_string_find_first(content_type, 0, ';') + 1, th_string_npos);
    while (!th_string_empty(content_type)) {
        th_string name, value = th_string_make_empty();
        size_t parsed = 0;
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_multipart_next_header_param(content_type, &name, &value, &parsed)) != TH_ERR_OK)
            return err;
        content_type = th_string_substr(content_type, parsed, th_string_npos);
        if (th_string_eq(name, TH_STRING("boundary"))) {
            *boundary = value;
            return TH_ERR_OK;
        }
    }
    return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
}

TH_LOCAL(th_err)
th_request_parser_do_multipart_form_data(th_request* request, th_string body)
{
    // first, read the boundary
    th_string content_type = th_request_get_header(request, TH_STRING("content-type"));
    if (th_string_empty(content_type)) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    th_err err = TH_ERR_OK;
    th_string boundary = th_string_make_empty();
    if ((err = th_request_parsed_multipart_parse_content_type(content_type, &boundary)) != TH_ERR_OK)
        return err;
    if (th_string_empty(boundary))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    // parse the body
    // first, find the first boundary
    bool last = false;
    size_t pos = th_request_parser_multipart_find_eol(body, 0);
    if (!th_request_parser_multipart_is_boundary_line(th_string_substr(body, 0, pos), boundary, &last)
        || last) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    body = th_string_substr(body, pos + 2, th_string_npos);
    do {
        size_t parsed = 0;
        if ((err = th_request_parser_multipart_do_next(request, body, boundary, &parsed)) != TH_ERR_OK) {
            return err;
        }
        body = th_string_substr(body, parsed, th_string_npos);
    } while (!th_string_empty(body));
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_body(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    if (buffer.len < parser->content_len) {
        *parsed = 0;
        return TH_ERR_OK;
    }
    // Got the whole body
    th_string body = th_string_substr(buffer, 0, parser->content_len);
    if (parser->body_encoding == TH_REQUEST_BODY_ENCODING_FORM_URL_ENCODED) {
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_do_bodyvars(request, body)) != TH_ERR_OK)
            return err;
    } else if (parser->body_encoding == TH_REQUEST_BODY_ENCODING_MULTIPART_FORM_DATA) {
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_do_multipart_form_data(request, body)) != TH_ERR_OK)
            return err;
    }
    th_request_set_body(request, body);
    *parsed = parser->content_len;
    parser->state = TH_REQUEST_PARSER_STATE_DONE;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_parse_next(th_request_parser* parser, th_request* request, th_string data, size_t* parsed)
{
    switch (parser->state) {
    case TH_REQUEST_PARSER_STATE_METHOD:
        return th_request_parser_do_method(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_PATH:
        return th_request_parser_do_path(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_VERSION:
        return th_request_parser_do_version(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_HEADERS:
        return th_request_parser_do_header(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_BODY:
        return th_request_parser_do_body(parser, request, data, parsed);
    default:
        *parsed = 0;
        break;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_request_parser_parse(th_request_parser* parser, th_request* request, th_string data, size_t* parsed)
{
    th_err err = TH_ERR_OK;
    while (data.len > 0) {
        size_t p = 0;
        if ((err = th_request_parser_parse_next(parser, request, th_string_substr(data, p, data.len), &p)) != TH_ERR_OK) {
            *parsed = p;
            return err;
        }
        data.ptr += p;
        data.len -= p;
        *parsed += p;
        if (p == 0 || parser->state == TH_REQUEST_PARSER_STATE_DONE) {
            return TH_ERR_OK;
        }
    }
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_request_parser_header_done(th_request_parser* parser)
{
    return parser->state > TH_REQUEST_PARSER_STATE_HEADERS;
}

TH_PRIVATE(bool)
th_request_parser_done(th_request_parser* parser)
{
    return parser->state == TH_REQUEST_PARSER_STATE_DONE;
}
