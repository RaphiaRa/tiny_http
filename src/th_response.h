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

#define TH_RESPONSE_MAX_IOV 32

/** th_last_chunk_type
 *  TH_CHUNK_TYPE_HEADER: The last chunk written was a header chunk.
 *  TH_CHUNK_TYPE_BODY: The last chunk written was a body chunk.
 *  The user can set headers after writing the body,
 *  we need to detect this case so that we can setup the
 *  buffers for the headers correctly.
 */
typedef enum th_last_chunk_type {
    TH_CHUNK_TYPE_NONE,
    TH_CHUNK_TYPE_HEADER,
    TH_CHUNK_TYPE_BODY,
} th_last_chunk_type;

struct th_response {
    th_context* context;
    th_allocator* allocator;
    th_iov iov[TH_RESPONSE_MAX_IOV];
    th_iov* header_buf;
    size_t cur_header_buf_len;
    size_t cur_header_buf_pos;

    char header_is_set[TH_HEADER_ID_MAX];
    th_heap_string body;
    int is_file;
    th_fcache* fcache;
    th_fcache_entry* fcache_entry;
    size_t file_len;
    th_last_chunk_type last_chunk_type;
    th_code code;
    int minor_version;
};

TH_PRIVATE(void)
th_response_init(th_response* response, th_fcache* fcache, th_allocator* allocator);

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
