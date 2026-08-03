#include "th_cookie_parser.h"

TH_PRIVATE(void)
th_cookie_parser_init(th_cookie_parser* parser, th_str cookie_header)
{
    parser->str = cookie_header;
    parser->pos = cookie_header.len == 0 ? th_str_npos : 0;
}

TH_PRIVATE(bool)
th_cookie_parser_done(const th_cookie_parser* parser)
{
    return parser->pos == th_str_npos;
}

/* RFC 2616 section 2.2 token: no CTLs, no separators
 * "()<>@,;:\"/[]?={} \t". Used for cookie-name. */
static const int th_cookie_parser_name_char[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
    0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, // 32-47  !"#$%&'()*+,-./
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 48-63 0123456789:;<=>?
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 64-79 @ABCDEFGHIJKLMNO
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, // 80-95 PQRSTUVWXYZ[\]^_
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 96-111 `abcdefghijklmno
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, // 112-127 pqrstuvwxyz{|}~ DEL
    // implicitly 0 for 128-255
};

/* RFC 6265 section 4.1.1 cookie-octet: %x21 / %x23-2B / %x2D-3A / %x3C-5B /
 * %x5D-7E - printable ASCII minus space, DQUOTE, comma, semicolon,
 * backslash. Used for a bare (unquoted) cookie-value. */
static const int th_cookie_parser_value_char[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
    0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 32-47  !"#$%&'()*+,-./
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, // 48-63 0123456789:;<=>?
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 64-79 @ABCDEFGHIJKLMNO
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 80-95 PQRSTUVWXYZ[\]^_
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 96-111 `abcdefghijklmno
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 112-127 pqrstuvwxyz{|}~ DEL
    // implicitly 0 for 128-255
};

/* Same as th_cookie_parser_value_char, plus space - the quoted form exists
 * so servers can embed characters a bare cookie-value can't (project
 * decision, not literal RFC 6265). */
static const int th_cookie_parser_quoted_value_char[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
    1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 32-47  !"#$%&'()*+,-./
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, // 48-63 0123456789:;<=>?
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 64-79 @ABCDEFGHIJKLMNO
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, // 80-95 PQRSTUVWXYZ[\]^_
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 96-111 `abcdefghijklmno
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // 112-127 pqrstuvwxyz{|}~ DEL
    // implicitly 0 for 128-255
};

TH_LOCAL(bool)
th_cookie_parser_is_space(char c)
{
    return c == ' ' || c == '\t';
}

TH_LOCAL(size_t)
th_cookie_parser_skip_space(th_str str, size_t pos)
{
    while (pos < str.len && th_cookie_parser_is_space(str.ptr[pos])) {
        pos++;
    }
    return pos;
}

/* Scans a cookie-name: one or more token chars, followed by optional space.
 * Leaves *pos on '=' (the caller checks it's actually there). */
TH_LOCAL(th_err)
th_cookie_parser_scan_name(th_str str, size_t* pos, th_str* name)
{
    size_t start = *pos;
    while (*pos < str.len && th_cookie_parser_name_char[(unsigned char)str.ptr[*pos]]) {
        (*pos)++;
    }
    if (*pos == start) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    *name = th_str_substr(str, start, *pos - start);
    *pos = th_cookie_parser_skip_space(str, *pos);
    if (*pos >= str.len || str.ptr[*pos] != '=') {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    return TH_ERR_OK;
}

/* Scans a quoted cookie-value, starting at the opening DQUOTE. */
TH_LOCAL(th_err)
th_cookie_parser_scan_quoted_value(th_str str, size_t* pos, th_str* value)
{
    size_t start = *pos + 1;
    size_t i = start;
    while (i < str.len && th_cookie_parser_quoted_value_char[(unsigned char)str.ptr[i]]) {
        i++;
    }
    if (i >= str.len || str.ptr[i] != '"') {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    *value = th_str_substr(str, start, i - start);
    *pos = i + 1;
    return TH_ERR_OK;
}

/* Scans a bare (unquoted) cookie-value: zero or more cookie-octets. */
TH_LOCAL(th_err)
th_cookie_parser_scan_bare_value(th_str str, size_t* pos, th_str* value)
{
    size_t start = *pos;
    while (*pos < str.len && th_cookie_parser_value_char[(unsigned char)str.ptr[*pos]]) {
        (*pos)++;
    }
    *value = th_str_substr(str, start, *pos - start);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_cookie_parser_scan_value(th_str str, size_t* pos, th_str* value)
{
    if (*pos < str.len && str.ptr[*pos] == '"') {
        return th_cookie_parser_scan_quoted_value(str, pos, value);
    }
    return th_cookie_parser_scan_bare_value(str, pos, value);
}

/* After a pair, only space may remain before ';' or the end of input - any
 * other byte (e.g. a stray octet the value scan stopped on) is malformed. */
TH_LOCAL(th_err)
th_cookie_parser_scan_pair_end(th_str str, size_t* pos)
{
    *pos = th_cookie_parser_skip_space(str, *pos);
    if (*pos == str.len) {
        return TH_ERR_OK;
    }
    if (str.ptr[*pos] != ';') {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    (*pos)++;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_cookie_parser_next(th_cookie_parser* parser, th_str* key, th_str* value)
{
    size_t pos = th_cookie_parser_skip_space(parser->str, parser->pos);

    th_str name;
    th_err err = th_cookie_parser_scan_name(parser->str, &pos, &name);
    if (err != TH_ERR_OK) {
        parser->pos = th_str_npos;
        return err;
    }
    pos = th_cookie_parser_skip_space(parser->str, pos + 1); // skip '=' and space

    th_str raw_value;
    if ((err = th_cookie_parser_scan_value(parser->str, &pos, &raw_value)) != TH_ERR_OK) {
        parser->pos = th_str_npos;
        return err;
    }

    if ((err = th_cookie_parser_scan_pair_end(parser->str, &pos)) != TH_ERR_OK) {
        parser->pos = th_str_npos;
        return err;
    }

    parser->pos = pos == parser->str.len ? th_str_npos : pos;
    *key = name;
    *value = raw_value;
    return TH_ERR_OK;
}
