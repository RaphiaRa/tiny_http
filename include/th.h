#ifndef TH_H
#define TH_H

#include <stdbool.h>
#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
#define TH_PRINTF_FMT(fmt_idx, args_idx) __attribute__((format(printf, fmt_idx, args_idx)))
#else
#define TH_PRINTF_FMT(fmt_idx, args_idx)
#endif

/* th_allocator declarations begin */

/** th_allocator
 * @brief Library user can implement this interface to override the default
 * memory allocation behavior. Can be passed to th_server_create, or
 * set as the global default allocator with th_default_allocator_set.
 */
typedef struct th_allocator {
    void* (*alloc)(void* self, size_t size);
    void* (*realloc)(void* self, void* ptr, size_t size);
    void (*free)(void* self, void* ptr);
} th_allocator;

/** th_default_allocator_set
 * @brief Set the global default allocator.
 */
void th_default_allocator_set(th_allocator* allocator);

/* th_allocator declarations end */

/** th_buffer
 * @brief Non-owning view of a buffer.
 */
typedef struct th_buffer {
    const char* ptr;
    size_t len;
} th_buffer;

/* error related declarations begin */

/* error categories */
#define TH_ERR_CATEGORY_OTHER 0  // other error, our default category
#define TH_ERR_CATEGORY_SYSTEM 1 // system error, corresponds to errno or GetLastError
#define TH_ERR_CATEGORY_HTTP 2   // http protocol error
#define TH_ERR_CATEGORY_SSL 3    // ssl error

/* other error codes */
#define TH_ERRC_OK 0
#define TH_ERRC_BAD_ALLOC 1
#define TH_ERRC_INVALID_ARG 2
#define TH_ERRC_EOF 3
#define TH_ERRC_NOSUPPORT 4
#define TH_ERRC_UNKNOWN 5
#define TH_ERRC_BUSY 6

/* error pack/unpack macros */
#define TH_ERR_CODE_BITS (sizeof(unsigned) * 8 - 4)
#define TH_ERR_CODE_MASK ((1 << TH_ERR_CODE_BITS) - 1)
#define TH_ERR_CATEGORY_SHIFT TH_ERR_CODE_BITS
#define TH_ERR_CODE_MAX TH_ERR_CODE_MASK
#define TH_ERR_CATEOGRY_MAX ((1 << (sizeof(unsigned) * 8 - TH_ERR_CODE_BITS)) - 1)

#define TH_ERR_PACK(category, code) (((category) << TH_ERR_CATEGORY_SHIFT) | (code))

typedef enum th_err {
    TH_ERR_OK = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, 0),
    TH_ERR_BAD_ALLOC = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, TH_ERRC_BAD_ALLOC),
    TH_ERR_INVALID_ARG = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, TH_ERRC_INVALID_ARG),
    TH_ERR_EOF = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, TH_ERRC_EOF),
    TH_ERR_NOSUPPORT = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, TH_ERRC_NOSUPPORT),
    TH_ERR_UNKNOWN = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, TH_ERRC_UNKNOWN),
    TH_ERR_BUSY = TH_ERR_PACK(TH_ERR_CATEGORY_OTHER, TH_ERRC_BUSY),
} th_err;

#define TH_ERR(category, code) (th_err)(TH_ERR_PACK(category, code))
#define TH_ERR_SYSTEM(code) TH_ERR(TH_ERR_CATEGORY_SYSTEM, code)
#define TH_ERR_HTTP(code) TH_ERR(TH_ERR_CATEGORY_HTTP, code)
#define TH_ERR_SSL(code) TH_ERR(TH_ERR_CATEGORY_SSL, code)
#define TH_ERR_CATEGORY(err) (err >> TH_ERR_CATEGORY_SHIFT)
#define TH_ERR_CODE(err) (err & TH_ERR_CODE_MASK)

/** th_strerror
 * @brief Get a string representation of an error.
 */
const char* th_strerror(th_err err);

