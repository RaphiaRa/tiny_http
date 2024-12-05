#ifndef TH_RESPONSE_H
#define TH_RESPONSE_H

#include <th.h>

#include <stdarg.h>

#include "th_allocator.h"
#include "th_config.h"
#include "th_fcache.h"
#include "th_header_id.h"
#include "th_heap_string.h"
#include "th_socket.h"
/* th_response begin */

// 3 = start line + headers + body
#define TH_RESPONSE_MAX_CHUNK_NUM 3

struct th_response {
    th_heap_string headers;
    th_heap_string body;
    th_iov iov[TH_RESPONSE_MAX_CHUNK_NUM];
    th_allocator* allocator;
    th_fcache* fcache;
    th_fcache_entry* fcache_entry;
    size_t file_len;
    th_code code;
    bool header_is_set[TH_HEADER_ID_MAX];
    bool is_file;
    // Set this to true if we have a HEAD request, so that we only write headers.
    bool only_headers;
};

TH_PRIVATE(void)
th_response_init(th_response* response, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_response_reset(th_response* response);

TH_PRIVATE(void)
th_response_set_code(th_response* response, th_code code);

TH_PRIVATE(th_err)
th_response_add_header(th_response* response, th_string key, th_string value);

TH_PRIVATE(th_err)
th_response_set_body(th_response* response, th_string body);

TH_PRIVATE(void)
th_response_deinit(th_response* response);

/* th_response end */

TH_PRIVATE(void)
th_response_async_write(th_response* response, th_socket* socket, th_io_handler* handler);

#endif
