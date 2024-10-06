#include "th_request_parser.h"

#include "th_header_id.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "request_parser"

TH_PRIVATE(void)
th_request_parser_init(th_request_parser* parser)
{
    parser->state = TH_REQUEST_PARSER_STATE_METHOD;
    parser->content_len = 0;
    parser->parse_body_params = false;
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
th_request_parser_do_next_query_param(th_string string, size_t* pos, th_string* key, th_string* value)
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
th_request_parser_do_body_params(th_request* request, th_string body)
{
    th_err err = TH_ERR_OK;
    size_t pos = 0;
    while (pos != th_string_npos) {
        th_string key;
        th_string value;
        err = th_request_parser_do_next_query_param(body, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if ((err = th_request_add_body_param(request, key, value)) != TH_ERR_OK) {
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
        th_err err = th_request_parser_do_next_query_param(path, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if (th_request_add_query_param(request, key, value) != TH_ERR_OK) {
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
            th_request_clear_query_params(request);
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

TH_PRIVATE(th_err)
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

TH_PRIVATE(th_err)
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
            parser->parse_body_params = true;
        } else if (th_string_eq(value, TH_STRING("multipart/form-data"))) {
            // not supported right now
            return TH_ERR_HTTP(TH_CODE_UNSUPPORTED_MEDIA_TYPE);
        }
        break;
    default:
        break;
    }
    return th_request_add_header(request, th_heap_string_view(&normalized_name), value);
}

TH_PRIVATE(th_err)
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

TH_PRIVATE(th_err)
th_request_parser_do_body(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    if (buffer.len < parser->content_len) {
        *parsed = 0;
        return TH_ERR_OK;
    }
    // Got the whole body
    th_string body = th_string_substr(buffer, 0, parser->content_len);
    if (parser->parse_body_params) {
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_do_body_params(request, body)) != TH_ERR_OK)
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