/* error related declarations end */
/* log declarations begin */

#define TH_LOG_LEVEL_TRACE 0
#define TH_LOG_LEVEL_DEBUG 1
#define TH_LOG_LEVEL_INFO 2
#define TH_LOG_LEVEL_WARN 3
#define TH_LOG_LEVEL_ERROR 4
#define TH_LOG_LEVEL_FATAL 5
#define TH_LOG_LEVEL_NONE 6

/** th_log
 * @brief Library user can implement this interface to override the default
 * logging behavior.
 */
typedef struct th_log {
    /** print
     * @brief Print a log message.
     * @param self Log instance.
     * @param level Log level of the message. One of TH_LOG_LEVEL_*.
     * @param msg Log message, null-terminated.
     */
    void (*print)(void* self, int level, const char* msg);
} th_log;

/** th_log_set
 * @brief Set the global log instance.
 */
void th_log_set(th_log* log);

/* log declarations end */

typedef enum th_method {
    TH_METHOD_GET,
    TH_METHOD_POST,
    TH_METHOD_PUT,
    TH_METHOD_DELETE,
    TH_METHOD_PATCH,
    TH_METHOD_CONNECT,
    TH_METHOD_OPTIONS,
    TH_METHOD_TRACE,
    TH_METHOD_HEAD,
    TH_METHOD_ANY,
    TH_METHOD_MAX,
} th_method;

typedef enum th_code {
    TH_CODE_SWITCHING_PROTOCOLS = 101,
    TH_CODE_OK = 200,
    TH_CODE_MOVED_PERMANENTLY = 301,
    TH_CODE_BAD_REQUEST = 400,
    TH_CODE_UNAUTHORIZED = 401,
    TH_CODE_FORBIDDEN = 403,
    TH_CODE_NOT_FOUND = 404,
    TH_CODE_METHOD_NOT_ALLOWED = 405,
    TH_CODE_REQUEST_TIMEOUT = 408,
    TH_CODE_PAYLOAD_TOO_LARGE = 413,
    TH_CODE_URI_TOO_LONG = 414,
    TH_CODE_UNSUPPORTED_MEDIA_TYPE = 415,
    TH_CODE_RANGE_NOT_SATISFIABLE = 416,
    TH_CODE_UPGRADE_REQUIRED = 426,
    TH_CODE_TOO_MANY_REQUESTS = 429,
    TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    TH_CODE_INTERNAL_SERVER_ERROR = 500,
    TH_CODE_NOT_IMPLEMENTED = 501,
    TH_CODE_SERVICE_UNAVAILABLE = 503,
} th_code;

typedef enum th_prot_version {
    TH_HTTP_1_0 = 0,
    TH_HTTP_1_1 = 1,
} th_prot_version;

/** th_bind_opt
 * @brief Listener options.
 */
typedef struct th_bind_opt {
    /** cert_file
     * @brief Path to the PEM encoded certificate file.
     * If this and key_file are set, the listener will use SSL.
     */
    const char* cert_file;

    /** key_file
     * @brief Path to the PEM encoded private key file.
     * If this and cert_file are set, the listener will use SSL.
     */
    const char* key_file;
} th_bind_opt;

/* datetime related declarations begin */

/** th_date
 * @brief Date struct corresponding to the Date header in HTTP. Always in GMT.
 */
typedef struct th_date {
    unsigned int year : 16;
    unsigned int month : 8;
    unsigned int day : 8;
    unsigned int weekday : 8;
    unsigned int hour : 8;
    unsigned int minute : 8;
    unsigned int second : 8;
} th_date;

typedef struct th_duration {
    int seconds;
} th_duration;

th_duration th_seconds(int seconds);
th_duration th_minutes(int minutes);
th_duration th_hours(int hours);
th_duration th_days(int days);

th_date th_date_now(void);
th_date th_date_add(th_date date, th_duration d);

/* datetime related declarations end */
/* cookie related declarations begin */

