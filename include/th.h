#ifndef TH_H
#define TH_H

#include <stdbool.h>
#include <stddef.h>

/* th_allocator declarations begin */

/** th_allocator
 * @brief Allocator interface.
 * Library user can implement this interface and pass it to th_default_allocator_set
 * to override the default allocator behavior of the library globally,
 * or pass it to th_server_create to override the default allocator behavior of a specific server instance.
 */
typedef struct th_allocator {
    void* (*alloc)(void* self, size_t size);
    void* (*realloc)(void* self, void* ptr, size_t size);
    void (*free)(void* self, void* ptr);
} th_allocator;

/** th_default_allocator_set
 * @brief Override the default allocator globally.
 * Set to NULL to revert to the internal default allocator.
 */
void th_default_allocator_set(th_allocator* allocator);

/* th_allocator declarations end */
/* th_string declarations begin */

/** th_buffer
 * @brief Represents a buffer of data.
 */
typedef struct th_buffer {
    const char* ptr;
    size_t len;
} th_buffer;

/* th_string declarations end */
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

/* pack error category and code into a single integer */
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

#define TH_LOG_LEVEL_DEBUG 0
#define TH_LOG_LEVEL_INFO 1
#define TH_LOG_LEVEL_WARN 2
#define TH_LOG_LEVEL_ERROR 3
#define TH_LOG_LEVEL_FATAL 4
#define TH_LOG_LEVEL_NONE 5

/** th_log
 * @brief Log interface. Library user can implement this interface and pass it to th_log_set
 * to override the logging behavior.
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
    TH_METHOD_HEAD,
    TH_METHOD_OPTIONS,
    TH_METHOD_CONNECT,
    TH_METHOD_TRACE,
    TH_METHOD_ANY,
    TH_METHOD_MAX,
} th_method;

typedef enum th_code {
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
    TH_CODE_TOO_MANY_REQUESTS = 429,
    TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    TH_CODE_INTERNAL_SERVER_ERROR = 500,
    TH_CODE_NOT_IMPLEMENTED = 501,
    TH_CODE_SERVICE_UNAVAILABLE = 503,
} th_code;

/** th_listener_opt
 * @brief Listener options.
 */
typedef struct th_listener_opt {
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
} th_listener_opt;

/* datetime related declarations begin */
/** th_date
 * @brief Date struct corresponding to the Date header in HTTP.
 * Always in GMT.
 */
