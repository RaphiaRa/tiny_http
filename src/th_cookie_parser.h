#ifndef TH_COOKIE_PARSER_H
#define TH_COOKIE_PARSER_H

#include <th.h>

#include "th_config.h"
#include "th_str.h"

#include <stddef.h>

/** th_cookie_parser
 * @brief Incremental parser over a Cookie request header value
 * (RFC 6265 section 4.2.1: cookie-string = cookie-pair *( ";" SP cookie-pair )).
 * Non-owning: the underlying bytes must outlive the parser. Call
 * th_cookie_parser_next repeatedly until th_cookie_parser_done is true.
 */
typedef struct th_cookie_parser {
    th_str str;
    size_t pos;
} th_cookie_parser;

/** th_cookie_parser_init
 * @brief Initializes parser to walk cookie_header from the start.
 */
TH_PRIVATE(void)
th_cookie_parser_init(th_cookie_parser* parser, th_str cookie_header);

/** th_cookie_parser_done
 * @brief Returns true once the whole header has been consumed - either by
 * th_cookie_parser_next reaching the end, or after it has returned an error.
 * No more pairs remain to be parsed either way.
 */
TH_PRIVATE(bool)
th_cookie_parser_done(const th_cookie_parser* parser);

/** th_cookie_parser_next
 * @brief Parses the next "name=value"
 *
 * cookie-name is validated against RFC 2616's token (no CTLs, and none of
 * the separators "()<>@,;:\"/[]?={} SP HT). 
 * 
 * cookie-value is validated against RFC 6265's cookie-octet 
 * (%x21 / %x23-2B / %x2D-3A / %x3C-5B / %x5D-7E - printable ASCII minus space, DQUOTE, comma, semicolon,
 * backslash), or the quoted form (DQUOTE *cookie-octet DQUOTE), with the
 * surrounding DQUOTEs stripped.
 *
 * @return TH_ERR_OK on success, with *key / *value filled.
 * @return TH_ERR_HTTP(TH_CODE_BAD_REQUEST)
 */
TH_PRIVATE(th_err)
th_cookie_parser_next(th_cookie_parser* parser, th_str* key, th_str* value);

#endif