typedef enum th_cookie_same_site {
    TH_COOKIE_SAME_SITE_UNSPECIFIED = 0,
    TH_COOKIE_SAME_SITE_NONE,
    TH_COOKIE_SAME_SITE_LAX,
    TH_COOKIE_SAME_SITE_STRICT,
} th_cookie_same_site;

typedef struct th_cookie_attr {
    th_date expires;
    th_duration max_age;
    const char* domain;
    const char* path;
    bool secure;
    bool http_only;
    th_cookie_same_site same_site;
} th_cookie_attr;

/* cookie related declarations end */
/* request related declarations begin */

typedef struct th_part th_part;

typedef struct th_iter_methods th_iter_methods;

typedef struct th_iter {
    th_iter_methods* methods;
    const void* ptr;
    const void* end;
} th_iter;

/** th_next
 * @brief Move the iterator to the next element.
 * @return True if there is a next element, false otherwise.
 */
bool th_next(th_iter* it);

// Returns the key associated with the current element.
const char* th_key(const th_iter* it);

// Returns the value associated with the current element.
const void* th_val(const th_iter* it);

// Same as (const char*)th_val(it)
const char* th_cval(const th_iter* it);

typedef struct th_request th_request;

const th_part* th_find_part(const th_request* req, const char* name);
th_iter th_part_iter(const th_request* req);

const char* th_part_name(const th_part* part);
const char* th_part_filename(const th_part* part);
const char* th_part_content_type(const th_part* part);
th_buffer th_part_content(const th_part* part);

const char* th_find_header(const th_request* req, const char* name);
th_iter th_header_iter(const th_request* req);

const char* th_find_cookie(const th_request* req, const char* name);
th_iter th_cookie_iter(const th_request* req);

const char* th_find_queryvar(const th_request* req, const char* name);
th_iter th_queryvar_iter(const th_request* req);

const char* th_find_formvar(const th_request* req, const char* name);
th_iter th_formvar_iter(const th_request* req);

const char* th_find_pathvar(const th_request* req, const char* name);
th_iter th_pathvar_iter(const th_request* req);

const char* th_get_path(const th_request* req);
const char* th_get_query(const th_request* req);
th_buffer th_get_body(const th_request* req);
th_method th_get_method(const th_request* req);
th_prot_version th_get_version(const th_request* req);

/* request related declarations end */
/* response related declarations begin */

typedef struct th_response th_response;

/** th_printf_body
 * @brief Set the body of the response from a printf-style format string.
 */
th_err th_printf_body(th_response* resp, const char* fmt, ...) TH_PRINTF_FMT(2, 3);

/** th_set_body_from_file
 * @brief Set the body of the response from a file.
 * @param dir_label Label of the root directory to use.
 * @param filepath Path to the file (relative to the root directory).
 */
th_err th_set_body_from_file(th_response* resp, const char* dir_label, const char* filepath);

/** th_set_body_from_buffer
 * @brief Set the body of the response from a buffer.
 */
th_err th_set_body_from_buffer(th_response* resp, th_buffer buffer);

/** th_set_body
 * @brief Set the body of the response from a null-terminated string.
 */
th_err th_set_body(th_response* resp, const char* body);

/** th_add_header
 * @brief Add a header to the response, where key and value are null-terminated strings.
 * If the header was already added it will be added again and NOT replaced.
 */
th_err th_add_header(th_response* resp, const char* key, const char* value);

/** th_add_cookie
 * @brief Add a cookie to the response, where key and value are null-terminated strings.
 * If the cookie was already added it will be added again and NOT replaced.
 */
th_err th_add_cookie(th_response* resp, const char* key, const char* value, th_cookie_attr* attr);

/** th_handler
 * @brief Request handler function, called when a request is received.
 */
typedef th_err (*th_handler)(void* userp, const th_request* req, th_response* resp);

/** th_ws_event
 * @brief The kind of event delivered to a th_ws_handler.
 */
