#ifndef TH_REQUEST_PARSER_H
#define TH_REQUEST_PARSER_H

#include <th.h>

#include "th_config.h"
#include "th_request.h"

#include <stddef.h>

typedef enum th_request_parser_state {
    TH_REQUEST_PARSER_STATE_METHOD,
    TH_REQUEST_PARSER_STATE_PATH,
    TH_REQUEST_PARSER_STATE_VERSION,
    TH_REQUEST_PARSER_STATE_HEADERS,
    TH_REQUEST_PARSER_STATE_BODY,
    TH_REQUEST_PARSER_STATE_DONE
} th_request_parser_state;

typedef enum th_request_body_encoding {
    TH_REQUEST_BODY_ENCODING_NONE,
    TH_REQUEST_BODY_ENCODING_FORM_URL_ENCODED,
    TH_REQUEST_BODY_ENCODING_MULTIPART_FORM_DATA
} th_request_body_encoding;

typedef struct th_request_parser {
    size_t content_len;
    th_request_parser_state state;
    th_request_body_encoding body_encoding;
} th_request_parser;

TH_PRIVATE(void)
th_request_parser_init(th_request_parser* parser);

TH_PRIVATE(void)
th_request_parser_reset(th_request_parser* parser);

TH_PRIVATE(size_t)
th_request_parser_content_len(th_request_parser* parser);

TH_PRIVATE(th_err)
th_request_parser_parse(th_request_parser* parser, th_request* request, th_string data, size_t* parsed);

TH_PRIVATE(bool)
th_request_parser_header_done(th_request_parser* parser);

TH_PRIVATE(bool)
th_request_parser_done(th_request_parser* parser);

#endif
