#include "th_multipart_parser.h"

#include "th_utility.h"

TH_LOCAL(th_err)
th_multipart_parser_next_header_param(th_str buffer, th_str* out_name, th_str* out_value, size_t* out_parsed)
{
    buffer = th_str_substr(buffer, th_str_find_first_not(buffer, 0, ' '), th_str_npos);
    size_t eq = th_str_find_first_of(buffer, 0, "=; ");
    if (eq == th_str_npos || buffer.ptr[eq] == ';') {
        *out_name = th_str_substr(buffer, 0, eq);
        *out_value = th_str_make_empty();
        *out_parsed = eq == th_str_npos ? buffer.len : eq + 1;
        return TH_ERR_OK;
    }
    if (buffer.ptr[eq] == ' ') // spaces are not allowed
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *out_name = th_str_substr(buffer, 0, eq);
    size_t parsed = eq + 1;
    buffer = th_str_substr(buffer, eq + 1, th_str_npos);
    if (th_str_empty(buffer)) // equals sign must be followed by a value
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (buffer.ptr[0] == '"') {
        size_t end = th_str_find_first(buffer, 1, '"');
        if (end == th_str_npos) // no closing quote
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *out_value = th_str_substr(buffer, 1, end - 1);
        parsed += (end == th_str_npos ? buffer.len : end + 1);
    } else {
        size_t end = th_str_find_first_of(buffer, 0, "; ");
        if (end != th_str_npos && buffer.ptr[end] == ' ')
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *out_value = th_str_substr(buffer, 0, end);
        parsed += (end == th_str_npos ? buffer.len : end + 1);
    }
    *out_parsed = parsed;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_multipart_parser_boundary(th_str content_type, th_str* boundary)
{
    content_type = th_str_substr(content_type, th_str_find_first(content_type, 0, ';') + 1, th_str_npos);
    while (!th_str_empty(content_type)) {
        th_str name, value = th_str_make_empty();
        size_t parsed = 0;
        th_err err = TH_ERR_OK;
        if ((err = th_multipart_parser_next_header_param(content_type, &name, &value, &parsed)) != TH_ERR_OK)
            return err;
        content_type = th_str_substr(content_type, parsed, th_str_npos);
        if (th_str_eq(name, TH_STR("boundary"))) {
            if (th_str_empty(value))
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            *boundary = value;
            return TH_ERR_OK;
        }
    }
    return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
}

TH_LOCAL(size_t)
th_multipart_parser_find_eol(th_str buffer, size_t start)
{
    if (start + 1 >= buffer.len)
        return th_str_npos;
    th_str searchable = th_str_substr(buffer, 0, buffer.len - 1);
    size_t pos = start;
    while (pos != th_str_npos) {
        pos = th_str_find_first(searchable, pos, '\r');
        if (pos == th_str_npos)
            return th_str_npos;
        if (buffer.ptr[pos + 1] == '\n')
            return pos;
        pos++;
    }
    return th_str_npos;
}

TH_LOCAL(bool)
th_multipart_parser_is_boundary_line(th_str line, th_str boundary, bool* last)
{
    *last = false;
    if (line.len < boundary.len + 2)
        return false;
    if (line.ptr[0] != '-' || line.ptr[1] != '-')
        return false;
    if (th_str_eq(th_str_substr(line, 2, boundary.len), boundary)) {
        if (line.len == boundary.len + 2)
            return true;
        if (line.ptr[boundary.len + 2] == '-' && line.ptr[boundary.len + 3] == '-') {
            *last = true;
            return true;
        }
    }
    return false;
}

TH_PRIVATE(th_err)
th_multipart_parser_init(th_multipart_parser* parser, th_str body, th_str boundary)
{
    parser->body = body;
    parser->boundary = boundary;
    bool last = false;
    size_t eol = th_multipart_parser_find_eol(body, 0);
    if (!th_multipart_parser_is_boundary_line(th_str_substr(body, 0, eol), boundary, &last) || last) {
        parser->pos = th_str_npos;
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    parser->pos = eol + 2;
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_multipart_parser_done(const th_multipart_parser* parser)
{
    return parser->pos == th_str_npos;
}

TH_LOCAL(th_err)
th_multipart_parser_content_disposition(th_str header_value, th_str* out_name, th_str* out_filename)
{
    header_value = th_str_substr(header_value, th_str_find_first(header_value, 0, ';') + 1, th_str_npos);
    while (!th_str_empty(header_value)) {
        th_err err = TH_ERR_OK;
        th_str name, value = th_str_make_empty();
        size_t parsed = 0;
        if ((err = th_multipart_parser_next_header_param(header_value, &name, &value, &parsed)) != TH_ERR_OK)
            return err;
        header_value = th_str_substr(header_value, parsed, th_str_npos);
        if (th_str_eq(name, TH_STR("name"))) {
            *out_name = value;
        } else if (th_str_eq(name, TH_STR("filename"))) {
            *out_filename = value;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_multipart_parser_find_boundary(th_str buffer, th_str boundary, bool* last, size_t* length)
{
    TH_ASSERT(length && "length pointer must not be NULL");
    size_t pos = 0;
    while (1) {
        size_t eol = th_multipart_parser_find_eol(buffer, pos);
        if (eol == th_str_npos)
            return th_str_npos;
        th_str line = th_str_substr(buffer, pos, eol - pos);
        if (th_multipart_parser_is_boundary_line(line, boundary, last)) {
            *length = line.len;
            break;
        }
        pos = eol + 2;
    }
    return pos;
}

TH_LOCAL(th_err)
th_multipart_parser_headers(th_str* buffer, th_str* content_disposition, th_str* content_type, size_t* content_len)
{
    while (1) {
        if (th_str_empty(*buffer))
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        size_t line_length = th_multipart_parser_find_eol(*buffer, 0);
        if (line_length == th_str_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        th_str line = th_str_substr(*buffer, 0, line_length);
        if (th_str_empty(line)) {
            *buffer = th_str_substr(*buffer, line_length + 2, th_str_npos);
            return TH_ERR_OK; // end of headers
        }
        th_str header_name, header_value;
        th_err err = TH_ERR_OK;
        size_t colon = th_str_find_first(line, 0, ':');
        if (colon == th_str_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        header_name = th_str_trim(th_str_substr(line, 0, colon));
        header_value = th_str_trim(th_str_substr(line, colon + 1, th_str_npos));
        if (th_str_eq(header_name, TH_STR("Content-Disposition"))) {
            *content_disposition = header_value;
        } else if (th_str_eq(header_name, TH_STR("Content-Length"))) {
            unsigned int part_content_len = 0;
            if ((err = th_str_to_uint(header_value, &part_content_len)) != TH_ERR_OK)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            *content_len = part_content_len;
        } else if (th_str_eq(header_name, TH_STR("Content-Type"))) {
            *content_type = header_value;
        }
        *buffer = th_str_substr(*buffer, line_length + 2, th_str_npos);
    }
}

TH_LOCAL(th_err)
th_multipart_parser_content(
    th_multipart_parser* parser, th_str* buffer, size_t content_len, th_str* content, bool* last)
{
    if (content_len != th_str_npos) {
        *content = th_str_substr(*buffer, 0, content_len);
        *buffer = th_str_substr(*buffer, content_len, th_str_npos);
        if (buffer->len < 2 || buffer->ptr[0] != '\r' || buffer->ptr[1] != '\n')
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        size_t line_end = th_multipart_parser_find_eol(*buffer, 2);
        th_str line = th_str_substr(*buffer, 2, line_end == th_str_npos ? th_str_npos : line_end - 2);
        if (!th_multipart_parser_is_boundary_line(line, parser->boundary, last))
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *buffer = th_str_substr(*buffer, content_len + parser->boundary.len + 2, th_str_npos);
    } else {
        // we don't have the content length, so we need to find the boundary
        size_t boundary_length = 0;
        size_t pos = th_multipart_parser_find_boundary(*buffer, parser->boundary, last, &boundary_length);
        if (pos == th_str_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *content = th_str_substr(*buffer, 0, pos - 2); // -2 to remove the \r\n
        *buffer = th_str_substr(*buffer, pos + boundary_length + 2, th_str_npos);
    }
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_multipart_parser_next(th_multipart_parser* parser, th_multipart_part* part)
{
    th_str buffer = th_str_substr(parser->body, parser->pos, th_str_npos);
    size_t original_len = buffer.len;

    th_str content_disposition = th_str_make_empty();
    th_str content_type = th_str_make_empty();
    size_t content_len = th_str_npos;
    th_err err = th_multipart_parser_headers(&buffer, &content_disposition, &content_type, &content_len);
    if (err != TH_ERR_OK) {
        parser->pos = th_str_npos;
        return err;
    }
    if (th_str_empty(content_disposition)) {
        parser->pos = th_str_npos;
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }

    th_str name = th_str_make_empty();
    th_str filename = th_str_make_empty();
    if ((err = th_multipart_parser_content_disposition(content_disposition, &name, &filename)) != TH_ERR_OK) {
        parser->pos = th_str_npos;
        return err;
    }
    if (th_str_empty(name)) {
        parser->pos = th_str_npos;
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }

    bool last = false;
    th_str content = th_str_make_empty();
    if ((err = th_multipart_parser_content(parser, &buffer, content_len, &content, &last)) != TH_ERR_OK) {
        parser->pos = th_str_npos;
        return err;
    }
    if (last && !th_str_empty(buffer)) {
        parser->pos = th_str_npos;
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }

    part->name = name;
    part->filename = filename;
    part->content_type = content_type;
    part->content = content;

    parser->pos = last ? th_str_npos : parser->pos + (original_len - buffer.len);
    return TH_ERR_OK;
}