typedef enum th_ws_event {
    TH_WS_EVENT_OPEN,  // connection upgraded, ready to send/receive
    TH_WS_EVENT_DATA,  // one complete message received (data holds its payload)
    TH_WS_EVENT_CLOSE, // connection closed, ws is no longer valid after this call returns
} th_ws_event;

typedef struct th_ws th_ws;

/** th_ws_type
 * @brief Text vs binary, for both received messages and th_ws_send.
 */
typedef enum th_ws_type {
    TH_WS_TEXT,
    TH_WS_BINARY,
} th_ws_type;

/** th_ws_handler
 * @brief WebSocket event callback. data is empty for TH_WS_EVENT_OPEN/CLOSE,
 * and holds one complete message's payload for TH_WS_EVENT_DATA - type is
 * only meaningful for TH_WS_EVENT_DATA. ws must not be used after
 * TH_WS_EVENT_CLOSE has been delivered.
 */
typedef th_err (*th_ws_handler)(void* userp, th_ws* ws, th_ws_event ev, th_buffer data, th_ws_type type);

/** th_ws_send
 * @brief Sends one WebSocket message. type selects the opcode (text vs
 * binary), it does not otherwise affect encoding - data is sent as-is.
 * @return TH_ERR_BUSY if a previous send on this connection hasn't
 * finished yet (retry once the next event is delivered), TH_ERR_INVALID_ARG
 * if the connection is closing/closed.
 */
th_err th_ws_send(th_ws* ws, th_buffer data, th_ws_type type);

/** th_ws_close
 * @brief Starts closing the connection. TH_WS_EVENT_CLOSE will be
 * delivered once the close handshake completes.
 */
th_err th_ws_close(th_ws* ws);

typedef struct th_server th_server;

/** th_server_create
 * @brief Create a new server instance.
 * @param server (out) th_server double pointer to store the server instance.
 * The server instance is created on success and must be destroyed with th_server_destroy.
 * @param allocator (optional) Allocator to use for the server instance.
 * If NULL the default allocator is used (malloc, realloc, free).
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_server_create(th_server** server, th_allocator* allocator);

/** th_bind
 * @brief Creates a new listener on the server that listens on the specified address and port.
 * @param server Server instance.
 * @param addr Address to bind to, should be an IP address or hostname.
 * @param port Port to bind to
 * @param opt (optional) Listener options, such as SSL certificates.
 */
th_err th_bind(th_server* server, const char* addr, const char* port, th_bind_opt* opt);

th_err th_route(th_server* server, th_method method, const char* route, th_handler handler, void* userp);

/** th_route_ws
 * @brief Registers a WebSocket endpoint at path. Any request to path with
 * a valid WebSocket handshake is upgraded, after which handler(userp, ...)
 * receives its events. To gate the upgrade (e.g. auth), also register a
 * plain th_route on the same path with TH_METHOD_GET: if it returns an
 * error, that error is sent as a normal HTTP response and no upgrade
 * happens; if it returns TH_ERR_OK (or is absent), the upgrade proceeds.
 */
th_err th_route_ws(th_server* server, const char* path, th_ws_handler handler, void* userp);

/** th_err
 * @brief Add a directory to the server (for serving or storing files).
 * @param name Label for the directory.
 * @param path File system path.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_add_dir(th_server* server, const char* name, const char* path);

/** th_save_to_disk
 * @brief Write data to a file inside one of the server's registered
 * directories (see th_add_dir).
 * @return TH_ERR_HTTP(TH_CODE_NOT_FOUND) if dir_label isn't registered,
 * otherwise an error from opening/writing the file.
 */
th_err th_save_to_disk(th_server* server, th_buffer data, const char* dir_label, const char* filepath);

/** th_poll
 * @brief Poll the server for any events and pending tasks.
 * Keep calling this function regularly to keep the server running.
 */
th_err th_poll(th_server* server, int timeout_ms);

/** th_server_destroy
 * @brief Destroy the server instance.
 */
void th_server_destroy(th_server* server);

#endif
