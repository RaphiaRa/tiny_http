#ifndef TH_RESPONSE_H
#define TH_RESPONSE_H

#include <th.h>

#include <stdarg.h>

#include "th_allocator.h"
#include "th_config.h"
#include "th_dir_mgr.h"
#include "th_fcache.h"
#include "th_file.h"
#include "th_header_id.h"
#include "th_iov.h"
#include "th_string.h"
/* th_response begin */

// 3 = start line + headers + body
#define TH_RESPONSE_MAX_CHUNK_NUM 3

struct th_response {
    th_string headers;
    th_string body;
    th_iov iov[TH_RESPONSE_MAX_CHUNK_NUM];
    th_allocator* allocator;
    th_dir_mgr* dir_mgr;
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
th_response_init(th_response* response, th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_response_reset(th_response* response);

TH_PRIVATE(void)
th_response_set_code(th_response* response, th_code code);

TH_PRIVATE(th_err)
th_response_add_header(th_response* response, th_str key, th_str value);

TH_PRIVATE(th_err)
th_response_set_body(th_response* response, th_str body);

TH_PRIVATE(void)
th_response_deinit(th_response* response);

/* th_response end */

/** th_response_write_plan
 * @brief What to send for a response: iov always (start line + headers,
 * plus body if any), file/offset/len additionally if a file is being
 * sent (file is NULL otherwise).
 */
typedef struct th_response_write_plan {
    th_iov* iov;
    size_t iovcnt;
    th_file* file;
    size_t offset;
    size_t len;
} th_response_write_plan;

/** th_response_prepare_write
 * @brief Finalizes headers (default headers, start line) and fills out
 * plan with what to send. Does no I/O - the caller sends plan itself
 * (e.g. via th_conn_send).
 */
TH_PRIVATE(th_err)
th_response_prepare_write(th_response* response, th_response_write_plan* plan);

#endif
