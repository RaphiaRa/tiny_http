#ifndef TH_MULTIPART_PARSER_H
#define TH_MULTIPART_PARSER_H

#include <th.h>

#include "th_config.h"
#include "th_str.h"

#include <stddef.h>

/** th_multipart_part
 * @brief One part of a multipart/form-data body (RFC 7578). filename/
 * content_type are empty for a plain form field (no file upload).
 */
typedef struct th_multipart_part {
    th_str name;
    th_str filename;
    th_str content_type;
    th_str content;
} th_multipart_part;

/** th_multipart_parser
 * @brief Non-owning: the underlying bytes must outlive the parser.
 */
typedef struct th_multipart_parser {
    th_str body;
    th_str boundary;
    size_t pos;
} th_multipart_parser;

/** th_multipart_parser_boundary
 * @brief Extracts the boundary parameter (token or quoted-string form)
 * from a multipart/form-data Content-Type header value.
 * @return TH_ERR_OK, with *boundary filled.
 * @return TH_ERR_HTTP(TH_CODE_BAD_REQUEST) if missing or empty.
 */
TH_PRIVATE(th_err)
th_multipart_parser_boundary(th_str content_type, th_str* boundary);

/** th_multipart_parser_init
 * @brief boundary is the value from th_multipart_parser_boundary, without
 * the leading "--".
 * @return TH_ERR_HTTP(TH_CODE_BAD_REQUEST) if body doesn't open with the
 * boundary delimiter, or has no parts at all.
 */
TH_PRIVATE(th_err)
th_multipart_parser_init(th_multipart_parser* parser, th_str body, th_str boundary);

/** th_multipart_parser_done
 * @brief True once the closing delimiter is reached, or after an error.
 */
TH_PRIVATE(bool)
th_multipart_parser_done(const th_multipart_parser* parser);

/** th_multipart_parser_next
 * @brief Parses the next part. Must not be called once
 * th_multipart_parser_done is already true.
 * @return TH_ERR_OK, with *part filled.
 * @return TH_ERR_HTTP(TH_CODE_BAD_REQUEST) on a malformed part: missing
 * Content-Disposition/"name", a header line with no CRLF, or content whose
 * declared Content-Length isn't followed by CRLF + a boundary line.
 */
TH_PRIVATE(th_err)
th_multipart_parser_next(th_multipart_parser* parser, th_multipart_part* part);

#endif