typedef struct th_date {
    unsigned year : 16;
    unsigned month : 8;
    unsigned day : 8;
    unsigned weekday : 8;
    unsigned hour : 8;
    unsigned minute : 8;
    unsigned second : 8;
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

typedef struct th_key_value {
    const char* key;
    const char* value;
} th_key_value;

typedef struct th_map th_map;
typedef const th_map* th_map_ref;

/** th_map_iter
 * @brief map iterator - Simply a pointer to a key-value pair.
 */
typedef const th_key_value* th_map_iter;

/** th_map_find
 * @brief Find an entry in the map by key.
 * @return Pointer to the key-value pair if found, otherwise NULL.
 */
th_map_iter th_map_find(th_map* map, const char* key);

/** th_map_begin
 * @brief Get the first entry in the map.
 * @return Pointer to the first key-value pair.
 */
th_map_iter th_map_begin(th_map* map);

/** th_map_end
 * @brief Get the end of the map.
 * @return Pointer to the end of the map.
 */
th_map_iter th_map_end(th_map* map);

/** th_map_next
 * @brief Get the next entry in the map.
 * @param map Map instance.
 * @param iter Current iterator.
 * @return Pointer to the next key-value pair in the map or NULL if the end of the map is reached.
 */
th_map_iter th_map_next(th_map* map, th_map_iter iter);

typedef struct th_request th_request;

/** th_get_method
 * @brief Get the method of the request.
 */
th_method th_get_method(const th_request* req);

/** th_get_headers
 * @brief Get the headers of the request.
 * @return Map of headers.
 */
th_map* th_get_headers(const th_request* req);

/** th_try_get_header
 * @brief Get a header value its name.
 * @return Header value if found, otherwise NULL.
 */
const char* th_try_get_header(const th_request* req, const char* name);

/** th_get_cookies
 * @brief Get the cookies of the request.
 * @return Map of cookies.
 */
th_map* th_get_cookies(const th_request* req);

/** th_try_get_cookie
 * @brief Get a cookie value by its name.
 * @return Cookie value if found, otherwise NULL.
 */
const char* th_try_get_cookie(const th_request* req, const char* name);

/** th_get_query_params
 * @brief Get the query parameters of the request.
 * These are key-value pairs in the query string of the URL,
 * e.g. (name, value) in /path?name=value.
 * @return Map of query parameters.
 */
th_map* th_get_query_params(const th_request* req);

/** th_try_get_query_param
 * @brief Get a query parameter value by its key.
 * @return Query parameter value if found, otherwise NULL.
 */
const char* th_try_get_query_param(const th_request* req, const char* key);

/** th_get_body_params
 * @brief Get the body parameters of the request.
 * These are key-value pairs in the body of the request. Typicaly used in POST requests.
 * @return Map of body parameters.
 */
th_map* th_get_body_params(const th_request* req);

/** th_try_get_body_param
 * @brief Get a body parameter value by its key.
 * @return Body parameter value if found, otherwise NULL.
 */
const char* th_try_get_body_param(const th_request* req, const char* key);

/** th_get_path_params
 * @brief Get the path parameters of the request.
 * These are the key-value pairs associated with a captured path segment.
 * @return Map of path parameters.
 */
th_map* th_get_path_params(const th_request* req);

/** th_try_get_path_param
 * @brief Get a path parameter value by its key.
 * @return Path parameter value if found, otherwise NULL.
 */
const char* th_try_get_path_param(const th_request* req, const char* key);

/** th_get_path
 * @brief Get the path of the request.
 */
const char* th_get_path(const th_request* req);

/** th_get_query
 * @brief Get the query string of the request.
 * e.g. the part after the ? in /path?query.
 */
const char* th_get_query(const th_request* req);

/** th_get_body
 * @brief Get the body of the request.
 * @return Buffer containing the body, this is simply a pointer and length pair.
 */
th_buffer th_get_body(const th_request* req);

typedef struct th_response th_response;

/** th_printf_body
 * @brief Set the body of the response using a printf-like format string.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_printf_body(th_response* resp, const char* fmt, ...);

/** th_set_body_from_file
 * @brief Set the body of the response from a file
 * @param root Label of the root directory that was added to the server via th_add_root.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_set_body_from_file(th_response* resp, const char* root, const char* filepath);

/** th_set_body_from_buffer
 * @brief Set the body of the response from a buffer.
 * @param buffer A pointer and length pair representing the buffer.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_set_body_from_buffer(th_response* resp, th_buffer buffer);

/** th_set_body
 * @brief Set the body of the response from a null-terminated string.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_set_body(th_response* resp, const char* body);

/** th_add_header
 * @brief Add a header to the response, where key and value are null-terminated strings.
 * This will not overwrite existing headers with the same key.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_add_header(th_response* resp, const char* key, const char* value);

/** th_add_cookie
 * @brief Add a cookie to the response.
 * This essentially sets a Set-Cookie header. If the cookie was already added it will be added again and NOT replaced.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_add_cookie(th_response* resp, const char* key, const char* value, th_cookie_attr* attr);

/** th_handler
 * @brief Request handler function type.
 *
 * This function is called by the server when a request is received.
 * The handler should fill the response object with the appropriate data.
 * And return TH_ERR_OK on success, otherwise an error code.
 */
typedef th_err (*th_handler)(void* userp, const th_request* req, th_response* resp);

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
th_err th_bind(th_server* server, const char* addr, const char* port, th_listener_opt* opt);

th_err th_route(th_server* server, th_method method, const char* route, th_handler handler, void* userp);

/** th_err
 * @brief Add a root directory to the server.
 * @param name Label for the root directory.
 * @param path File system path.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
th_err th_add_root(th_server* server, const char* name, const char* path);

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
