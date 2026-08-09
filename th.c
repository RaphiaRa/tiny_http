#define TH_WITH_AMALGAMATION 1
#include "th.h"
/* Start of th_config.h */

#if defined(__GNUC__)
// Buggy warning in GCC
// TODO: Check the exact versions where these warnings are fixed
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

/* feature configuration begin */

#ifndef TH_WITH_SSL
#define TH_WITH_SSL 0
#endif

#ifndef TH_WITH_SENDFILE
#define TH_WITH_SENDFILE 1
#endif

#ifndef TH_WITH_MMAP
#define TH_WITH_MMAP 1
#endif

#ifndef TH_LOG_LEVEL
#define TH_LOG_LEVEL TH_LOG_LEVEL_INFO
#endif

#ifndef TH_MAX_BODY_LEN
#define TH_MAX_BODY_LEN (4 * 1024 * 1024)
#endif

/* feature configuration end */

#if defined(__APPLE__)
#define TH_CONFIG_OS_OSX 1
#define TH_CONFIG_OS_POSIX 1
#define TH_CONFIG_OS_BSD 1
#endif

#if defined(__FreeBSD__)
#define TH_CONFIG_OS_FreeBSD 1
#define TH_CONFIG_OS_POSIX 1
#define TH_CONFIG_OS_BSD 1
#endif

#if defined(__NetBSD__)
#define TH_CONFIG_OS_NetBSD 1
#define TH_CONFIG_OS_POSIX 1
#define TH_CONFIG_OS_BSD 1
#endif

#if defined(__OpenBSD__)
#define TH_CONFIG_OS_OpenBSD 1
#define TH_CONFIG_OS_POSIX 1
#define TH_CONFIG_OS_BSD 1
#endif

#if defined(__linux__)
#define TH_CONFIG_OS_LINUX 1
#define TH_CONFIG_OS_POSIX 1
#endif

#if defined(_WIN32)
#define TH_CONFIG_OS_WIN 1
#endif

/* IO service config begin */

#if defined(TH_CONFIG_OS_POSIX)
#define TH_CONFIG_WITH_POLL 1
#endif

#if defined(TH_CONFIG_OS_OSX)
#define TH_CONFIG_WITH_KQUEUE 1
#endif

#if defined(TH_CONFIG_OS_FreeBSD)
#define TH_CONFIG_WITH_KQUEUE 1
#endif

/* IO service config end */
/* sendfile config begin */

#if TH_WITH_SENDFILE
#if defined(TH_CONFIG_OS_LINUX)
// #define TH_CONFIG_WITH_LINUX_SENDFILE 1
#elif defined(TH_CONFIG_OS_OSX) || defined(TH_CONFIG_OS_FreeBSD) || defined(TH_CONFIG_OS_NetBSD) || defined(TH_CONFIG_OS_OpenBSD)
#define TH_CONFIG_WITH_BSD_SENDFILE 1
#endif
#endif

/* Unused attribute is platform dependent */

#define TH_MAYBE_UNUSED __attribute__((unused))

/* Helper macros for defining public and private functions */

#define TH_PUBLIC(type) type

#ifdef TH_WITH_AMALGAMATION
#define TH_PRIVATE(type) static type
#else
#define TH_PRIVATE(type) type
#endif

#define TH_LOCAL(type) static type

#define TH_INLINE(type) static inline type

/* Server related config begin */

#ifndef TH_CONFIG_MAX_HANDLES
#define TH_CONFIG_MAX_HANDLES (8 * 1024)
#endif

#ifndef TH_CONFIG_MAX_CONNECTIONS
#define TH_CONFIG_MAX_CONNECTIONS 512
#endif

#ifndef TH_CONFIG_SMALL_HEADER_LEN
#define TH_CONFIG_SMALL_HEADER_LEN 1024
#endif

#ifndef TH_CONFIG_LARGE_HEADER_LEN
#define TH_CONFIG_LARGE_HEADER_LEN (4 * 1024)
#endif

#ifndef TH_CONFIG_MAX_CONTENT_LEN
#define TH_CONFIG_MAX_CONTENT_LEN (1024 * 1024)
#endif

#ifndef TH_CONFIG_MAX_HEADER_NUM
#define TH_CONFIG_MAX_HEADER_NUM 64
#endif

#ifndef TH_CONFIG_MAX_PATH_LEN
#define TH_CONFIG_MAX_PATH_LEN 512
#endif

#ifndef TH_CONFIG_MAX_CACHED_FDS
#define TH_CONFIG_MAX_CACHED_FDS 64
#endif

#ifndef TH_CONFIG_SENDFILE_CHUNK_LEN
#define TH_CONFIG_SENDFILE_CHUNK_LEN (4 * 1024 * 1024)
#endif

#ifndef TH_CONFIG_LARGE_BUF_LEN
#define TH_CONFIG_LARGE_BUF_LEN (4 * 1024)
#endif

#ifndef TH_CONFIG_SMALL_SSL_BUF_LEN
#define TH_CONFIG_SMALL_SSL_BUF_LEN (2 * 1024)
#endif

#ifndef TH_CONFIG_LARGE_SSL_BUF_LEN
#define TH_CONFIG_LARGE_SSL_BUF_LEN (8 * 1024)
#endif

#ifndef TH_CONFIG_MAX_SSL_READ_BUF_LEN
#define TH_CONFIG_MAX_SSL_READ_BUF_LEN (1024 * 1024)
#endif

#ifndef TH_CONFIG_MAX_SSL_WRITE_BUF_LEN
#define TH_CONFIG_MAX_SSL_WRITE_BUF_LEN (4 * 1024 * 1024)
#endif

/* Socket related configuration */

#ifndef TH_CONFIG_REUSE_ADDR
#define TH_CONFIG_REUSE_ADDR 1
#endif

#ifndef TH_CONFIG_REUSE_PORT
#define TH_CONFIG_REUSE_PORT 0
#endif

#ifndef TH_CONFIG_TCP_NODELAY
#define TH_CONFIG_TCP_NODELAY 0
#endif

// Socket timeout in seconds
#ifndef TH_CONFIG_IO_TIMEOUT
#define TH_CONFIG_IO_TIMEOUT 10
#endif

/* Server related config end */

/* End of th_config.h */
/* Start of th_str.h */

#include <stdint.h>
#include <string.h>



extern size_t th_str_npos;

typedef struct th_str {
    const char* ptr;
    size_t len;
} th_str;

/** th_str_make
 * @brief Helper function to create a th_str from a pointer and a length.
 */
TH_INLINE(th_str)
th_str_make(const char* ptr, size_t len)
{
    return (th_str){ptr, len};
}

/** th_str_make_empty
 * @brief Helper function to create an empty th_str.
 */
TH_INLINE(th_str)
th_str_make_empty(void)
{
    return (th_str){"", 0};
}

/** th_str_from_cstr
 * @brief Helper function to create a th_str from a null-terminated string.
 */
TH_INLINE(th_str)
th_str_from_cstr(const char* str)
{
    return th_str_make(str, strlen(str));
}

/** th_str_eq
 * @brief Helper function to compare two th_strs.
 * @return 1 if the strings are equal, 0 otherwise.
 */
TH_PRIVATE(bool)
th_str_eq(th_str a, th_str b);

/** th_str_empty
 * @brief Helper function to check if a th_str is empty.
 * @return true if the string is empty, false otherwise.
 */
TH_INLINE(bool)
th_str_empty(th_str str)
{
    return str.len == 0;
}

/** TH_STR_INIT
 * @brief Helper macro to initialize a th_str from string literal.
 */
#define TH_STR_INIT(str) {"" str, sizeof(str) - 1}

/** TH_STR
 * @brief Helper macro to create a th_str compound literal from a string literal.
 */
#define TH_STR(str) ((th_str){"" str, sizeof(str) - 1})

/** TH_STR_EQ
 * @brief Helper macro to compare a th_str with a string literal.
 */
#define TH_STR_EQ(str, cmp) (th_str_eq(str, TH_STR(cmp)))

TH_PRIVATE(bool)
th_str_is_uint(th_str str);

TH_PRIVATE(th_err)
th_str_to_uint(th_str str, unsigned int* out);

TH_PRIVATE(size_t)
th_str_find_first(th_str str, size_t start, char c);

TH_PRIVATE(size_t)
th_str_find_first_not(th_str str, size_t start, char c);

TH_PRIVATE(size_t)
th_str_find_first_of(th_str str, size_t start, const char* chars);

TH_PRIVATE(size_t)
th_str_find_last(th_str str, size_t start, char c);

/** th_str_substr
 * @brief Returns a substring of a string.
 * If len == th_str_npos, the substring will go to the end of the string.
 * If start > len, an empty string is returned (ptr = str.ptr + str.len, len = 0).
 */
TH_PRIVATE(th_str)
th_str_substr(th_str str, size_t start, size_t len);

/** th_str_trim
 * @brief Removes leading and trailing whitespace from a string.
 * This doesn't modify the original string, just returns a new view of it.
 * @param str The string to trim.
 * @return A new string view with leading and trailing whitespace removed.
 */
TH_PRIVATE(th_str)
th_str_trim(th_str str);

TH_PRIVATE(size_t)
th_str_hash(th_str str);

/* End of th_str.h */
/* Start of th_cookie_parser.h */



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

/* End of th_cookie_parser.h */
/* Start of th_fmt.h */


#include <stddef.h>
#include <stdint.h>
#include <time.h>


TH_PRIVATE(const char*)
th_fmt_uint_to_str(char* buf, size_t len, unsigned int val);

TH_PRIVATE(const char*)
th_fmt_uint_to_str_ex(char* buf, size_t len, unsigned int val, size_t* out_len);

/** th_fmt_str_append
 * @brief Append a string to a buffer.
 * @param buf The buffer to append to.
 * @param pos The current position in the buffer (where to append).
 * @param len The length of the buffer.
 * @param str The string to append.
 * @return The number of characters appended.
 */
TH_PRIVATE(size_t)
th_fmt_str_append(char* buf, size_t pos, size_t len, const char* str);

TH_PRIVATE(size_t)
th_fmt_strn_append(char* buf, size_t pos, size_t len, const char* str, size_t n);

TH_PRIVATE(size_t)
th_fmt_strtime(char* buf, size_t len, th_date date);
/* End of th_fmt.h */
/* Start of th_system_error.h */


#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <string.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <windows.h>
#endif

TH_INLINE(const char*)
th_system_strerror(int errc) TH_MAYBE_UNUSED;

TH_INLINE(const char*)
th_system_strerror(int errc)
{
#if defined(TH_CONFIG_OS_POSIX)
    return strerror(errc);
#elif defined(TH_CONFIG_OS_WIN)
    static char buf[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, sizeof(buf), NULL);
    return buf;
#endif
}

/* Define the system error codes that we use */
#if defined(TH_CONFIG_OS_POSIX)
#define TH_ENOENT ENOENT
#define TH_EINTR EINTR
#define TH_EIO EIO
#define TH_EBADF EBADF
#define TH_EBUSY EBUSY
#define TH_EAGAIN EAGAIN
#define TH_EWOULDBLOCK EWOULDBLOCK
#define TH_ENOMEM ENOMEM
#define TH_ENOSYS ENOSYS
#define TH_ETIMEDOUT ETIMEDOUT
#define TH_ECANCELED ECANCELED
#elif defined(TH_CONFIG_OS_WIN)
#define TH_ENOENT ERROR_FILE_NOT_FOUND
#define TH_EINTR ERROR_INTERRUPT
#define TH_EIO ERROR_IO_DEVICE
#define TH_EBADF ERROR_BAD_FORMAT
#define TH_EBUSY ERROR_BUSY
#define TH_EAGAIN ERROR_RETRY
#define TH_EWOULDBLOCK ERROR_RETRY
#define TH_ENOMEM ERROR_OUTOFMEMORY
#define TH_ENOSYS ERROR_NOT_SUPPORTED
#define TH_ETIMEDOUT ERROR_TIMEOUT
#define TH_ECANCELED ERROR_CANCELLED
#endif

/* End of th_system_error.h */
/* Start of th_http_error.h */



#include <errno.h>

/** th_http_err
 * @brief Converts a error code to a equivalent HTTP error code.
 */
TH_INLINE(th_err)
th_http_error(th_err err)
{
    if (err == TH_ERR_OK)
        return TH_ERR_HTTP(TH_CODE_OK);
    switch (TH_ERR_CATEGORY(err)) {
    case TH_ERR_CATEGORY_SYSTEM:
        switch (TH_ERR_CODE(err)) {
        case TH_ENOENT:
            return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
            break;
        case TH_ETIMEDOUT:
            return TH_ERR_HTTP(TH_CODE_REQUEST_TIMEOUT);
            break;
        default:
            return TH_ERR_HTTP(TH_CODE_INTERNAL_SERVER_ERROR);
            break;
        }
        break;
    case TH_ERR_CATEGORY_HTTP:
        return err;
        break;
    default:
        break;
    }
    return TH_ERR_HTTP(TH_CODE_INTERNAL_SERVER_ERROR);
}

TH_INLINE(const char*)
th_http_strerror(int code)
{
    switch (code) {
    case TH_CODE_OK:
        return "OK";
        break;
    case TH_CODE_MOVED_PERMANENTLY:
        return "Moved Permanently";
        break;
    case TH_CODE_BAD_REQUEST:
        return "Bad Request";
        break;
    case TH_CODE_NOT_FOUND:
        return "Not Found";
        break;
    case TH_CODE_METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
        break;
    case TH_CODE_PAYLOAD_TOO_LARGE:
        return "Payload Too Large";
        break;
    case TH_CODE_INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
        break;
    case TH_CODE_SERVICE_UNAVAILABLE:
        return "Service Unavailable";
        break;
    case TH_CODE_NOT_IMPLEMENTED:
        return "Method Not Implemented";
        break;
    case TH_CODE_REQUEST_TIMEOUT:
        return "Request Timeout";
        break;
    case TH_CODE_TOO_MANY_REQUESTS:
        return "Too Many Requests";
        break;
    case TH_CODE_URI_TOO_LONG:
        return "URI Too Long";
        break;
    case TH_CODE_UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported Media Type";
        break;
    case TH_CODE_RANGE_NOT_SATISFIABLE:
        return "Range Not Satisfiable";
        break;
    case TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE:
        return "Request Header Fields Too Large";
        break;
    case TH_CODE_UNAUTHORIZED:
        return "Unauthorized";
        break;
    case TH_CODE_FORBIDDEN:
        return "Forbidden";
        break;
    default:
        return "Unknown";
        break;
    }
}

typedef enum th_http_code_type {
    TH_HTTP_CODE_TYPE_INFORMATIONAL,
    TH_HTTP_CODE_TYPE_SUCCESS,
    TH_HTTP_CODE_TYPE_REDIRECT,
    TH_HTTP_CODE_TYPE_CLIENT_ERROR,
    TH_HTTP_CODE_TYPE_SERVER_ERROR,
} th_http_code_type;

TH_INLINE(th_http_code_type)
th_http_code_get_type(int code)
{
    if (code >= 100 && code < 200)
        return TH_HTTP_CODE_TYPE_INFORMATIONAL;
    if (code >= 200 && code < 300)
        return TH_HTTP_CODE_TYPE_SUCCESS;
    if (code >= 300 && code < 400)
        return TH_HTTP_CODE_TYPE_REDIRECT;
    if (code >= 400 && code < 500)
        return TH_HTTP_CODE_TYPE_CLIENT_ERROR;
    if (code >= 500 && code < 600)
        return TH_HTTP_CODE_TYPE_SERVER_ERROR;
    return TH_HTTP_CODE_TYPE_SERVER_ERROR;
}

/* End of th_http_error.h */
/* Start of th_address.h */



#include <sys/socket.h>

/** th_address
 * @brief Storage for a peer address filled in by th_acceptor_ops.accept.
 */
typedef struct th_address {
    struct sockaddr_storage addr;
    socklen_t addrlen;
} th_address;

TH_INLINE(void)
th_address_init(th_address* addr)
{
    addr->addrlen = sizeof(addr->addr);
}

/* End of th_address.h */
/* Start of th_dir.h */



/** th_dir_ops
 * @brief The raw open/close syscalls a th_dir performs. Injected at
 * construction time so tests can fake a directory fd without touching the
 * filesystem. open behaves like the underlying syscall: TH_ERR_OK with
 * *fd set on success, TH_ERR_SYSTEM(errno) on failure.
 */
typedef struct th_dir_ops {
    th_err (*open)(void* self, const char* path, int* fd);
    void (*close)(void* self, int fd);
} th_dir_ops;

TH_PRIVATE(th_dir_ops*)
th_dir_ops_os(void);

typedef struct th_dir {
    th_dir_ops* ops;
    int fd;
} th_dir;

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_dir_ops* ops);

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_str path);

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir);

/* End of th_dir.h */
/* Start of th_filepath.h */



/** th_filepath
 * @brief A validated, NUL-terminated relative path, ready to pass to a
 * syscall. th_filepath_init rejects absolute paths and "." / ".."
 * components - openat(dir->fd, ...) doesn't confine resolution to dir, a
 * ".." component walks back out of it like normal path resolution - so
 * any path built from untrusted input (e.g. a client-supplied filename)
 * must go through this first.
 */
typedef struct th_filepath {
    char buf[TH_CONFIG_MAX_PATH_LEN + 1];
} th_filepath;

/** th_filepath_init
 * @brief Fills path with str NUL-terminated.
 * @return TH_ERR_INVALID_ARG if str is absolute, too long, empty, or has
 * a "." / ".." component.
 */
TH_PRIVATE(th_err)
th_filepath_init(th_filepath* path, th_str str);

TH_INLINE(const char*)
th_filepath_cstr(const th_filepath* path)
{
    return path->buf;
}

/* End of th_filepath.h */
/* Start of th_file.h */



#include <sys/stat.h>

typedef struct th_open_opt {
    bool read;
    bool write;
    bool create;
    bool truncate;
} th_open_opt;

/** th_file_ops
 * @brief The raw syscalls a th_file performs. Injected at construction time
 * so tests can fake a file fd without touching the filesystem. Each method
 * behaves like the underlying syscall: TH_ERR_OK (with any out-params set)
 * on success, TH_ERR_SYSTEM(errno) on failure.
 */
typedef struct th_file_ops {
    th_err (*openat)(void* self, int dirfd, const char* path, int flags, int* fd);
    th_err (*seek)(void* self, int fd, int whence, size_t* pos);
    th_err (*read)(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read);
    th_err (*write)(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written);
    th_err (*stat)(void* self, int fd, struct stat* out);
    void (*close)(void* self, int fd);
} th_file_ops;

TH_PRIVATE(th_file_ops*)
th_file_ops_os(void);

typedef struct th_file {
    th_file_ops* ops;
    int fd;
    size_t size;
} th_file;

TH_PRIVATE(void)
th_file_init(th_file* stream, th_file_ops* ops);

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, const th_filepath* path, th_open_opt opt);

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_file_write(th_file* stream, const void* addr, size_t len, size_t offset, size_t* written) TH_MAYBE_UNUSED;

TH_PRIVATE(uint32_t)
th_file_stat_hash(th_file* stream);

TH_PRIVATE(void)
th_file_close(th_file* stream);

TH_PRIVATE(void)
th_file_deinit(th_file* stream);

/* End of th_file.h */
/* Start of th_iov.h */

#include <stddef.h>
#include <sys/uio.h>


/** th_iov
 *@brief I/O vector.
 */

typedef struct th_iov {
    void* base;
    size_t len;
} th_iov;

/** th_iov_consume
 *@brief Consume the I/O vector and
 * return the number of bytes that were not consumed.
 */
TH_INLINE(size_t)
th_iov_consume(th_iov** iov, size_t* iov_len, size_t consume)
{
    size_t zeroed = 0;
    for (size_t i = 0; i < *iov_len; i++) {
        if (consume < (*iov)[i].len) {
            (*iov)[i].base = (char*)(*iov)[i].base + consume;
            (*iov)[i].len -= consume;
            consume = 0;
            break;
        }
        consume -= (*iov)[i].len;
        (*iov)[i].len = 0;
        zeroed++;
    }
    *iov_len -= zeroed;
    (*iov) += zeroed;
    return consume;
}

TH_INLINE(size_t)
th_iov_bytes(th_iov* iov, size_t iov_len)
{
    size_t bytes = 0;
    for (size_t i = 0; i < iov_len; i++) {
        bytes += iov[i].len;
    }
    return bytes;
}

/* End of th_iov.h */
/* Start of th_log.h */


#include <stdarg.h>
#include <stdio.h>


#ifndef TH_LOG_LEVEL
#define TH_LOG_LEVEL TH_LOG_LEVEL_INFO
#endif

#define TH_LOG_TAG "default"

TH_PRIVATE(th_log*)
th_default_log_get(void);

TH_PRIVATE(void)
th_log_printf(int level, const char* fmt, ...) TH_MAYBE_UNUSED TH_PRINTF_FMT(2, 3);

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_TRACE
#define TH_LOG_TRACE(...) th_log_printf(TH_LOG_LEVEL_TRACE, "TRACE: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_TRACE(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_DEBUG
#define TH_LOG_DEBUG(...) th_log_printf(TH_LOG_LEVEL_DEBUG, "DEBUG: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_DEBUG(...) ((void)0)
#endif

#if (TH_LOG_LEVEL <= TH_LOG_LEVEL_INFO)
#define TH_LOG_INFO(...) th_log_printf(TH_LOG_LEVEL_INFO, "INFO: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_INFO(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_WARN
#define TH_LOG_WARN(...) th_log_printf(TH_LOG_LEVEL_WARN, "WARN: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_WARN(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_ERROR
#define TH_LOG_ERROR(...) th_log_printf(TH_LOG_LEVEL_ERROR, "ERROR: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_ERROR(...) ((void)0)
#endif

#if TH_LOG_LEVEL <= TH_LOG_LEVEL_FATAL
#define TH_LOG_FATAL(...) th_log_printf(TH_LOG_LEVEL_FATAL, "FATAL: [" TH_LOG_TAG "] " __VA_ARGS__)
#else
#define TH_LOG_FATAL(...) ((void)0)
#endif

/* End of th_log.h */
/* Start of th_utility.h */


#include <stdlib.h>

#define TH_MIN(a, b) ((a) < (b) ? (a) : (b))
#define TH_MAX(a, b) ((a) > (b) ? (a) : (b))
#define TH_ABS(a) ((a) < 0 ? -(a) : (a))

#define TH_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Move a pointer from src to dst and set src to NULL
TH_INLINE(void*)
th_move_ptr(void** src)
{
    void* dst = *src;
    *src = NULL;
    return dst;
}

#define TH_MOVE_PTR(ptr) th_move_ptr((void**)&(ptr))

// Custom assert macros

#ifndef NDEBUG
#define TH_ASSERT(cond)                                                               \
    do {                                                                              \
        if (!(cond)) {                                                                \
            TH_LOG_FATAL("Assertion failed: %s at %s:%d", #cond, __FILE__, __LINE__); \
            abort();                                                                  \
        }                                                                             \
    } while (0)
#else
#define TH_ASSERT(cond) ((void)0)
#endif

// Mathematical utility functions

TH_INLINE(size_t)
th_next_pow2(size_t n)
{
    TH_ASSERT(n > 0);
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

/* End of th_utility.h */
/* Start of th_list.h */


/** Generic doubly linked list implementation.
 * that works with any struct that has a next and prev pointer.
 */
#define TH_DEFINE_LIST(NAME, T, PREV, NEXT)                                         \
    typedef struct NAME {                                                           \
        T* head;                                                                    \
        T* tail;                                                                    \
    } NAME;                                                                         \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_push_back(NAME* list, T* item) TH_MAYBE_UNUSED;                          \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_pop_front(NAME* list) TH_MAYBE_UNUSED;                                   \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_front(NAME* list) TH_MAYBE_UNUSED;                                       \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_erase(NAME* list, T* item) TH_MAYBE_UNUSED;                              \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_next(T* item) TH_MAYBE_UNUSED;                                           \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_push_back(NAME* list, T* item)                                           \
    {                                                                               \
        TH_ASSERT(item != NULL);                                                    \
        if (list->head == NULL) {                                                   \
            list->head = item;                                                      \
            item->PREV = NULL;                                                      \
        } else {                                                                    \
            list->tail->NEXT = item;                                                \
            item->PREV = list->tail;                                                \
        }                                                                           \
        list->tail = item;                                                          \
        item->NEXT = NULL;                                                          \
    }                                                                               \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_pop_front(NAME* list)                                                    \
    {                                                                               \
        T* item = list->head;                                                       \
        if (item) {                                                                 \
            list->head = item->NEXT;                                                \
            if (list->head) {                                                       \
                list->head->PREV = NULL;                                            \
            } else {                                                                \
                list->tail = NULL;                                                  \
            }                                                                       \
            item->NEXT = NULL;                                                      \
        }                                                                           \
        return item;                                                                \
    }                                                                               \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_front(NAME* list)                                                        \
    {                                                                               \
        return list->head;                                                          \
    }                                                                               \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_erase(NAME* list, T* item)                                               \
    {                                                                               \
        TH_ASSERT(item != NULL);                                                    \
        TH_ASSERT((item->NEXT || item == list->tail) && "Item is not in the list"); \
        TH_ASSERT((item->PREV || item == list->head) && "Item is not in the list"); \
        T* next = item->NEXT;                                                       \
        T* prev = item->PREV;                                                       \
        if (prev) {                                                                 \
            prev->NEXT = next;                                                      \
            item->PREV = NULL;                                                      \
        } else {                                                                    \
            list->head = next;                                                      \
        }                                                                           \
        if (next) {                                                                 \
            next->PREV = prev;                                                      \
            item->NEXT = NULL;                                                      \
        } else {                                                                    \
            list->tail = prev;                                                      \
        }                                                                           \
    }                                                                               \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_next(T* item)                                                            \
    {                                                                               \
        return item->NEXT;                                                          \
    }

/* End of th_list.h */
/* Start of th_allocator.h */

#include <stddef.h>
#include <stdint.h>


TH_INLINE(void*)
th_allocator_alloc(th_allocator* allocator, size_t size)
{
    return allocator->alloc(allocator, size);
}

TH_INLINE(void*)
th_allocator_realloc(th_allocator* allocator, void* ptr, size_t size)
{
    return allocator->realloc(allocator, ptr, size);
}

TH_INLINE(void)
th_allocator_free(th_allocator* allocator, void* ptr)
{
    allocator->free(allocator, ptr);
}

TH_PRIVATE(th_allocator*)
th_default_allocator_get(void);

/* th_arena_allocator begin */

typedef struct th_arena_allocator {
    th_allocator base;
    th_allocator* allocator;
    void* buf;
    size_t size;
    size_t pos;
    size_t prev_pos;
    uint16_t alignment;
} th_arena_allocator;

/** th_arena_allocator_init
 * @brief The arena allocator is a simple allocator that allocates memory from a fixed-size buffer.
 * It only frees memory when the free operation is called on the previously allocated memory.
 * If no memory is available in the buffer, it will fall back to the default allocator.
 * @param allocator The arena allocator to initialize.
 * @param buf The buffer to use for allocations.
 * @param size The size of the buffer.
 */
TH_PRIVATE(void)
th_arena_allocator_init(th_arena_allocator* allocator, void* buf, size_t size, th_allocator* fallback);

/** th_arena_allocator_init_with_alignment
 * @brief Just like th_arena_allocator_init, but allows specifying the alignment of the allocations.
 */
TH_PRIVATE(void)
th_arena_allocator_init_with_alignment(th_arena_allocator* allocator, void* buf, size_t size, size_t alignment, th_allocator* fallback);

/* th_arena_allocator end */
/** Generic object pool allocator.
 * The pool allocator is a allocator that allocates objects from a pool of fixed-size blocks.
 * It can be used with any object that has a next and prev pointer.
 */
#define TH_DEFINE_POOL_ALLOCATOR(NAME, T, PREV, NEXT)                                            \
    TH_DEFINE_LIST(NAME##_list, T, PREV, NEXT)                                                    \
    typedef struct NAME {                                                                         \
        th_allocator base;                                                                        \
        NAME##_list free_list;                                                                    \
        NAME##_list used_list;                                                                    \
        th_allocator* allocator;                                                                  \
        size_t count;                                                                             \
        size_t max;                                                                               \
    } NAME;                                                                                       \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_init(NAME* pool, th_allocator* allocator, size_t initial, size_t max) TH_MAYBE_UNUSED; \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_deinit(NAME* pool) TH_MAYBE_UNUSED;                                                    \
                                                                                                  \
    TH_INLINE(void*)                                                                              \
    NAME##_alloc(void* self, size_t) TH_MAYBE_UNUSED;                                             \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_free(void* self, void* ptr) TH_MAYBE_UNUSED;                                           \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_init(NAME* pool, th_allocator* allocator, size_t initial, size_t max)                  \
    {                                                                                             \
        TH_ASSERT(allocator != NULL && "Invalid allocator");                                      \
        TH_ASSERT(max > 0 && "Invalid max");                                                      \
        pool->base.alloc = NAME##_alloc;                                                          \
        pool->base.realloc = NULL;                                                                \
        pool->base.free = NAME##_free;                                                            \
        pool->allocator = allocator;                                                              \
        pool->count = 0;                                                                          \
        pool->max = max;                                                                          \
        pool->used_list = (NAME##_list){0};                                                       \
        pool->free_list = (NAME##_list){0};                                                       \
        for (size_t i = 0; i < initial; i++) {                                                    \
            T* item = (T*)th_allocator_alloc(pool->allocator, sizeof(T));                         \
            if (item) {                                                                           \
                NAME##_list_push_back(&pool->free_list, item);                                    \
                ++pool->count;                                                                    \
            }                                                                                     \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_deinit(NAME* pool)                                                                     \
    {                                                                                             \
        T* item = NULL;                                                                           \
        while ((item = NAME##_list_pop_front(&pool->free_list))) {                                \
            th_allocator_free(pool->allocator, item);                                             \
        }                                                                                         \
        item = NAME##_list_pop_front(&pool->used_list);                                           \
        TH_ASSERT(item == NULL);                                                                  \
    }                                                                                             \
                                                                                                  \
    TH_INLINE(void*)                                                                              \
    NAME##_alloc(void* self, size_t size)                                                         \
    {                                                                                             \
        TH_ASSERT(size == sizeof(T) && "Invalid size");                                           \
        (void)size;                                                                               \
        NAME* pool = (NAME*)self;                                                                 \
        T* item = NAME##_list_pop_front(&pool->free_list);                                        \
        if (item == NULL) {                                                                       \
            if (pool->count < pool->max) {                                                        \
                item = (T*)th_allocator_alloc(pool->allocator, sizeof(T));                        \
                if (item) {                                                                       \
                    pool->count++;                                                                \
                }                                                                                 \
            }                                                                                     \
        }                                                                                         \
        if (item) {                                                                               \
            NAME##_list_push_back(&pool->used_list, item);                                        \
        }                                                                                         \
        return item;                                                                              \
    }                                                                                             \
                                                                                                  \
    TH_INLINE(void)                                                                               \
    NAME##_free(void* self, void* ptr)                                                            \
    {                                                                                             \
        NAME* pool = (NAME*)self;                                                                 \
        T* item = (T*)ptr;                                                                        \
        if (item) {                                                                               \
            NAME##_list_erase(&pool->used_list, item);                                            \
            NAME##_list_push_back(&pool->free_list, item);                                        \
        }                                                                                         \
    }

/* End of th_allocator.h */
/* Start of th_queue.h */


#include <stdbool.h>

/** Generic queue implementation.
 * that works with any struct that has a next pointer.
 */
#define TH_DEFINE_QUEUE(NAME, T)                                 \
    typedef struct NAME {                                        \
        T* head;                                                 \
        T* tail;                                                 \
    } NAME;                                                      \
                                                                 \
    TH_INLINE(NAME)                                              \
    NAME##_make(void) TH_MAYBE_UNUSED;                           \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push(NAME* queue, T* item) TH_MAYBE_UNUSED;           \
                                                                 \
    TH_INLINE(T*)                                                \
    NAME##_pop(NAME* queue) TH_MAYBE_UNUSED;                     \
                                                                 \
    TH_INLINE(bool)                                              \
    NAME##_empty(NAME* queue) TH_MAYBE_UNUSED;                   \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push_queue(NAME* queue, NAME* other) TH_MAYBE_UNUSED; \
                                                                 \
    TH_INLINE(NAME)                                              \
    NAME##_make(void)                                            \
    {                                                            \
        return (NAME){.head = NULL, .tail = NULL};               \
    }                                                            \
                                                                 \
    TH_INLINE(bool)                                              \
    NAME##_empty(NAME* queue)                                    \
    {                                                            \
        return queue->head == NULL;                              \
    }                                                            \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push(NAME* queue, T* item)                            \
    {                                                            \
        if (queue->head == NULL) {                               \
            queue->head = item;                                  \
        } else {                                                 \
            queue->tail->next = item;                            \
        }                                                        \
        queue->tail = item;                                      \
        item->next = NULL;                                       \
    }                                                            \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push_queue(NAME* queue, NAME* other)                  \
    {                                                            \
        if (queue->head == NULL) {                               \
            *queue = *other;                                     \
        } else if (other->head) {                                \
            queue->tail->next = other->head;                     \
            queue->tail = other->tail;                           \
        }                                                        \
        *other = NAME##_make();                                  \
    }                                                            \
                                                                 \
    TH_INLINE(T*)                                                \
    NAME##_pop(NAME* queue)                                      \
    {                                                            \
        T* item = queue->head;                                   \
        if (item) {                                              \
            queue->head = item->next;                            \
            item->next = NULL;                                   \
        }                                                        \
        return item;                                             \
    }

/* End of th_queue.h */
/* Start of th_task.h */


#include <stdbool.h>
#include <stdlib.h>


typedef struct th_task {
    /** fn
     * @brief The function to execute.
     */
    void (*fn)(void* self);

    /** This is used internally by the runner. */
    struct th_task* next;
} th_task;

/** th_task_init
 * @brief Initializes a task.
 */
TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void* self));

/** th_task complete
 * @brief Runs the task. Safe even if fn frees the object embedding
 * task: nothing reads task after fn returns.
 */
TH_PRIVATE(void)
th_task_complete(th_task* task);

/* th_task_queue declarations begin */

#ifndef TH_TASK_QUEUE
#define TH_TASK_QUEUE
TH_DEFINE_QUEUE(th_task_queue, th_task)
#endif

/* th_task_queue declarations end */

/* End of th_task.h */
/* Start of th_op.h */



#include <stdint.h>

/** th_op_type
 * @brief Which readiness an op is waiting for.
 */
typedef enum th_op_type {
    TH_OP_READ = 0,
    TH_OP_WRITE = 1,
    TH_OP_MAX = 2,
} th_op_type;

/** th_op_flags
 * @brief TH_OP_COMPLETED marks that op->base.fn should finalize (e.g.
 * invoke a user callback) rather than perform I/O again; it is set right
 * before the op is posted to a th_loop, so finalization always runs from
 * a queue drain rather than synchronously inside the call that completed
 * the I/O — this bounds stack depth when I/O completes immediately over
 * and over (e.g. a fast local socket).
 *
 * TH_OP_IMMEDIATE marks that this op has not yet had its first real
 * attempt (set once at th_op_init). th_handle_submit checks it before
 * calling th_op_perform: if set, it tries the op inline right now — this
 * lets an op that's immediately satisfiable (e.g. data already buffered)
 * complete without ever touching the reactor. An op's perform function
 * must clear TH_OP_IMMEDIATE unconditionally on its very first attempt,
 * before checking the result — so on EAGAIN/EWOULDBLOCK it is already
 * clear on all resubmissions from then on. Once clear, th_handle_submit
 * skips the inline attempt entirely and registers straight for real
 * readiness. Without this, a submit that hits EAGAIN would recurse into
 * th_handle_submit -> th_op_perform -> the op's fn -> submit again, once
 * per retry with no real event ever separating attempts (e.g. a
 * listening socket with no pending connection loops until the stack
 * overflows, since nothing ever changes between synchronous attempts).
 */
typedef uint32_t th_op_flags;
#define TH_OP_COMPLETED ((th_op_flags)1 << 0)
#define TH_OP_IMMEDIATE ((th_op_flags)1 << 1)

/** th_op
 * @brief A task submitted to a th_handle (see th_reactor.h). th_handle_submit
 * runs op->base.fn immediately; on TH_EAGAIN/TH_EWOULDBLOCK it registers the
 * op for readiness and fn runs again once ready. On timeout/cancellation/
 * error it calls abort instead (with a th_err describing why), and fn is
 * never invoked for that attempt.
 */
typedef struct th_op {
    th_task base;
    void (*abort)(void* self, th_err err);
    th_op_type type;
    th_op_flags flags;
} th_op;

TH_INLINE(void)
th_op_init(th_op* op, th_op_type type, void (*fn)(void* self), void (*abort)(void* self, th_err err))
{
    th_task_init(&op->base, fn);
    op->abort = abort;
    op->type = type;
    op->flags = TH_OP_IMMEDIATE;
}

/** th_op_perform
 * @brief Runs the op's fn: performs I/O if not yet TH_OP_COMPLETED, or
 * finalizes (e.g. invokes a user callback) if it is.
 */
TH_INLINE(void)
th_op_perform(th_op* op)
{
    th_task_complete(&op->base);
}

TH_INLINE(void)
th_op_abort(th_op* op, th_err err)
{
    op->abort(op, err);
}

TH_INLINE(void)
th_op_set_flags(th_op* op, th_op_flags flags)
{
    op->flags |= flags;
}

TH_INLINE(void)
th_op_clear_flags(th_op* op, th_op_flags flags)
{
    op->flags &= ~flags;
}

TH_INLINE(th_op_flags)
th_op_get_flags(const th_op* op)
{
    return op->flags;
}

/* End of th_op.h */
/* Start of th_reactor.h */



/** th_handle
 * @brief One fd registered with a th_reactor. Vtable so different reactor
 * backends (poll, kqueue, ...) can implement it without the caller caring.
 */
typedef struct th_handle_methods {
    void (*cancel)(void* self);
    th_err (*submit)(void* self, th_op* op);
    void (*enable_timeout)(void* self, bool enabled);
    int (*get_fd)(const void* self);
    void (*destroy)(void* self);
} th_handle_methods;

typedef struct th_handle {
    const th_handle_methods* methods;
} th_handle;

TH_INLINE(void)
th_handle_cancel(th_handle* handle)
{
    handle->methods->cancel(handle);
}

/** th_handle_submit
 * @brief If op is still TH_OP_IMMEDIATE (its very first attempt), runs
 * op->base.fn inline right now — an op that's immediately satisfiable
 * completes without ever touching the reactor. Otherwise (a resubmit
 * after TH_EAGAIN/TH_EWOULDBLOCK, where TH_OP_IMMEDIATE is already
 * clear) skips straight to waiting for op->type readiness on this
 * handle's fd and runs fn once ready. At most one op per op type may be
 * pending at a time.
 */
TH_INLINE(th_err)
th_handle_submit(th_handle* handle, th_op* op)
{
    return handle->methods->submit(handle, op);
}

TH_INLINE(int)
th_handle_get_fd(const th_handle* handle)
{
    return handle->methods->get_fd(handle);
}

TH_INLINE(void)
th_handle_enable_timeout(th_handle* handle, bool enabled)
{
    handle->methods->enable_timeout(handle, enabled);
}

TH_INLINE(void)
th_handle_destroy(th_handle* handle)
{
    handle->methods->destroy(handle);
}

/** th_reactor
 * @brief Event loop backend: turns fd readiness into op completions.
 */
typedef struct th_reactor_methods {
    void (*run)(void* self, int timeout_ms);
    th_err (*create_handle)(void* self, th_handle** out, int fd);
    void (*destroy)(void* self);
} th_reactor_methods;

typedef struct th_reactor {
    const th_reactor_methods* methods;
} th_reactor;

TH_INLINE(void)
th_reactor_run(th_reactor* reactor, int timeout_ms)
{
    reactor->methods->run(reactor, timeout_ms);
}

TH_INLINE(th_err)
th_reactor_create_handle(th_reactor* reactor, th_handle** out, int fd)
{
    return reactor->methods->create_handle(reactor, out, fd);
}

TH_INLINE(void)
th_reactor_destroy(th_reactor* reactor)
{
    if (reactor->methods->destroy)
        reactor->methods->destroy(reactor);
}

/* End of th_reactor.h */
/* Start of th_loop.h */



/** th_loop
 * @brief Task scheduler: runs queued tasks, and polls the reactor for more
 * work whenever the queue would otherwise go empty.
 */
typedef struct th_loop {
    th_reactor* reactor;
    th_task reactor_task;
    th_task_queue queue;
    size_t num_tasks;
} th_loop;

TH_PRIVATE(void)
th_loop_init(th_loop* loop, th_reactor* reactor);

/** th_loop_push_task
 * @brief Queue a task to run on a future th_loop_poll call.
 */
TH_PRIVATE(void)
th_loop_push_task(th_loop* loop, th_task* task);

/** th_loop_push_uncounted_task
 * @brief Like th_loop_push_task, but for tasks the reactor already counted
 * (e.g. a completion handed back from th_reactor_run) — avoids double count.
 */
TH_PRIVATE(void)
th_loop_push_uncounted_task(th_loop* loop, th_task* task);

/** th_loop_increase_task_count
 * @brief Tells the loop it has pending work it wouldn't otherwise see —
 * e.g. a reactor holding an op pending for readiness, not yet queued.
 * Pair with th_loop_decrease_task_count once that work resolves.
 */
TH_PRIVATE(void)
th_loop_increase_task_count(th_loop* loop);

TH_PRIVATE(void)
th_loop_decrease_task_count(th_loop* loop);

/** th_loop_poll
 * @brief Run exactly one pending task, or poll the reactor for readiness if
 * the queue is otherwise empty (blocking up to timeout_ms in that case).
 * @return TH_ERR_OK on success, TH_ERR_EOF if there are no tasks at all.
 */
TH_PRIVATE(th_err)
th_loop_poll(th_loop* loop, int timeout_ms);

/** th_loop_run
 * @brief Repeatedly polls with a zero timeout until th_loop_poll reports
 * no work left (TH_ERR_EOF). Never blocks.
 */
TH_PRIVATE(void)
th_loop_run(th_loop* loop);

TH_PRIVATE(void)
th_loop_deinit(th_loop* loop);

/* End of th_loop.h */
/* Start of th_socket.h */



#include <stddef.h>

/** th_socket_ops
 * @brief The raw send/recv syscalls a th_socket performs. Injected at
 * construction time so tests can fake a socket without a real fd. Each
 * call behaves like the underlying syscall: TH_ERR_SYSTEM(TH_EAGAIN) /
 * TH_ERR_SYSTEM(TH_EWOULDBLOCK) when it would block, otherwise TH_ERR_OK
 * with *result set to the number of bytes transferred.
 */
typedef struct th_socket_ops {
    th_err (*send)(void* self, int fd, const void* addr, size_t len, size_t* result);
    th_err (*sendvec)(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result);
    th_err (*recv)(void* self, int fd, void* addr, size_t len, size_t* result);

    /** sendfile
     * @brief Sends header (iov/iovcnt, may be empty) followed by up to
     * len bytes of file starting at offset. *result is the total bytes
     * transferred across header and file combined.
     */
    th_err (*sendfile)(void* self, int fd, const th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, size_t* result);
} th_socket_ops;

TH_PRIVATE(th_socket_ops*)
th_socket_ops_os(void);

/** th_socket
 * @brief A non-blocking TCP connection: an fd registered with a reactor
 * plus the ops used to read/write it. Holds the th_loop (not just its
 * reactor) so ops can defer completion via th_socket_post instead of
 * invoking it inline.
 */
typedef struct th_socket {
    th_loop* loop;
    th_handle* handle;
    th_socket_ops* ops;
} th_socket;

TH_PRIVATE(void)
th_socket_init(th_socket* socket, th_loop* loop, th_socket_ops* ops);

/** th_socket_set_fd
 * @brief Registers fd with the socket's reactor, replacing any fd
 * previously set.
 */
TH_PRIVATE(th_err)
th_socket_set_fd(th_socket* socket, int fd);

TH_INLINE(int)
th_socket_get_fd(const th_socket* socket)
{
    return socket->handle ? th_handle_get_fd(socket->handle) : -1;
}

TH_INLINE(void)
th_socket_cancel(th_socket* socket)
{
    if (socket->handle)
        th_handle_cancel(socket->handle);
}

TH_INLINE(void)
th_socket_enable_timeout(th_socket* socket, bool enabled)
{
    th_handle_enable_timeout(socket->handle, enabled);
}

/** th_socket_submit
 * @brief Waits for op->type readiness on the socket's fd, then runs op.
 */
TH_INLINE(th_err)
th_socket_submit(th_socket* socket, th_op* op)
{
    return th_handle_submit(socket->handle, op);
}

/** th_socket_post
 * @brief Queues task (typically an op with TH_OP_COMPLETED just set) to
 * finalize on a future th_loop_poll/th_loop_run, rather than inline —
 * bounds stack depth when I/O completes immediately, repeatedly.
 */
TH_INLINE(void)
th_socket_post(th_socket* socket, th_task* task)
{
    th_loop_push_task(socket->loop, task);
}

TH_INLINE(th_err)
th_socket_send(th_socket* socket, const void* addr, size_t len, size_t* result)
{
    return socket->ops->send(socket->ops, th_socket_get_fd(socket), addr, len, result);
}

TH_INLINE(th_err)
th_socket_sendvec(th_socket* socket, const th_iov* iov, size_t iovcnt, size_t* result)
{
    return socket->ops->sendvec(socket->ops, th_socket_get_fd(socket), iov, iovcnt, result);
}

TH_INLINE(th_err)
th_socket_recv(th_socket* socket, void* addr, size_t len, size_t* result)
{
    return socket->ops->recv(socket->ops, th_socket_get_fd(socket), addr, len, result);
}

TH_INLINE(th_err)
th_socket_sendfile(th_socket* socket, const th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, size_t* result)
{
    return socket->ops->sendfile(socket->ops, th_socket_get_fd(socket), iov, iovcnt, file, offset, len, result);
}

/** th_socket_close
 * @brief Closes the underlying fd; the socket object itself stays valid
 * and can be reused via th_socket_set_fd.
 */
TH_PRIVATE(void)
th_socket_close(th_socket* socket);

TH_PRIVATE(void)
th_socket_deinit(th_socket* socket);

/* End of th_socket.h */
/* Start of th_acceptor.h */



/** th_acceptor_ops
 * @brief The raw listen-socket syscalls a th_acceptor performs. Injected
 * at construction time so tests can fake an acceptor without a real fd.
 */
typedef struct th_acceptor_ops {
    /** open
     * @brief Resolves addr/port, creates a non-blocking listening socket
     * bound and listening on it, and writes its fd to *fd.
     */
    th_err (*open)(void* self, const char* addr, const char* port, int* fd);

    /** accept
     * @brief Accepts one pending connection on fd, writes the peer
     * address to addr and the new non-blocking socket's fd to *out_fd.
     * TH_ERR_SYSTEM(TH_EAGAIN)/TH_EWOULDBLOCK when none is pending.
     */
    th_err (*accept)(void* self, int fd, th_address* addr, int* out_fd);
} th_acceptor_ops;

TH_PRIVATE(th_acceptor_ops*)
th_acceptor_ops_os(void);

/** th_acceptor
 * @brief A non-blocking listening socket: an fd registered with a reactor
 * plus the ops used to open it and accept connections from it. Holds the
 * th_loop (not just its reactor) so th_accept_op can defer completion via
 * th_acceptor_post instead of invoking it inline.
 */
typedef struct th_acceptor {
    th_loop* loop;
    th_handle* handle;
    th_acceptor_ops* ops;
} th_acceptor;

TH_PRIVATE(void)
th_acceptor_init(th_acceptor* acceptor, th_loop* loop, th_acceptor_ops* ops);

/** th_acceptor_open
 * @brief Resolves addr/port and registers the resulting listening socket
 * with the acceptor's reactor, replacing any fd previously set.
 */
TH_PRIVATE(th_err)
th_acceptor_open(th_acceptor* acceptor, const char* addr, const char* port);

TH_INLINE(int)
th_acceptor_get_fd(const th_acceptor* acceptor)
{
    return acceptor->handle ? th_handle_get_fd(acceptor->handle) : -1;
}

TH_INLINE(void)
th_acceptor_cancel(th_acceptor* acceptor)
{
    if (acceptor->handle)
        th_handle_cancel(acceptor->handle);
}

/** th_acceptor_submit
 * @brief Waits for op->type readiness on the acceptor's fd, then runs op.
 */
TH_INLINE(th_err)
th_acceptor_submit(th_acceptor* acceptor, th_op* op)
{
    return th_handle_submit(acceptor->handle, op);
}

/** th_acceptor_post
 * @brief Queues task (typically an op with TH_OP_COMPLETED just set) to
 * finalize on a future th_loop_poll/th_loop_run, rather than inline.
 */
TH_INLINE(void)
th_acceptor_post(th_acceptor* acceptor, th_task* task)
{
    th_loop_push_task(acceptor->loop, task);
}

/** th_acceptor_accept
 * @brief Accepts one pending connection and registers it with out_socket
 * (via th_socket_set_fd), replacing any fd previously set on it.
 */
TH_PRIVATE(th_err)
th_acceptor_accept(th_acceptor* acceptor, th_address* addr, th_socket* out_socket);

/** th_acceptor_close
 * @brief Closes the underlying fd; the acceptor object itself stays valid
 * and can be reused via th_acceptor_open.
 */
TH_PRIVATE(void)
th_acceptor_close(th_acceptor* acceptor);

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor);

/* End of th_acceptor.h */
/* Start of th_accept.h */



typedef void (*th_accept_cb)(void* user_data, th_err err);

/** th_accept_op
 * @brief Accepts one connection on a th_acceptor directly into socket
 * (via th_acceptor_accept/th_socket_set_fd). After init, start with
 * th_op_perform(&op->base): it performs the first, immediate accept
 * attempt and submits to the acceptor for readiness only on
 * TH_EAGAIN/TH_EWOULDBLOCK. On completion the op posts itself to the
 * acceptor's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_accept_op {
    th_op base;
    th_acceptor* acceptor;
    th_address* addr;
    th_socket* socket;
    th_accept_cb callback;
    void* user_data;
    th_err err;
} th_accept_op;

TH_PRIVATE(void)
th_accept_op_init(th_accept_op* op, th_acceptor* acceptor, th_address* addr,
                  th_socket* socket, th_accept_cb callback, void* user_data);

/* End of th_accept.h */
/* Start of th_recv.h */



#include <stdbool.h>

typedef void (*th_recv_cb)(void* user_data, size_t size, th_err err);

/** th_recv_op
 * @brief Reads from a th_socket into addr. If exact is false, completes
 * as soon as any bytes arrive (0 bytes => TH_ERR_EOF); if true, retries
 * until exactly len bytes have been read or an error/EOF occurs. After
 * init, start with th_op_perform(&op->base): it performs the first,
 * immediate recv attempt and submits to the socket for readiness only
 * on TH_EAGAIN/TH_EWOULDBLOCK. On completion the op posts itself to the
 * socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_recv_op {
    th_op base;
    th_socket* socket;
    th_recv_cb callback;
    void* user_data;
    void* addr;
    size_t len;
    size_t pos;
    bool exact;
    th_err err;
} th_recv_op;

TH_PRIVATE(void)
th_recv_op_init(th_recv_op* op, th_socket* socket, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data);

/* End of th_recv.h */
/* Start of th_send.h */



typedef void (*th_send_cb)(void* user_data, size_t size, th_err err);

/** th_send_op
 * @brief Writes addr to a th_socket, retrying until exactly len bytes
 * have been written or an error occurs. After init, start with
 * th_op_perform(&op->base). On completion the op posts itself to the
 * socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_send_op {
    th_op base;
    th_socket* socket;
    th_send_cb callback;
    void* user_data;
    const void* addr;
    size_t len;
    size_t pos;
    th_err err;
} th_send_op;

TH_PRIVATE(void)
th_send_op_init(th_send_op* op, th_socket* socket, const void* addr, size_t len, th_send_cb callback, void* user_data);

/* End of th_send.h */
/* Start of th_conn.h */



/* th_conn interface begin */

/** th_conn_methods
 * @brief A connection: an accepted socket plus the send/recv operations
 * needed to shuttle an HTTP request/response over it. th_response/th_http
 * call these directly instead of reaching through to a socket type, so
 * that e.g. th_ssl_conn can do handshake/BIO shuttling internally without
 * callers needing to know the connection is encrypted.
 */
typedef struct th_conn_methods {
    th_address* (*get_address)(void* self);
    th_socket* (*get_socket)(void* self);
    void (*start)(void* self);

    /** recv
     * @brief Reads into addr. If exact is false, completes as soon as
     * any bytes arrive (0 bytes => TH_ERR_EOF); if true, retries until
     * exactly len bytes have been read or an error/EOF occurs.
     */
    void (*recv)(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data);

    /** send
     * @brief Writes iov (mutated in place as buffers are consumed),
     * retrying until every byte has been written or an error occurs.
     * If file is NULL, only iov is sent. If file is non-NULL, iov is
     * sent as a header followed by len bytes of file starting at
     * offset (offset/len are ignored when file is NULL).
     */
    void (*send)(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data);

    void (*cancel)(void* self);
    void (*destroy)(void* self);
} th_conn_methods;

typedef struct th_conn {
    const th_conn_methods* methods;
} th_conn;

TH_INLINE(th_address*)
th_conn_get_address(th_conn* conn)
{
    return conn->methods->get_address(conn);
}

TH_INLINE(th_socket*)
th_conn_get_socket(th_conn* conn)
{
    return conn->methods->get_socket(conn);
}

TH_INLINE(void)
th_conn_start(th_conn* conn)
{
    conn->methods->start(conn);
}

TH_INLINE(void)
th_conn_recv(th_conn* conn, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    conn->methods->recv(conn, addr, len, exact, callback, user_data);
}

TH_INLINE(void)
th_conn_send(th_conn* conn, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    conn->methods->send(conn, iov, iovcnt, file, offset, len, callback, user_data);
}

TH_INLINE(void)
th_conn_cancel(th_conn* conn)
{
    conn->methods->cancel(conn);
}

TH_INLINE(void)
th_conn_destroy(th_conn* conn)
{
    conn->methods->destroy(conn);
}

/* th_conn interface end */
/* th_conn_upgrader interface begin */

/** th_conn_upgrader
 * @brief Implement this interface and pass it to `th_conn` to define
 * how a connection should be upgraded to a higher level protocol.
 */
typedef struct th_conn_upgrader {
    void (*upgrade)(void* self, th_conn* conn);
} th_conn_upgrader;

TH_INLINE(void)
th_conn_upgrader_init(th_conn_upgrader* upgrader, void (*upgrade)(void* self, th_conn* conn))
{
    upgrader->upgrade = upgrade;
}

TH_INLINE(void)
th_conn_upgrader_upgrade(th_conn_upgrader* upgrader, th_conn* conn)
{
    upgrader->upgrade(upgrader, conn);
}

/* th_conn_upgrader interface end */
/* th_conn_observable interface begin */

/** th_conn_observer
 * @brief Implement this interface to observe when a client is
 * initialized and destroyed.
 */
typedef struct th_conn_observable th_conn_observable;

typedef struct th_conn_observer th_conn_observer;
struct th_conn_observer {
    void (*on_init)(th_conn_observer* self, th_conn_observable* observable);
    void (*on_deinit)(th_conn_observer* self, th_conn_observable* observable);
};

TH_INLINE(void)
th_conn_observer_on_init(th_conn_observer* observer, th_conn_observable* observable)
{
    observer->on_init(observer, observable);
}

TH_INLINE(void)
th_conn_observer_on_deinit(th_conn_observer* observer, th_conn_observable* observable)
{
    observer->on_deinit(observer, observable);
}

struct th_conn_observable {
    th_conn base;
    void (*destroy)(void* self);
    th_conn_observer* observer;
    th_conn_observable *next, *prev;
};

TH_PRIVATE(void)
th_conn_observable_init(th_conn_observable* observable, const th_conn_methods* methods,
                        void (*destroy)(void* self), th_conn_observer* observer);

/** th_conn_observable_destroy
 * @brief The destroy every concrete conn type's th_conn_methods table
 * must point at: notifies the observer, then calls the type's real
 * destructor (the destroy passed to th_conn_observable_init).
 */
TH_PRIVATE(void)
th_conn_observable_destroy(void* self);

/* th_conn_observable interface end */

/* End of th_conn.h */
/* Start of th_conn_tracker.h */

/** th_conn_tracker
 * @brief The client tracker keep track of all clients that are currently active.
 * It is used to cancel all clients when the server is shutting down.
 */


TH_DEFINE_LIST(th_conn_observable_list, th_conn_observable, prev, next)

typedef struct th_conn_tracker {
    th_conn_observer base;
    th_conn_observable_list observables;
    th_task* task;
    size_t count;
} th_conn_tracker;

TH_PRIVATE(void)
th_conn_tracker_init(th_conn_tracker* conn_tracker);

TH_PRIVATE(void)
th_conn_tracker_cancel_all(th_conn_tracker* conn_tracker);

TH_PRIVATE(void)
th_conn_tracker_async_wait(th_conn_tracker* conn_tracker, th_task* task);

TH_PRIVATE(size_t)
th_conn_tracker_count(const th_conn_tracker* conn_tracker);

TH_PRIVATE(void)
th_conn_tracker_deinit(th_conn_tracker* conn_tracker);

/* End of th_conn_tracker.h */
/* Start of th_method.h */


struct th_method_mapping {
    const char* name;
    th_method method;
};

struct th_method_mapping* th_method_mapping_find(const char* str, size_t len);

/* End of th_method.h */
/* Start of th_hash.h */

#include <stddef.h>
#include <stdint.h>
#include <string.h>


/** th_hash_bytes
 * @brief Fowler-Noll-Vo hash function (FNV-1a).
 * See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */
TH_INLINE(size_t)
th_hash_bytes(const void* data, size_t len)
{
    size_t hash = 2166136261u;
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= 16777619;
    }
    return hash;
}

TH_INLINE(size_t)
th_hash_cstr(const char* str)
{
    return th_hash_bytes(str, strlen(str));
}

/* End of th_hash.h */
/* Start of th_hashmap.h */



#include <string.h>

#define TH_DEFINE_HASHMAP(NAME, K, V, HASH, K_EQ, K_NULL)                                                                           \
    typedef struct NAME##_entry {                                                                                                   \
        K key;                                                                                                                      \
        V value;                                                                                                                    \
    } NAME##_entry;                                                                                                                 \
                                                                                                                                    \
    typedef struct NAME {                                                                                                           \
        NAME##_entry* entries;                                                                                                      \
        size_t size;                                                                                                                \
        size_t capacity;                                                                                                            \
        size_t end;                                                                                                                 \
        size_t begin;                                                                                                               \
        th_allocator* allocator;                                                                                                    \
    } NAME;                                                                                                                         \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_init(NAME* map, th_allocator* allocator) TH_MAYBE_UNUSED;                                                                \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_reset(NAME* map) TH_MAYBE_UNUSED;                                                                                        \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_reserve(NAME* map, size_t capacity) TH_MAYBE_UNUSED;                                                                     \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_deinit(NAME* map) TH_MAYBE_UNUSED;                                                                                       \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_set(NAME* map, K key, V value) TH_MAYBE_UNUSED;                                                                          \
                                                                                                                                    \
    TH_INLINE(V*)                                                                                                                   \
    NAME##_try_get(const NAME* map, K key) TH_MAYBE_UNUSED;                                                                         \
                                                                                                                                    \
    typedef NAME##_entry* NAME##_iter;                                                                                              \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_find(const NAME* map, K key) TH_MAYBE_UNUSED;                                                                            \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_erase(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                   \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_begin(NAME* map) TH_MAYBE_UNUSED;                                                                                        \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_next(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                    \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_prev(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                    \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_init(NAME* map, th_allocator* allocator)                                                                                 \
    {                                                                                                                               \
        map->allocator = allocator;                                                                                                 \
        if (map->allocator == NULL) {                                                                                               \
            map->allocator = th_default_allocator_get();                                                                            \
        }                                                                                                                           \
        map->entries = NULL;                                                                                                        \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_deinit(NAME* map)                                                                                                        \
    {                                                                                                                               \
        if (map->entries) {                                                                                                         \
            th_allocator_free(map->allocator, map->entries);                                                                        \
            map->entries = NULL;                                                                                                    \
        }                                                                                                                           \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_reset(NAME* map)                                                                                                         \
    {                                                                                                                               \
        if (map->entries) {                                                                                                         \
            if (map->size > 0) {                                                                                                    \
                for (size_t i = map->begin; i < map->end; i++) {                                                                    \
                    NAME##_entry* entry = &map->entries[i];                                                                         \
                    entry->key = K_NULL;                                                                                            \
                }                                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_reserve(NAME* map, size_t capacity)                                                                                      \
    {                                                                                                                               \
        if (map->capacity >= capacity) {                                                                                            \
            return TH_ERR_OK;                                                                                                       \
        }                                                                                                                           \
        capacity = th_next_pow2(capacity);                                                                                          \
        NAME##_entry* entries = (NAME##_entry*)th_allocator_realloc(map->allocator, map->entries, capacity * sizeof(NAME##_entry)); \
        if (entries == NULL) {                                                                                                      \
            return TH_ERR_BAD_ALLOC;                                                                                                \
        }                                                                                                                           \
        for (size_t i = map->capacity; i < capacity; i++) {                                                                         \
            entries[i] = (NAME##_entry){.key = K_NULL};                                                                             \
        }                                                                                                                           \
        map->entries = entries;                                                                                                     \
        map->capacity = capacity;                                                                                                   \
        return TH_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_LOCAL(void)                                                                                                                  \
    NAME##_update_begin_end(NAME* map, size_t new_index)                                                                            \
    {                                                                                                                               \
        if (map->size == 1) {                                                                                                       \
            map->begin = new_index;                                                                                                 \
            map->end = new_index + 1;                                                                                               \
        } else {                                                                                                                    \
            if (new_index < map->begin) {                                                                                           \
                map->begin = new_index;                                                                                             \
            }                                                                                                                       \
            if (new_index + 1 > map->end) {                                                                                         \
                map->end = new_index + 1;                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_LOCAL(th_err)                                                                                                                \
    NAME##_do_set(NAME* map, size_t hash, K key, V value)                                                                         \
    {                                                                                                                               \
        for (size_t i = hash; i < map->capacity; i++) {                                                                             \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                map->size++;                                                                                                        \
                NAME##_update_begin_end(map, i);                                                                                    \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                entry->value = value;                                                                                               \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
        for (size_t i = 0; i < hash; i++) {                                                                                         \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                map->size++;                                                                                                        \
                NAME##_update_begin_end(map, i);                                                                                    \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                entry->value = value;                                                                                               \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
        return TH_ERR_BAD_ALLOC;                                                                                                    \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_fix_hole(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_fix_hole(NAME* map, NAME##_entry* entry)                                                                                 \
    {                                                                                                                               \
        TH_ASSERT(entry >= map->entries && entry < map->entries + map->capacity && "Entry is out of bounds");                       \
        size_t last_zeroed = (size_t)(entry - map->entries);                                                                        \
        for (size_t i = (size_t)(entry - map->entries + 1); i < map->end; i++) {                                                    \
            size_t hash = 0;                                                                                                      \
            if (K_EQ(map->entries[i].key, K_NULL)) {                                                                                \
                break;                                                                                                              \
            } else if ((hash = (HASH(map->entries[i].key) & (map->capacity - 1))) <= last_zeroed) {                                 \
                map->entries[last_zeroed] = map->entries[i];                                                                        \
                map->entries[i].key = K_NULL;                                                                                       \
                last_zeroed = i;                                                                                                    \
            }                                                                                                                       \
        }                                                                                                                           \
        if (map->size == 0) {                                                                                                       \
            map->begin = 0;                                                                                                         \
            map->end = 0;                                                                                                           \
        } else if (last_zeroed == map->end - 1) {                                                                                   \
            map->end = (size_t)(NAME##_prev(map, &map->entries[last_zeroed]) - map->entries + 1);                                   \
        } else if (last_zeroed == map->begin) {                                                                                     \
            map->begin = (size_t)(NAME##_next(map, &map->entries[last_zeroed]) - map->entries);                                     \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_expand(NAME* map)                                                                                                        \
    {                                                                                                                               \
        th_err err = TH_ERR_OK;                                                                                                     \
        size_t old_capacity = map->capacity;                                                                                        \
        size_t new_capacity = old_capacity * 2;                                                                                     \
        if (new_capacity == 0) {                                                                                                    \
            new_capacity = 1;                                                                                                       \
        }                                                                                                                           \
        if ((err = NAME##_reserve(map, new_capacity)) != TH_ERR_OK) {                                                               \
            return err;                                                                                                             \
        }                                                                                                                           \
        /* Need to rehash all entries */                                                                                            \
        for (size_t i = 0; i < old_capacity; i++) {                                                                                 \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                /* rearranged == 0; */                                                                                              \
                continue;                                                                                                           \
            }                                                                                                                       \
            size_t hash = HASH(entry->key);                                                                                       \
            /* Don't need to rehash every entry */                                                                                  \
            hash &= (new_capacity - 1);                                                                                             \
            NAME##_entry e = *entry;                                                                                                \
            entry->key = K_NULL;                                                                                                    \
            --map->size;                                                                                                            \
            NAME##_fix_hole(map, entry);                                                                                            \
            if ((err = NAME##_do_set(map, hash, e.key, e.value)) != TH_ERR_OK) {                                                    \
                return err;                                                                                                         \
            }                                                                                                                       \
        }                                                                                                                           \
        return TH_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_set(NAME* map, K key, V value)                                                                                           \
    {                                                                                                                               \
        if (map->size >= map->capacity / 2) {                                                                                       \
            th_err err = NAME##_expand(map);                                                                                        \
            if (err != TH_ERR_OK) {                                                                                                 \
                return err;                                                                                                         \
            }                                                                                                                       \
        }                                                                                                                           \
        size_t hash = HASH(key) & (map->capacity - 1);                                                                            \
        return NAME##_do_set(map, hash, key, value);                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_find(const NAME* map, K key)                                                                                             \
    {                                                                                                                               \
        size_t hash = HASH(key) & (map->capacity - 1);                                                                            \
        if (map->size == 0) {                                                                                                       \
            return NULL;                                                                                                            \
        }                                                                                                                           \
        for (size_t i = hash; i < map->end; i++) {                                                                                  \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                return NULL;                                                                                                        \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                return entry;                                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        for (size_t i = map->begin; i < hash; i++) {                                                                                \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                return NULL;                                                                                                        \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                return entry;                                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_erase(NAME* map, NAME##_entry* entry)                                                                                    \
    {                                                                                                                               \
        entry->key = K_NULL;                                                                                                        \
        map->size--;                                                                                                                \
        NAME##_fix_hole(map, entry);                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(V*)                                                                                                                   \
    NAME##_try_get(const NAME* map, K key)                                                                                          \
    {                                                                                                                               \
        NAME##_entry* entry = NAME##_find(map, key);                                                                                \
        if (entry) {                                                                                                                \
            return &entry->value;                                                                                                   \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_begin(NAME* map)                                                                                                         \
    {                                                                                                                               \
        if (map->begin == map->end)                                                                                                 \
            return NULL;                                                                                                            \
        return &map->entries[map->begin];                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_next(NAME* map, NAME##_entry* entry)                                                                                     \
    {                                                                                                                               \
        TH_ASSERT(entry >= map->entries && entry < map->entries + map->capacity && "Entry is out of bounds");                       \
        size_t i = (size_t)(entry - map->entries);                                                                                  \
        for (size_t j = i + 1; j < map->end; j++) {                                                                                 \
            NAME##_entry* e = &map->entries[j];                                                                                     \
            if (!K_EQ(e->key, K_NULL)) {                                                                                            \
                return e;                                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_prev(NAME* map, NAME##_entry* entry)                                                                                     \
    {                                                                                                                               \
        TH_ASSERT(entry >= map->entries && entry < map->entries + map->capacity && "Entry is out of bounds");                       \
        size_t i = (size_t)(entry - map->entries);                                                                                  \
        for (size_t j = i - 1; j >= map->begin; j--) {                                                                              \
            NAME##_entry* e = &map->entries[j];                                                                                     \
            if (!K_EQ(e->key, K_NULL)) {                                                                                            \
                return e;                                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
        return NAME##_begin(map);                                                                                                   \
    }

/* th_cstr_map begin */

TH_INLINE(size_t)
th_cstr_hash(const char* str)
{
    return th_hash_cstr(str);
}

TH_INLINE(bool)
th_cstr_eq(const char* a, const char* b)
{
    if (!a || !b)
        return a == b;
    return *a == *b && (strcmp(a, b) == 0);
}

TH_DEFINE_HASHMAP(th_cstr_map, const char*, const char*, th_cstr_hash, th_cstr_eq, NULL)

/* th_cstr_map end */

/* End of th_hashmap.h */
/* Start of th_vec.h */



#include <stddef.h>

#define TH_DEFINE_VEC(NAME, TYPE, DEINIT)                                                                  \
    typedef struct NAME {                                                                                  \
        TYPE* data;                                                                                        \
        size_t size;                                                                                       \
        size_t capacity;                                                                                   \
        th_allocator* allocator;                                                                           \
    } NAME;                                                                                                \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_init(NAME* vec, th_allocator* allocator) TH_MAYBE_UNUSED;                                       \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_clear(NAME* vec) TH_MAYBE_UNUSED;                                                               \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_deinit(NAME* vec) TH_MAYBE_UNUSED;                                                              \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_size(const NAME* vec) TH_MAYBE_UNUSED;                                                          \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_capacity(const NAME* vec) TH_MAYBE_UNUSED;                                                      \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_resize(NAME* vec, size_t size) TH_MAYBE_UNUSED;                                                 \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_push_back(NAME* vec, TYPE value) TH_MAYBE_UNUSED;                                               \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_at(NAME* vec, size_t index) TH_MAYBE_UNUSED;                                                    \
                                                                                                           \
    TH_INLINE(const TYPE*)                                                                                 \
    NAME##_cat(const NAME* vec, size_t index) TH_MAYBE_UNUSED;                                             \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_begin(NAME* vec) TH_MAYBE_UNUSED;                                                               \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_end(NAME* vec) TH_MAYBE_UNUSED;                                                                 \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_init(NAME* vec, th_allocator* allocator)                                                        \
    {                                                                                                      \
        vec->allocator = allocator ? allocator : th_default_allocator_get();                               \
        vec->capacity = 0;                                                                                 \
        vec->size = 0;                                                                                     \
        vec->data = NULL;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_deinit(NAME* vec)                                                                               \
    {                                                                                                      \
        if (vec->data) {                                                                                   \
            for (size_t i = 0; i < vec->size; i++) {                                                       \
                DEINIT(&vec->data[i]);                                                                     \
            }                                                                                              \
            th_allocator_free(vec->allocator, vec->data);                                                  \
        }                                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(void)                                                                                        \
    NAME##_clear(NAME* vec)                                                                                \
    {                                                                                                      \
        if (vec->data) {                                                                                   \
            for (size_t i = 0; i < vec->size; i++) {                                                       \
                DEINIT(&vec->data[i]);                                                                     \
            }                                                                                              \
        }                                                                                                  \
        vec->size = 0;                                                                                     \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_size(const NAME* vec)                                                                           \
    {                                                                                                      \
        return vec->size;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(size_t)                                                                                      \
    NAME##_capacity(const NAME* vec)                                                                       \
    {                                                                                                      \
        return vec->capacity;                                                                              \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_resize(NAME* vec, size_t size)                                                                  \
    {                                                                                                      \
        if (size < vec->size) {                                                                            \
            vec->size = size;                                                                              \
            return TH_ERR_OK;                                                                              \
        }                                                                                                  \
        if (size > vec->capacity) {                                                                        \
            size_t new_capacity = th_next_pow2(size);                                                      \
            TYPE* new_data = th_allocator_realloc(vec->allocator, vec->data, new_capacity * sizeof(TYPE)); \
            if (new_data == NULL) {                                                                        \
                return TH_ERR_BAD_ALLOC;                                                                   \
            }                                                                                              \
            vec->data = new_data;                                                                          \
            vec->capacity = new_capacity;                                                                  \
        }                                                                                                  \
        vec->size = size;                                                                                  \
        return TH_ERR_OK;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(th_err)                                                                                      \
    NAME##_push_back(NAME* vec, TYPE value)                                                                \
    {                                                                                                      \
        if (vec->size >= vec->capacity) {                                                                  \
            size_t new_capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;                              \
            TYPE* new_data = th_allocator_realloc(vec->allocator, vec->data, new_capacity * sizeof(TYPE)); \
            if (new_data == NULL) {                                                                        \
                return TH_ERR_BAD_ALLOC;                                                                   \
            }                                                                                              \
            vec->data = new_data;                                                                          \
            vec->capacity = new_capacity;                                                                  \
        }                                                                                                  \
        vec->data[vec->size++] = value;                                                                    \
        return TH_ERR_OK;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_at(NAME* vec, size_t index)                                                                     \
    {                                                                                                      \
        TH_ASSERT(index <= vec->size);                                                                     \
        return vec->data + index;                                                                          \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(const TYPE*)                                                                                 \
    NAME##_cat(const NAME* vec, size_t index)                                                              \
    {                                                                                                      \
        TH_ASSERT(index <= vec->size);                                                                     \
        return vec->data + index;                                                                          \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_begin(NAME* vec)                                                                                \
    {                                                                                                      \
        return vec->data;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    TH_INLINE(TYPE*)                                                                                       \
    NAME##_end(NAME* vec)                                                                                  \
    {                                                                                                      \
        return vec->data + vec->size;                                                                      \
    }

// Default vectors
TH_DEFINE_VEC(th_buf_vec, char, (void))

/* End of th_vec.h */
/* Start of th_string.h */


typedef struct th_detail_large_string {
    size_t capacity;
    size_t len;
    char* ptr;
    th_allocator* allocator;
} th_detail_large_string;

#define TH_STRING_SMALL_BUF_LEN (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 1)
#define TH_STRING_SMALL_MAX_LEN (TH_STRING_SMALL_BUF_LEN - 1)
typedef struct th_detail_small_string {
    unsigned char small : 1;
    unsigned char len : 7;
    char buf[TH_STRING_SMALL_BUF_LEN];
    th_allocator* allocator;
} th_detail_small_string;

typedef struct th_string {
    union {
        th_detail_small_string small;
        th_detail_large_string large;
    } impl;
} th_string;

TH_PRIVATE(void)
th_string_init(th_string* self, th_allocator* allocator);

TH_PRIVATE(th_err)
th_string_init_with(th_string* self, th_str str, th_allocator* allocator);

TH_PRIVATE(th_err)
th_string_set(th_string* self, th_str str);

TH_PRIVATE(th_err)
th_string_append(th_string* self, th_str str);

TH_PRIVATE(th_err)
th_string_append_cstr(th_string* self, const char* str);

TH_PRIVATE(th_err)
th_string_push_back(th_string* self, char c);

TH_PRIVATE(th_err)
th_string_resize(th_string* self, size_t new_len, char fill);

TH_PRIVATE(th_str)
th_string_view(const th_string* self);

TH_PRIVATE(char*)
th_string_at(th_string* self, size_t index);

TH_PRIVATE(const char*)
th_string_data(const th_string* self);

TH_PRIVATE(size_t)
th_string_len(const th_string* self);

TH_PRIVATE(void)
th_string_deinit(th_string* self);

TH_PRIVATE(void)
th_string_clear(th_string* self);

TH_PRIVATE(void)
th_string_to_lower(th_string* self);

TH_PRIVATE(bool)
th_string_eq(const th_string* self, th_str other);

// TH_PRIVATE(uint32_t)
// th_string_hash(const th_string* self);

TH_DEFINE_VEC(th_string_vec, th_string, th_string_deinit)

/* End of th_string.h */
/* Start of th_dir_mgr.h */



TH_DEFINE_HASHMAP(th_dir_map, th_str, th_dir, th_str_hash, th_str_eq, (th_str){0})

typedef struct th_dir_mgr {
    th_allocator* allocator;
    th_dir_map map;
    th_string_vec strings;
} th_dir_mgr;

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator);

/** th_dir_mgr_add
 * @brief Registers dir under label. dir must already be open (see
 * th_dir_open); ownership always moves into this call, so the caller must
 * not touch or deinit dir afterwards, whether or not it succeeds.
 */
TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_str label, th_dir dir);

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_str label);

TH_PRIVATE(void)
th_dir_mgr_deinit(th_dir_mgr* mgr);

/* End of th_dir_mgr.h */
/* Start of th_part.h */



struct th_part {
    th_string name;
    th_string filename;
    th_string content_type;
    th_str content;
};

TH_PRIVATE(void)
th_part_init(th_part* part, th_str content, th_allocator* allocator);

TH_PRIVATE(void)
th_part_deinit(th_part* part);

TH_PRIVATE(th_err)
th_part_set_name(th_part* part, th_str name);

TH_PRIVATE(th_err)
th_part_set_filename(th_part* part, th_str filename);

TH_PRIVATE(th_err)
th_part_set_content_type(th_part* part, th_str content_type);

/* End of th_part.h */
/* Start of th_request.h */



struct th_iter_methods {
    bool (*next)(th_iter* it);
    const char* (*key)(const th_iter* it);
    const void* (*val)(const th_iter* it);
};

typedef struct th_hstr_pair {
    th_string key;
    th_string value;
} th_hstr_pair;

TH_INLINE(void)
th_hstr_pair_deinit(th_hstr_pair* pair)
{
    th_string_deinit(&pair->key);
    th_string_deinit(&pair->value);
}

TH_DEFINE_VEC(th_hstr_vec, th_hstr_pair, th_hstr_pair_deinit)

TH_DEFINE_VEC(th_part_vec, th_part, th_part_deinit)

struct th_request {
    th_allocator* allocator;
    th_string uri_path;
    th_string uri_query;
    th_part_vec parts;
    th_hstr_vec cookies;
    th_hstr_vec headers;
    th_hstr_vec queryvars;
    th_hstr_vec formvars;
    th_hstr_vec pathvars;
    th_str body;
    th_method method;
    int version;
    bool close;
};

TH_PRIVATE(void)
th_request_init(th_request* request, th_allocator* allocator);

TH_PRIVATE(void)
th_request_deinit(th_request* request);

TH_PRIVATE(void)
th_request_reset(th_request* request);

TH_PRIVATE(void)
th_request_set_version(th_request* request, int version);

TH_PRIVATE(void)
th_request_set_method(th_request* request, th_method method);

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_str path);

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_str query);

TH_PRIVATE(th_err)
th_request_add_queryvar(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_str key, th_str value);

TH_PRIVATE(th_err)
th_request_add_part(th_request* request, th_str content, th_str name, th_str filename, th_str content_type);

TH_PRIVATE(void)
th_request_clear_queryvars(th_request* request);

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_str body);

TH_PRIVATE(th_str)
th_request_get_header(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_str)
th_request_get_pathvar(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_str)
th_request_get_queryvar(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_str)
th_request_get_formvar(th_request* request, th_str key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_part*)
th_request_get_part(th_request* request, th_str key) TH_MAYBE_UNUSED;

/* End of th_request.h */
/* Start of th_request_parser.h */



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
th_request_parser_parse(th_request_parser* parser, th_request* request, th_str data, size_t* parsed);

TH_PRIVATE(bool)
th_request_parser_header_done(th_request_parser* parser);

TH_PRIVATE(bool)
th_request_parser_done(th_request_parser* parser);

/* End of th_request_parser.h */
/* Start of th_refcounted.h */



typedef struct th_refcounted {
    unsigned int refcount;
    void (*destroy)(void* self);
} th_refcounted;

TH_INLINE(void)
th_refcounted_init(th_refcounted* refcounted, void (*destroy)(void* self))
{
    refcounted->refcount = 1;
    refcounted->destroy = destroy;
}

TH_INLINE(th_refcounted*)
th_refcounted_ref(th_refcounted* refcounted)
{
    ++refcounted->refcount;
    return refcounted;
}

TH_INLINE(void)
th_refcounted_unref(th_refcounted* refcounted)
{
    TH_ASSERT(refcounted->refcount > 0 && "Invalid refcount");
    if (--refcounted->refcount == 0) {
        refcounted->destroy(refcounted);
    }
}

/* End of th_refcounted.h */
/* Start of th_clock.h */



#include <time.h>

/** th_clock
 * @brief Source of monotonic time for th_timer. Injected as a dependency so
 * tests can supply a fully controllable clock instead of the real one.
 */
typedef struct th_clock {
    /** monotonic_now
     * @brief Write the current monotonic time (in seconds) to *out.
     * @return TH_ERR_OK on success, TH_ERR_SYSTEM(errno) on failure.
     */
    th_err (*monotonic_now)(void* self, time_t* out);
} th_clock;

/** th_clock_os
 * @brief The real, OS-backed clock (POSIX clock_gettime / Windows GetTickCount64).
 */
TH_PRIVATE(th_clock*)
th_clock_os(void);

/* End of th_clock.h */
/* Start of th_timer.h */



#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct th_timer {
    th_clock* clock;
    time_t expire;
} th_timer;

/** th_timer_init
 * @brief Initialize a timer, unexpired, using the given clock as its time
 * source. Pass th_clock_os() in production; tests can supply a fake clock.
 */
TH_PRIVATE(void)
th_timer_init(th_timer* timer, th_clock* clock);

/** th_timer_from_duration
 * @brief Create a timer that expires after the given duration.
 * Equivalent to th_timer_init followed by th_timer_set, but the assert-only
 * error handling of th_timer_set means this can never fail in practice.
 */
TH_PRIVATE(th_timer)
th_timer_from_duration(th_clock* clock, th_duration duration);

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration);

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer);

/** th_timer_remaining
 * @brief Time left until the timer expires, clamped to zero (never negative).
 */
TH_PRIVATE(th_duration)
th_timer_remaining(const th_timer* timer);

/** th_timer_less
 * @brief True if `a` expires before `b`. For use in timer lists/heaps.
 */
TH_PRIVATE(bool)
th_timer_less(const th_timer* a, const th_timer* b);

/* End of th_timer.h */
/* Start of th_fcache.h */



typedef struct th_fcache th_fcache;
typedef struct th_fcache_entry th_fcache_entry;
struct th_fcache_entry {
    th_refcounted base;
    th_file stream;
    th_string path;
    th_dir* dir;
    th_allocator* allocator;
    th_fcache* cache;
    th_fcache_entry* next;
    th_fcache_entry* prev;
    uint32_t stat_hash;
};

typedef struct th_fcache_id {
    th_str path;
    th_dir* dir;
} th_fcache_id;

TH_INLINE(bool)
th_fcache_id_eq(th_fcache_id a, th_fcache_id b)
{
    return a.dir == b.dir && th_str_eq(a.path, b.path);
}

TH_INLINE(size_t)
th_fcache_id_hash(th_fcache_id id)
{
    return th_str_hash(id.path) + (size_t)id.dir->fd;
}

TH_DEFINE_HASHMAP(th_fcache_map, th_fcache_id, th_fcache_entry*, th_fcache_id_hash, th_fcache_id_eq, (th_fcache_id){0})
TH_DEFINE_LIST(th_fcache_list, th_fcache_entry, prev, next)

struct th_fcache {
    th_allocator* allocator;
    th_file_ops* file_ops;
    th_fcache_map map;
    th_fcache_list list;
    size_t num_cached;
    size_t max_cached;
};

// fcache entry functions

TH_PRIVATE(void)
th_fcache_entry_unref(th_fcache_entry* entry);

// fcache functions

TH_PRIVATE(void)
th_fcache_init(th_fcache* cache, th_file_ops* file_ops, th_allocator* allocator);

TH_PRIVATE(th_err)
th_fcache_get(th_fcache* cache, th_dir* dir, th_str path, th_fcache_entry** out);

TH_PRIVATE(void)
th_fcache_deinit(th_fcache* cache);

/* End of th_fcache.h */
/* Start of th_header_id.h */


#include <stddef.h>
#include <stdint.h>

typedef enum th_header_id {
    TH_HEADER_ID_CONNECTION,
    TH_HEADER_ID_CONTENT_LENGTH,
    TH_HEADER_ID_CONTENT_TYPE,
    TH_HEADER_ID_DATE,
    TH_HEADER_ID_SERVER,
    TH_HEADER_ID_COOKIE,
    TH_HEADER_ID_TRANSFER_ENCODING,
    TH_HEADER_ID_RANGE,
    TH_HEADER_ID_MAX,
    TH_HEADER_ID_UNKNOWN = TH_HEADER_ID_MAX,
} th_header_id;

struct th_header_id_mapping {
    const char* name;
    th_header_id id;
};

struct th_header_id_mapping*
th_header_id_mapping_find(const char* name, size_t len);

TH_INLINE(th_header_id)
th_header_id_from_string(const char* name, size_t len)
{
    struct th_header_id_mapping* mapping = th_header_id_mapping_find(name, (unsigned int)len);
    return mapping ? mapping->id : TH_HEADER_ID_UNKNOWN;
}

/* End of th_header_id.h */
/* Start of th_response.h */


#include <stdarg.h>

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

TH_PRIVATE(void)
th_response_async_write(th_response* response, th_conn* conn, th_send_cb callback, void* user_data);

/* End of th_response.h */
/* Start of th_router.h */



typedef struct th_route_handler {
    th_handler handler;
    void* user_data;
} th_route_handler;

typedef struct th_capture {
    th_str key;
    th_str value;
} th_capture;

typedef enum th_capture_type {
    TH_CAPTURE_TYPE_NONE = 0,
    TH_CAPTURE_TYPE_INT,
    TH_CAPTURE_TYPE_STRING,
    TH_CAPTURE_TYPE_PATH,
} th_capture_type;

typedef struct th_route_segment th_route_segment;
struct th_route_segment {
    th_capture_type type;
    th_string name;
    th_route_handler handler[TH_METHOD_MAX];
    th_route_segment* next;
    th_route_segment* children;
    th_allocator* allocator;
};

typedef struct th_router {
    th_route_segment* routes;
    th_allocator* allocator;
} th_router;

TH_PRIVATE(void)
th_router_init(th_router* router, th_allocator* allocator);

TH_PRIVATE(void)
th_router_deinit(th_router* router);

TH_PRIVATE(th_err)
th_router_handle(th_router* router, th_request* request, th_response* response);

/** th_router_would_handle
 *  Check if the router would handle the request, if
 * it was to be passed with the given method (and not the actual method in the request).
 */
TH_PRIVATE(bool)
th_router_would_handle(th_router* router, th_method method, th_request* request);

TH_PRIVATE(th_err)
th_router_add_route(th_router* router, th_method method, th_str route, th_handler handler, void* user_data);

/* End of th_router.h */
/* Start of th_http.h */



typedef struct th_http th_http;

struct th_http {
    const th_conn_tracker* tracker;
    th_request_parser parser;
    th_request request;
    th_response response;
    th_buf_vec buf;
    th_conn* conn;
    th_router* router;
    th_dir_mgr* dir_mgr;
    th_fcache* fcache;
    th_allocator* allocator;
    size_t read_bytes;
    size_t parsed_bytes;

    // true if the connection should be closed
    bool close;
};

typedef struct th_http_upgrader {
    th_conn_upgrader base;
    const th_conn_tracker* tracker;
    th_router* router;
    th_dir_mgr* dir_mgr;
    th_fcache* fcache;
    th_allocator* allocator;
} th_http_upgrader;

TH_PRIVATE(void)
th_http_upgrader_init(th_http_upgrader* upgrader, const th_conn_tracker* tracker, th_router* router,
                      th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator);

/* End of th_http.h */
/* Start of th_ssl_ops.h */


#if TH_WITH_SSL

#include <openssl/ssl.h>

/** th_ssl_ops
 * @brief The raw OpenSSL calls th_ssl_session/th_ssl_context perform.
 * Injected at construction time so tests can fake SSL without a real
 * SSL_CTX/SSL/BIO. Each call mirrors the underlying OpenSSL function
 * directly (same return value meaning), so callers can interpret
 * results (and call SSL_get_error on failure) themselves.
 */
typedef struct th_ssl_ops {
    /* SSL_CTX (th_ssl_context) */
    SSL_CTX* (*ctx_new)(void* self);
    void (*ctx_free)(void* self, SSL_CTX* ctx);
    int (*ctx_use_certificate_chain_file)(void* self, SSL_CTX* ctx, const char* cert);
    int (*ctx_use_private_key_file)(void* self, SSL_CTX* ctx, const char* key);
    int (*ctx_set_min_proto_version)(void* self, SSL_CTX* ctx);
    int (*ctx_set_cipher_list)(void* self, SSL_CTX* ctx, const char* ciphers);
    void (*ctx_set_session_cache_off)(void* self, SSL_CTX* ctx);

    /* SSL (th_ssl_session) */
    SSL* (*new_ssl)(void* self, SSL_CTX* ctx);
    void (*free_ssl)(void* self, SSL* ssl);
    void (*set_bio)(void* self, SSL* ssl, BIO* rbio, BIO* wbio);
    void (*set_accept_state)(void* self, SSL* ssl);
    void (*set_partial_write)(void* self, SSL* ssl);
    int (*do_handshake)(void* self, SSL* ssl);
    int (*read)(void* self, SSL* ssl, void* buf, int len);
    int (*write)(void* self, SSL* ssl, const void* buf, int len);
    int (*get_error)(void* self, SSL* ssl, int ret);
} th_ssl_ops;

TH_PRIVATE(th_ssl_ops*)
th_ssl_ops_os(void);

#endif
/* End of th_ssl_ops.h */
/* Start of th_ssl_context.h */


#if TH_WITH_SSL


#include <openssl/ssl.h>

typedef struct th_ssl_context {
    SSL_CTX* ctx;
    BIO_METHOD* smem_method;
    th_ssl_ops* ops;
} th_ssl_context;

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, th_ssl_ops* ops, const char* key, const char* cert);

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context);

#endif
/* End of th_ssl_context.h */
/* Start of th_listener.h */


typedef struct th_listener th_listener;

typedef struct th_listener_conn_destroy_handler {
    th_task base;
    th_listener* listener;
} th_listener_conn_destroy_handler;

struct th_listener {
    th_acceptor acceptor;
    th_address accept_addr;
    th_listener* next;
    th_loop* loop;

    /** The conn that will be used to handle the incoming connections. */
    th_conn* conn;

    /** Used to keep track of all the clients that are currently active. */
    th_conn_tracker conn_tracker;

    /** Used to react to the destruction of a client. */
    th_listener_conn_destroy_handler client_destroy_handler;

    th_http_upgrader upgrader;

    /** The accept op that will be used to handle the completion
     * of the accept operation.
     */
    th_accept_op accept_op;

#if TH_WITH_SSL
    /** Ssl context used to create SSL connections, when ssl_enabled. */
    th_ssl_context ssl_context;
#endif

    /** As long as the listener keeps accepting new connections,
     * this flag will be set to 1.
     */
    bool running;

    /** Set once th_listener_enable_ssl succeeds; incoming connections
     * are then accepted as th_ssl_conn instead of th_tcp_conn.
     */
    bool ssl_enabled;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_loop* loop,
                   const char* host, const char* port,
                   th_router* router, th_dir_mgr* dir_mgr, th_fcache* fcache,
                   th_bind_opt* opt, th_allocator* allocator);

TH_PRIVATE(th_err)
th_listener_start(th_listener* listener);

TH_PRIVATE(void)
th_listener_stop(th_listener* listener);

TH_PRIVATE(void)
th_listener_destroy(th_listener* listener);

/* End of th_listener.h */
/* Start of th_mime.h */



struct th_mime_mapping {
    const char* name;
    th_str mime;
};

struct th_mime_mapping* th_mime_mapping_find(const char* ext, size_t len);

/* End of th_mime.h */
/* Start of th_multipart_parser.h */



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

/* End of th_multipart_parser.h */
/* Start of th_poll.h */



#if !defined(TH_CONFIG_OS_WIN)
#include <poll.h>
#include <sys/types.h>

/** th_pollops
 * @brief The poll(2) syscall, injected so tests can control fd readiness
 * without a real fd. th_pollops_os() is the real implementation.
 */
typedef struct th_pollops {
    int (*poll)(void* self, struct pollfd* fds, nfds_t nfds, int timeout_ms);
} th_pollops;

TH_PRIVATE(th_pollops*)
th_pollops_os(void);

/** th_poll_create
 * @brief Create a poll-based reactor.
 * @param loop The th_loop this reactor will be registered with (via
 * loop->reactor, set by the caller after this returns — th_loop_init
 * must run first since it doesn't require a reactor yet). Used to keep
 * loop's task count in sync with ops the reactor is holding pending for
 * readiness, which otherwise aren't visible to th_loop_poll's own queue.
 * @param clock Clock used for per-handle I/O timeouts.
 * @param ops The poll(2) implementation to use; pass th_pollops_os() in
 * production, a fake in tests.
 */
TH_PRIVATE(th_err)
th_poll_create(th_reactor** out, th_loop* loop, th_allocator* allocator, th_clock* clock, th_pollops* ops);

#endif /* !TH_CONFIG_OS_WIN */
/* End of th_poll.h */
/* Start of th_sendfile.h */



/** th_sendfile_op
 * @brief Sends header (iov/iovcnt, may be empty) followed by len bytes of
 * file starting at offset, retrying in TH_CONFIG_SENDFILE_CHUNK_LEN-sized
 * steps until every byte (header + file) has been written or an error
 * occurs. iov is mutated in place as header buffers are consumed. After
 * init, start with th_op_perform(&op->base). On completion the op posts
 * itself to the socket's loop and callback runs from that later drain,
 * never the caller's stack.
 */
typedef struct th_sendfile_op {
    th_op base;
    th_socket* socket;
    th_send_cb callback;
    void* user_data;
    th_iov* iov;
    size_t iovcnt;
    th_file* file;
    size_t offset;
    size_t len;
    size_t header_len;
    size_t pos;
    th_err err;
} th_sendfile_op;

TH_PRIVATE(void)
th_sendfile_op_init(th_sendfile_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data);

/* End of th_sendfile.h */
/* Start of th_sendvec.h */



/** th_sendvec_op
 * @brief Writes an iovec to a th_socket, retrying until every byte
 * across all buffers has been written or an error occurs. iov is
 * mutated in place as buffers are consumed. After init, start with
 * th_op_perform(&op->base). On completion the op posts itself to the
 * socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_sendvec_op {
    th_op base;
    th_socket* socket;
    th_send_cb callback;
    void* user_data;
    th_iov* iov;
    size_t iovcnt;
    size_t pos;
    th_err err;
} th_sendvec_op;

TH_PRIVATE(void)
th_sendvec_op_init(th_sendvec_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_send_cb callback, void* user_data);

/* End of th_sendvec.h */
/* Start of th_ssl_conn.h */


#if TH_WITH_SSL


/** th_ssl_conn_create
 * @brief Allocates and initializes an SSL th_conn, taking ownership of
 * socket by value (the caller's th_socket is moved in, not referenced —
 * construct it with th_socket_init and don't use it again after this
 * call). The returned conn has no fd yet; set one via
 * th_socket_set_fd(th_conn_get_socket(conn), fd) before use. The SSL
 * handshake only runs once th_conn_start is called, not at creation.
 */
TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_socket* socket, th_ssl_context* ssl_context, th_ssl_ops* ssl_ops,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

#endif
/* End of th_ssl_conn.h */
/* Start of th_ssl_error.h */


#if TH_WITH_SSL


TH_PRIVATE(const char*)
th_ssl_strerror(int code);

#endif // TH_WITH_SSL
/* End of th_ssl_error.h */
/* Start of th_ssl_session.h */


#if TH_WITH_SSL


#include <openssl/ssl.h>

/** th_ssl_result
 * @brief Outcome of one th_ssl_session step. TH_SSL_WANT_READ means more
 * ciphertext must be fed in (via fed_ciphertext_in) before retrying;
 * TH_SSL_WANT_WRITE means pending ciphertext (get_ciphertext_out) must be
 * drained before retrying.
 */
typedef enum th_ssl_result {
    TH_SSL_DONE,
    TH_SSL_WANT_READ,
    TH_SSL_WANT_WRITE,
    TH_SSL_ERROR,
} th_ssl_result;

/** th_ssl_session
 * @brief Drives an OpenSSL handshake/read/write over a pair of memory
 * BIOs. Has no knowledge of th_conn/th_socket/the reactor — purely
 * plaintext in/out on one side, ciphertext in/out on the other; the
 * caller is responsible for shuttling ciphertext to/from a real socket.
 */
typedef struct th_ssl_session {
    SSL* ssl;
    BIO* rbio;
    BIO* wbio;
    th_ssl_ops* ops;
} th_ssl_session;

TH_PRIVATE(th_err)
th_ssl_session_init(th_ssl_session* session, th_ssl_context* context, th_ssl_ops* ops, th_allocator* allocator);

TH_PRIVATE(void)
th_ssl_session_deinit(th_ssl_session* session);

TH_PRIVATE(th_ssl_result)
th_ssl_session_handshake(th_ssl_session* session, th_err* err);

TH_PRIVATE(th_ssl_result)
th_ssl_session_read(th_ssl_session* session, void* buf, size_t len, size_t* out, th_err* err);

TH_PRIVATE(th_ssl_result)
th_ssl_session_write(th_ssl_session* session, const void* buf, size_t len, size_t* out, th_err* err);

/** th_ssl_session_get_ciphertext_out
 * @brief Ciphertext produced by the last handshake/read/write step that
 * still needs to be sent over the real socket.
 */
TH_PRIVATE(void)
th_ssl_session_get_ciphertext_out(th_ssl_session* session, th_iov* iov);

TH_PRIVATE(void)
th_ssl_session_consume_ciphertext_out(th_ssl_session* session, size_t n);

/** th_ssl_session_get_ciphertext_in_buf
 * @brief Spare capacity to recv() real-socket ciphertext into.
 */
TH_PRIVATE(void)
th_ssl_session_get_ciphertext_in_buf(th_ssl_session* session, th_iov* iov);

TH_PRIVATE(void)
th_ssl_session_fed_ciphertext_in(th_ssl_session* session, size_t n);

#endif
/* End of th_ssl_session.h */
/* Start of th_ssl_io.h */


#if TH_WITH_SSL


typedef void (*th_ssl_io_cb)(void* user_data, size_t size, th_err err);

typedef enum th_ssl_io_kind {
    TH_SSL_IO_HANDSHAKE,
    TH_SSL_IO_READ,
    TH_SSL_IO_WRITE,
} th_ssl_io_kind;

/** th_ssl_io_op
 * @brief Drives one th_ssl_session step (handshake/read/write) to
 * completion, shuttling ciphertext to/from socket in between as the
 * session reports TH_SSL_WANT_READ/TH_SSL_WANT_WRITE. After init, start
 * with th_op_perform(&op->base). On completion the op posts itself to
 * the socket's loop and callback runs from that later drain, never the
 * caller's stack.
 */
typedef struct th_ssl_io_op {
    th_op base;
    th_socket* socket;
    th_ssl_session* session;
    th_ssl_io_kind kind;
    void* buf; /* plaintext in (READ) / plaintext out (WRITE), unused for HANDSHAKE */
    size_t len;
    size_t result;
    bool shuttling_write; /* mid raw-socket-send draining ciphertext out */
    bool draining;        /* plaintext progress made; finish the shuttle, don't step again */
    th_ssl_io_cb callback;
    void* user_data;
    th_err err;
} th_ssl_io_op;

TH_PRIVATE(void)
th_ssl_io_op_init_handshake(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, th_ssl_io_cb callback, void* user_data);

TH_PRIVATE(void)
th_ssl_io_op_init_read(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, void* buf, size_t len, th_ssl_io_cb callback, void* user_data);

TH_PRIVATE(void)
th_ssl_io_op_init_write(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, const void* buf, size_t len, th_ssl_io_cb callback, void* user_data);

#endif
/* End of th_ssl_io.h */
/* Start of th_ssl_recv.h */


#if TH_WITH_SSL


#include <stdbool.h>

/** th_ssl_recv_op
 * @brief Reads plaintext from a th_ssl_session (shuttling ciphertext over
 * socket as needed) into addr. If exact is false, completes as soon as
 * any bytes arrive (0 bytes => TH_ERR_EOF); if true, retries until
 * exactly len bytes have been read or an error/EOF occurs. After init,
 * the first th_ssl_io_op read is already in flight (no separate perform
 * call needed, unlike th_recv_op).
 */
typedef struct th_ssl_recv_op {
    th_ssl_io_op io;
    th_socket* socket;
    th_ssl_session* session;
    th_recv_cb callback;
    void* user_data;
    void* addr;
    size_t len;
    size_t pos;
    bool exact;
} th_ssl_recv_op;

TH_PRIVATE(void)
th_ssl_recv_op_init(th_ssl_recv_op* op, th_socket* socket, th_ssl_session* session, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data);

#endif
/* End of th_ssl_recv.h */
/* Start of th_ssl_send.h */


#if TH_WITH_SSL


#define TH_SSL_SEND_CHUNK_LEN (16 * 1024)

/** th_ssl_send_op
 * @brief Writes iov (mutated in place as buffers are consumed) as
 * plaintext through a th_ssl_session (shuttling ciphertext over socket
 * as needed), followed by len bytes of file starting at offset if file
 * is non-NULL, retrying in TH_SSL_SEND_CHUNK_LEN-sized steps until every
 * byte has been written or an error occurs. After init, the first
 * th_ssl_io_op write is already in flight.
 */
typedef struct th_ssl_send_op {
    th_ssl_io_op io;
    th_socket* socket;
    th_ssl_session* session;
    th_send_cb callback;
    void* user_data;
    th_iov* iov;
    size_t iovcnt;
    th_file* file;
    size_t offset;
    size_t len;
    size_t file_pos;
    size_t pos;
    char buffer[TH_SSL_SEND_CHUNK_LEN];
} th_ssl_send_op;

TH_PRIVATE(void)
th_ssl_send_op_init(th_ssl_send_op* op, th_socket* socket, th_ssl_session* session,
                    th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len,
                    th_send_cb callback, void* user_data);

#endif
/* End of th_ssl_send.h */
/* Start of th_ssl_smem_bio.h */


#if TH_WITH_SSL

#include <openssl/bio.h>


TH_PRIVATE(BIO_METHOD*)
th_smem_bio(th_ssl_context* ssl_context);

TH_PRIVATE(void)
th_smem_bio_setup_buf(BIO* bio, th_allocator* allocator, size_t max_len);

TH_PRIVATE(size_t)
th_smem_ensure_buf_size(BIO* bio, size_t size);

TH_PRIVATE(void)
th_smem_bio_set_eof(BIO* bio);

TH_PRIVATE(void)
th_smem_bio_get_rdata(BIO* bio, th_iov* buf);

TH_PRIVATE(void)
th_smem_bio_get_wbuf(BIO* bio, th_iov* buf);

TH_PRIVATE(void)
th_smem_bio_inc_read_pos(BIO* bio, size_t len);

TH_PRIVATE(void)
th_smem_bio_inc_write_pos(BIO* bio, size_t len);

#endif
/* End of th_ssl_smem_bio.h */
/* Start of th_tcp_conn.h */



/** th_tcp_conn_create
 * @brief Allocates and initializes a plain (non-SSL) th_conn, taking
 * ownership of socket by value (the caller's th_socket is moved in, not
 * referenced — construct it with th_socket_init and don't use it again
 * after this call). The returned conn has no fd yet; set one via
 * th_socket_set_fd(th_conn_get_socket(conn), fd) before use.
 */
TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_socket* socket,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

/* End of th_tcp_conn.h */
/* Start of th_url_decode.h */



#include <stddef.h>

typedef enum th_url_decode_type {
    TH_URL_DECODE_TYPE_PATH = 0,
    TH_URL_DECODE_TYPE_QUERY
} th_url_decode_type;

TH_PRIVATE(th_err)
th_url_decode_string(th_str input, th_string* output, th_url_decode_type type);

/* End of th_url_decode.h */
/* Start of th_align.h */

#include <stdint.h>

#define TH_ALIGNOF(type) ((size_t)&(((struct { char c; type member; }*)0)->member))
#define TH_ALIGNAS(align, ptr) ((void*)(((uintptr_t)(ptr) + ((uintptr_t)(align) - 1)) & ~((uintptr_t)(align) - 1)))
#define TH_ALIGNUP(n, align) (((n) + (size_t)(align) - 1) & ~((size_t)(align) - 1))
#define TH_ALIGNDOWN(n, align) ((n) & ~((align) - 1))

typedef long double th_max_align;

/* End of th_align.h */
/* Start of src/th_server.c */


struct th_server {
    th_reactor* reactor;
    th_loop loop;
    th_router router;
    th_dir_mgr dir_mgr;
    th_fcache fcache;
    th_listener* listeners;
    th_allocator* allocator;
};

TH_LOCAL(th_err)
th_server_init(th_server* server, th_allocator* allocator)
{
    th_router_init(&server->router, allocator);
    th_err err = TH_ERR_OK;
    th_loop_init(&server->loop, NULL);
    if ((err = th_poll_create(&server->reactor, &server->loop, allocator, th_clock_os(), th_pollops_os())) != TH_ERR_OK)
        goto cleanup_router;
    server->loop.reactor = server->reactor;
    th_dir_mgr_init(&server->dir_mgr, allocator);
    th_fcache_init(&server->fcache, th_file_ops_os(), allocator);
    server->listeners = NULL;
    server->allocator = allocator;
cleanup_router:
    th_router_deinit(&server->router);
    return err;
}

TH_LOCAL(void)
th_server_stop(th_server* server)
{
    th_listener* listener = server->listeners;
    while (listener) {
        th_listener_stop(listener);
        listener = listener->next;
    }
    th_loop_run(&server->loop);
}

TH_LOCAL(void)
th_server_deinit(th_server* server)
{
    th_listener* listener = server->listeners;
    while (listener) {
        th_listener* next = listener->next;
        th_listener_destroy(listener);
        listener = next;
    }
    th_loop_deinit(&server->loop);
    th_reactor_destroy(server->reactor);
    th_router_deinit(&server->router);
    th_fcache_deinit(&server->fcache);
    th_dir_mgr_deinit(&server->dir_mgr);
}

TH_LOCAL(th_err)
th_server_bind(th_server* server, const char* host, const char* port, th_bind_opt* opt)
{
    th_listener* listener = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_create(&listener, &server->loop,
                                  host, port,
                                  &server->router, &server->dir_mgr, &server->fcache,
                                  opt, server->allocator))
        != TH_ERR_OK) {
        return err;
    }
    if ((err = th_listener_start(listener)) != TH_ERR_OK) {
        th_listener_destroy(listener);
        return err;
    }
    listener->next = server->listeners;
    server->listeners = listener;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_server_route(th_server* server, th_method method, const char* path, th_handler handler, void* user_data)
{
    return th_router_add_route(&server->router, method, th_str_from_cstr(path), handler, user_data);
}

TH_LOCAL(th_err)
th_server_add_dir(th_server* server, const char* name, const char* path)
{
    th_dir dir;
    th_dir_init(&dir, th_dir_ops_os());
    th_err err = TH_ERR_OK;
    if ((err = th_dir_open(&dir, th_str_from_cstr(path))) != TH_ERR_OK) {
        th_dir_deinit(&dir);
        return err;
    }
    return th_dir_mgr_add(&server->dir_mgr, th_str_from_cstr(name), dir);
}

TH_LOCAL(th_err)
th_server_save_to_disk(th_server* server, th_buffer data, const char* dir_label, const char* filepath)
{
    th_dir* dir = th_dir_mgr_get(&server->dir_mgr, th_str_from_cstr(dir_label));
    if (!dir)
        return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
    th_err err = TH_ERR_OK;
    th_filepath path;
    if ((err = th_filepath_init(&path, th_str_from_cstr(filepath))) != TH_ERR_OK)
        return err;
    th_open_opt opt = {.create = true, .write = true, .truncate = true};
    th_file file;
    th_file_init(&file, server->fcache.file_ops);
    if ((err = th_file_openat(&file, dir, &path, opt)) != TH_ERR_OK)
        return err;
    size_t total_written = 0;
    while (total_written < data.len) {
        size_t written = 0;
        if ((err = th_file_write(&file, data.ptr + total_written, data.len - total_written, total_written, &written))
            != TH_ERR_OK) {
            th_file_close(&file);
            return err;
        }
        total_written += written;
    }
    th_file_close(&file);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_server_poll(th_server* server, int timeout_ms)
{
    return th_loop_poll(&server->loop, timeout_ms);
}

/* public server API */

TH_PUBLIC(th_err)
th_server_create(th_server** out, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_server* server = th_allocator_alloc(allocator, sizeof(th_server));
    if (!server)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_server_init(server, allocator)) != TH_ERR_OK) {
        th_allocator_free(server->allocator, server);
        return err;
    }
    *out = server;
    return TH_ERR_OK;
}

TH_PUBLIC(void)
th_server_destroy(th_server* server)
{
    th_server_stop(server);
    th_server_deinit(server);
    th_allocator_free(server->allocator, server);
}

TH_PUBLIC(th_err)
th_bind(th_server* server, const char* addr, const char* port, th_bind_opt* opt)
{
    return th_server_bind(server, addr, port, opt);
}

TH_PUBLIC(th_err)
th_route(th_server* server, th_method method, const char* route, th_handler handler, void* userp)
{
    return th_server_route(server, method, route, handler, userp);
}

TH_PUBLIC(th_err)
th_add_dir(th_server* server, const char* name, const char* path)
{
    return th_server_add_dir(server, name, path);
}

TH_PUBLIC(th_err)
th_save_to_disk(th_server* server, th_buffer data, const char* dir_label, const char* filepath)
{
    return th_server_save_to_disk(server, data, dir_label, filepath);
}

TH_PUBLIC(th_err)
th_poll(th_server* server, int timeout_ms)
{
    return th_server_poll(server, timeout_ms);
}
/* End of src/th_server.c */
/* Start of src/th_listener.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#undef TH_LOG_TAG
#define TH_LOG_TAG "listener"

TH_LOCAL(th_err)
th_listener_enable_ssl(th_listener* listener, const char* key_file, const char* cert_file)
{
#if TH_WITH_SSL
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_context_init(&listener->ssl_context, th_ssl_ops_os(), key_file, cert_file)) != TH_ERR_OK)
        return err;
    listener->ssl_enabled = true;
    return TH_ERR_OK;
#else
    (void)listener;
    (void)key_file;
    (void)cert_file;
    TH_LOG_ERROR("SSL is not enabled in this build.");
    return TH_ERR_NOSUPPORT;
#endif
}

TH_LOCAL(th_err)
th_listener_init(th_listener* listener, th_loop* loop,
                 const char* host, const char* port,
                 th_router* router, th_dir_mgr* dir_mgr, th_fcache* fcache,
                 th_bind_opt* opt, th_allocator* allocator)
{
    listener->loop = loop;
    listener->running = 0;
    listener->ssl_enabled = false;
    listener->allocator = allocator ? allocator : th_default_allocator_get();
    th_err err = TH_ERR_OK;
    th_acceptor_init(&listener->acceptor, loop, th_acceptor_ops_os());
    if ((err = th_acceptor_open(&listener->acceptor, host, port)) != TH_ERR_OK)
        return err;
    if (opt && opt->key_file && opt->cert_file) {
        if ((err = th_listener_enable_ssl(listener, opt->key_file, opt->cert_file)) != TH_ERR_OK)
            goto cleanup_acceptor;
    }
    th_conn_tracker_init(&listener->conn_tracker);
    th_http_upgrader_init(&listener->upgrader, &listener->conn_tracker, router, dir_mgr, fcache, allocator);
    TH_LOG_INFO("Created listener on %s:%s", host, port);
    return TH_ERR_OK;
cleanup_acceptor:
    th_acceptor_deinit(&listener->acceptor);
    return err;
}

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_loop* loop,
                   const char* host, const char* port,
                   th_router* router, th_dir_mgr* dir_mgr, th_fcache* fcache,
                   th_bind_opt* opt, th_allocator* allocator)
{
    th_listener* listener = th_allocator_alloc(allocator, sizeof(th_listener));
    if (!listener)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_init(listener, loop, host, port, router, dir_mgr, fcache, opt, allocator)) != TH_ERR_OK)
        goto cleanup;
    *out = listener;
    return TH_ERR_OK;
cleanup:
    th_allocator_free(allocator, listener);
    return err;
}

TH_LOCAL(void)
th_listener_accept_complete(void* user_data, th_err err);

TH_LOCAL(th_err)
th_listener_async_accept(th_listener* listener)
{
    th_socket socket;
    th_socket_init(&socket, listener->loop, th_socket_ops_os());
    th_err err = TH_ERR_OK;
#if TH_WITH_SSL
    if (listener->ssl_enabled) {
        err = th_ssl_conn_create(&listener->conn, &socket, &listener->ssl_context, th_ssl_ops_os(),
                                 &listener->upgrader.base,
                                 (th_conn_observer*)&listener->conn_tracker,
                                 listener->allocator);
    } else
#endif
    {
        err = th_tcp_conn_create(&listener->conn, &socket,
                                 &listener->upgrader.base,
                                 (th_conn_observer*)&listener->conn_tracker,
                                 listener->allocator);
    }
    if (err != TH_ERR_OK) {
        return err;
    }
    th_accept_op_init(&listener->accept_op, &listener->acceptor, &listener->accept_addr,
                      th_conn_get_socket(listener->conn),
                      th_listener_accept_complete, listener);
    th_op_perform(&listener->accept_op.base);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_listener_client_destroy_handler_fn(void* self)
{
    th_listener* listener = self;
    if (!listener->running)
        return;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_async_accept(listener)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_conn_tracker_async_wait(&listener->conn_tracker, &listener->client_destroy_handler.base);
    }
}

TH_LOCAL(void)
th_listener_accept_complete(void* user_data, th_err err)
{
    th_listener* listener = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("Accept failed: %s", th_strerror(err));
        th_conn_destroy(TH_MOVE_PTR(listener->conn));
    } else {
        th_conn_start(listener->conn);
    }
    if (!listener->running) {
        return;
    }
    if ((err = th_listener_async_accept(listener)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to initiate accept: %s, try again later", th_strerror(err));
        th_conn_tracker_async_wait(&listener->conn_tracker, &listener->client_destroy_handler.base);
    }
}

TH_PRIVATE(th_err)
th_listener_start(th_listener* listener)
{
    // Client destroy handler
    listener->client_destroy_handler.listener = listener;
    th_task_init(&listener->client_destroy_handler.base, th_listener_client_destroy_handler_fn);
    listener->running = 1;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_async_accept(listener)) != TH_ERR_OK)
        return err;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_listener_stop(th_listener* listener)
{
    listener->running = 0;
    th_acceptor_cancel(&listener->acceptor);
    th_conn_tracker_cancel_all(&listener->conn_tracker);
}

TH_LOCAL(void)
th_listener_deinit(th_listener* listener)
{
    th_acceptor_deinit(&listener->acceptor);
    th_conn_tracker_deinit(&listener->conn_tracker);
#if TH_WITH_SSL
    if (listener->ssl_enabled)
        th_ssl_context_deinit(&listener->ssl_context);
#endif
}

TH_PRIVATE(void)
th_listener_destroy(th_listener* listener)
{
    th_listener_deinit(listener);
    th_allocator_free(listener->allocator, listener);
}
/* End of src/th_listener.c */
/* Start of src/th_router.c */

#include <assert.h>
#include <string.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "router"

TH_LOCAL(th_err)
th_route_init(th_route_segment* route, th_capture_type type, th_str segment, th_allocator* allocator)
{
    th_string_init(&route->name, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_string_set(&route->name, segment)) != TH_ERR_OK) {
        th_string_deinit(&route->name);
        return err;
    }
    route->type = type;
    route->next = NULL;
    route->children = NULL;
    route->allocator = allocator;
    for (size_t i = 0; i < TH_METHOD_MAX; ++i)
        route->handler[i] = (th_route_handler){NULL, NULL};
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_route_create(th_route_segment** out, th_capture_type type, th_str token, th_allocator* allocator)
{
    th_route_segment* route = th_allocator_alloc(allocator, sizeof(th_route_segment));
    if (!route)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_route_init(route, type, token, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, route);
        return err;
    }
    *out = route;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_route_destroy(th_route_segment* route);

TH_LOCAL(void)
th_route_deinit(th_route_segment* route)
{
    for (th_route_segment* child = route->children; child != NULL;) {
        th_route_segment* next = child->next;
        th_route_destroy(child);
        child = next;
    }
    th_string_deinit(&route->name);
}

TH_LOCAL(void)
th_route_destroy(th_route_segment* route)
{
    th_route_deinit(route);
    th_allocator_free(route->allocator, route);
}

TH_PRIVATE(void)
th_router_init(th_router* router, th_allocator* allocator)
{
    router->routes = NULL;
    router->allocator = allocator;
    if (!router->allocator)
        router->allocator = th_default_allocator_get();
}

TH_PRIVATE(void)
th_router_deinit(th_router* router)
{
    for (th_route_segment* route = router->routes; route != NULL;) {
        th_route_segment* next = route->next;
        th_route_destroy(route);
        route = next;
    }
}

TH_LOCAL(th_err)
th_route_consume_trail(th_route_segment* route, th_request* request, th_str* trail, bool dry, bool* result)
{
    th_str route_name = th_string_view(&route->name);
    th_str raw_segment = th_str_substr(*trail, 0, th_str_find_first_of(*trail, 0, "/?"));
    th_string decoded;
    bool decoded_init = false;
    th_str segment = raw_segment;
    th_err err = TH_ERR_OK;
    if (th_str_find_first(raw_segment, 0, '%') != th_str_npos) {
        th_string_init(&decoded, route->allocator);
        decoded_init = true;
        if ((err = th_url_decode_string(raw_segment, &decoded, TH_URL_DECODE_TYPE_PATH)) != TH_ERR_OK) {
            goto cleanup;
        }
        segment = th_string_view(&decoded);
    }
    switch (route->type) {
    case TH_CAPTURE_TYPE_NONE:
        if (th_str_eq(route_name, segment)) {
            *trail = th_str_substr(*trail, raw_segment.len + 1, th_str_npos);
            *result = true;
        }
        break;
    case TH_CAPTURE_TYPE_INT:
        if (th_str_is_uint(segment)) {
            if (!dry)
                (void)th_request_add_pathvar(request, route_name, segment);
            *trail = th_str_substr(*trail, raw_segment.len + 1, th_str_npos);
            *result = true;
        }
        break;
    case TH_CAPTURE_TYPE_STRING:
        if (!dry)
            (void)th_request_add_pathvar(request, route_name, segment);
        *trail = th_str_substr(*trail, raw_segment.len + 1, th_str_npos);
        *result = true;
        break;
    case TH_CAPTURE_TYPE_PATH:
        if (!dry)
            (void)th_request_add_pathvar(request, route_name, *trail);
        *trail = th_str_make(NULL, 0);
        *result = true;
        break;
    default:
        break;
    }
cleanup:
    if (decoded_init)
        th_string_deinit(&decoded);
    return err;
}

TH_LOCAL(th_err)
th_router_do_handle(th_router* router, th_method method, th_request* request, th_response* response, bool dry)
{
    TH_LOG_DEBUG("Handling request %p: %s", request, th_string_data(&request->uri_path));
    if (*th_string_at(&request->uri_path, 0) != '/')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_str trail = th_str_substr(th_string_view(&request->uri_path), 1, th_str_npos);
    th_route_segment* route = router->routes;
    while (1) {
        th_err err = TH_ERR_OK;
        bool consumed = false;
        if (route == NULL) {
            break;
        } else if ((err = th_route_consume_trail(route, request, &trail, dry, &consumed)) != TH_ERR_OK
                   || consumed) {
            if (err != TH_ERR_OK)
                return err;
            if (th_str_empty(trail))
                break;
            route = route->children;
        } else {
            route = route->next;
        }
    }
    if (route == NULL) {
        return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
    }
    th_route_handler handler = route->handler[method].handler ? route->handler[method] : route->handler[TH_METHOD_ANY];
    if (handler.handler == NULL) {
        return TH_ERR_HTTP(TH_CODE_METHOD_NOT_ALLOWED);
    }
    if (dry)
        return TH_ERR_OK;
    return handler.handler(handler.user_data, request, response);
}

TH_PRIVATE(th_err)
th_router_handle(th_router* router, th_request* request, th_response* response)
{
    return th_router_do_handle(router, request->method, request, response, false);
}

TH_PRIVATE(bool)
th_router_would_handle(th_router* router, th_method method, th_request* request)
{
    return th_router_do_handle(router, method, request, NULL, true) == TH_ERR_OK;
}

// abc < {int} < {string} < {path}
TH_LOCAL(bool)
th_route_lower(th_route_segment* lh, th_route_segment* rh)
{
    return lh->type < rh->type;
}

TH_LOCAL(void)
th_route_insert_sorted(th_route_segment** list, th_route_segment* route)
{
    while (*list != NULL && th_route_lower(*list, route))
        list = &(*list)->next;
    th_route_segment* temp = *list;
    *list = route;
    route->next = temp;
}

TH_LOCAL(th_err)
th_route_parse_trail(th_str* trail, th_str* name, th_capture_type* type)
{
    th_str segment = th_str_substr(*trail, 0, th_str_find_first_of(*trail, 0, "/"));
    size_t open_curly = th_str_find_first(segment, 0, '{');
    size_t close_curly = th_str_find_first(segment, 0, '}');
    if (segment.len > 2 && open_curly == 0 && close_curly == segment.len - 1) {
        th_str capture = th_str_substr(segment, 1, segment.len - 2);
        size_t sep = th_str_find_first(capture, 0, ':');
        if (sep == th_str_npos) {
            *name = capture;
            *type = TH_CAPTURE_TYPE_STRING;
        } else {
            th_str type_str = th_str_substr(capture, 0, sep);
            if (th_str_eq(type_str, TH_STR("int"))) {
                *name = th_str_substr(capture, sep + 1, th_str_npos);
                *type = TH_CAPTURE_TYPE_INT;
            } else if (th_str_eq(type_str, TH_STR("path"))) {
                *name = th_str_substr(capture, sep + 1, th_str_npos);
                *type = TH_CAPTURE_TYPE_PATH;
            } else {
                return TH_ERR_INVALID_ARG;
            }
        }
    } else if (open_curly == th_str_npos && close_curly == th_str_npos) {
        *name = segment;
        *type = TH_CAPTURE_TYPE_NONE;
    } else {
        return TH_ERR_INVALID_ARG;
    }
    // Consume segment
    *trail = th_str_substr(*trail, segment.len + 1, th_str_npos);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_router_add_route(th_router* router, th_method method, th_str path, th_handler handler, void* user_data)
{
    if (th_str_empty(path) || path.ptr[0] != '/')
        return TH_ERR_INVALID_ARG;
    th_str trail = th_str_substr(path, 1, th_str_npos);
    th_route_segment** list = &router->routes;
    th_route_segment* route = *list;

    // find a matching route
    bool last = false;
    while (!last) {
        th_str name = {0};
        th_capture_type type = TH_CAPTURE_TYPE_NONE;
        th_err err = TH_ERR_OK;
        if ((err = th_route_parse_trail(&trail, &name, &type)) != TH_ERR_OK)
            return err;
        last = th_str_empty(trail);
        if (type == TH_CAPTURE_TYPE_PATH && !last)
            return TH_ERR_INVALID_ARG;
        while (1) {
            if (route == NULL) {
                if ((err = th_route_create(&route, type, name, router->allocator)) != TH_ERR_OK)
                    return err;
                th_route_insert_sorted(list, route);
                route = *list; // restart
            }
            if ((type == TH_CAPTURE_TYPE_NONE
                 && th_str_eq(th_string_view(&route->name), name))
                || (type != TH_CAPTURE_TYPE_NONE && type == route->type)) {
                if (last)
                    break;
                list = &route->children;
                route = *list;
                break;
            } else {
                route = route->next;
            }
        }
    }

    if (route->handler[TH_METHOD_ANY].handler != NULL
        || route->handler[method].handler != NULL)
        return TH_ERR_INVALID_ARG; // Route already exists
    route->handler[method].handler = handler;
    route->handler[method].user_data = user_data;
    return TH_ERR_OK;
}
/* End of src/th_router.c */
/* Start of src/th_mime.c */
/* ANSI-C code produced by gperf version 3.2.1 */
/* Computed positions: -k'1,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif


#include <stddef.h>
#include <string.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wconversion"
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
struct th_mime_mapping;

#define TH_MIME_TOTAL_KEYWORDS 33
#define TH_MIME_MIN_WORD_LENGTH 2
#define TH_MIME_MAX_WORD_LENGTH 5
#define TH_MIME_MIN_HASH_VALUE 3
#define TH_MIME_MAX_HASH_VALUE 88
/* maximum key range = 86, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
th_mime_hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
       0, 45, 40, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 15, 89, 40,
       0, 89, 25, 10,  0,  1,  5, 89, 35, 40,
       0,  0, 20, 89, 89, 10, 35, 89,  0,  5,
      30, 89, 55, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
      89, 89, 89, 89, 89, 89
    };
  return len + asso_values[(unsigned char)str[len - 1]] + asso_values[(unsigned char)str[0]];
}

struct th_mime_mapping *
th_mime_mapping_find (register const char *str, register size_t len)
{
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
  static struct th_mime_mapping wordlist[] =
    {
      {""}, {""}, {""},
      {"ogv",  TH_STR_INIT("video/ogg")},
      {"ico",  TH_STR_INIT("image/x-icon")},
      {""}, {""}, {""},
      {"wav",  TH_STR_INIT("audio/wav")},
      {"json", TH_STR_INIT("application/json")},
      {"woff2",TH_STR_INIT("font/woff2")},
      {""}, {""},
      {"ogg",  TH_STR_INIT("audio/ogg")},
      {"opus", TH_STR_INIT("audio/opus")},
      {""}, {""},
      {"js",   TH_STR_INIT("text/javascript")},
      {"jpg",  TH_STR_INIT("image/jpeg")},
      {"jpeg", TH_STR_INIT("image/jpeg")},
      {""}, {""}, {""},
      {"svg",  TH_STR_INIT("image/svg+xml")},
      {"weba", TH_STR_INIT("audio/webm")},
      {""}, {""}, {""},
      {"otf",  TH_STR_INIT("font/otf")},
      {"webp", TH_STR_INIT("image/webp")},
      {""}, {""}, {""},
      {"png",  TH_STR_INIT("image/png")},
      {"woff", TH_STR_INIT("font/woff")},
      {""}, {""}, {""},
      {"gif",  TH_STR_INIT("image/gif")},
      {"html", TH_STR_INIT("text/html")},
      {""}, {""},
      {"md",   TH_STR_INIT("text/markdown")},
      {"csv",  TH_STR_INIT("text/csv")},
      {"avif", TH_STR_INIT("image/avif")},
      {""}, {""}, {""},
      {"pdf",  TH_STR_INIT("application/pdf")},
      {"webm", TH_STR_INIT("video/webm")},
      {""}, {""}, {""},
      {"css",  TH_STR_INIT("text/css")},
      {"mpeg", TH_STR_INIT("video/mpeg")},
      {""}, {""}, {""},
      {"aac",  TH_STR_INIT("audio/aac")},
      {""}, {""}, {""}, {""},
      {"ttf",  TH_STR_INIT("font/ttf")},
      {""}, {""}, {""}, {""},
      {"xml",  TH_STR_INIT("application/xml")},
      {""},
      {"xhtml",TH_STR_INIT("application/xhtml+xml")},
      {""}, {""},
      {"txt",  TH_STR_INIT("text/plain")},
      {""}, {""}, {""}, {""},
      {"zip",  TH_STR_INIT("application/zip")},
      {""}, {""}, {""}, {""},
      {"mp4",  TH_STR_INIT("video/mp4")},
      {""}, {""}, {""}, {""},
      {"mp3",  TH_STR_INIT("audio/mpeg")}
    };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

  if (len <= TH_MIME_MAX_WORD_LENGTH && len >= TH_MIME_MIN_WORD_LENGTH)
    {
      register unsigned int key = th_mime_hash (str, len);

      if (key <= TH_MIME_MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return (struct th_mime_mapping *) 0;
}

#pragma GCC diagnostic pop
/* End of src/th_mime.c */
/* Start of src/th_method.c */
/* ANSI-C code produced by gperf version 3.2.1 */
/* Computed positions: -k'1' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif


#include <stddef.h>
#include <string.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wconversion"
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
struct th_method_mapping;

#define TH_METHOD_TOTAL_KEYWORDS 9
#define TH_METHOD_MIN_WORD_LENGTH 3
#define TH_METHOD_MAX_WORD_LENGTH 7
#define TH_METHOD_MIN_HASH_VALUE 3
#define TH_METHOD_MAX_HASH_VALUE 12
/* maximum key range = 10, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
th_method_hash (register const char *str, register size_t len)
{
  static unsigned char asso_values[] =
    {
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13,  5,  0, 13,
      13,  0,  0, 13, 13, 13, 13, 13, 13,  0,
       5, 13, 13, 13,  0, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
      13, 13, 13, 13, 13, 13
    };
  return len + asso_values[(unsigned char)str[0]];
}

struct th_method_mapping *
th_method_mapping_find (register const char *str, register size_t len)
{
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
  static struct th_method_mapping wordlist[] =
    {
      {""}, {""}, {""},
      {"GET",  TH_METHOD_GET},
      {"HEAD", TH_METHOD_HEAD},
      {"TRACE", TH_METHOD_TRACE},
      {"DELETE", TH_METHOD_DELETE},
      {"OPTIONS", TH_METHOD_OPTIONS},
      {"PUT",  TH_METHOD_PUT},
      {"POST", TH_METHOD_POST},
      {"PATCH", TH_METHOD_PATCH},
      {""},
      {"CONNECT", TH_METHOD_CONNECT}
    };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

  if (len <= TH_METHOD_MAX_WORD_LENGTH && len >= TH_METHOD_MIN_WORD_LENGTH)
    {
      register unsigned int key = th_method_hash (str, len);

      if (key <= TH_METHOD_MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return (struct th_method_mapping *) 0;
}

#pragma GCC diagnostic pop
/* End of src/th_method.c */
/* Start of src/th_allocator.c */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct th_default_allocator {
    th_allocator base;
} th_default_allocator;

TH_LOCAL(void*)
th_default_allocator_alloc(void* self, size_t size)
{
    (void)self;
    void* ptr = malloc(size);
    return ptr;
}

TH_LOCAL(void*)
th_default_allocator_realloc(void* self, void* ptr, size_t size)
{
    (void)self;
    return realloc(ptr, size);
}

TH_LOCAL(void)
th_default_allocator_free(void* self, void* ptr)
{
    (void)self;
    free(ptr);
}

static th_default_allocator default_allocator = {
    .base = {
        .alloc = th_default_allocator_alloc,
        .realloc = th_default_allocator_realloc,
        .free = th_default_allocator_free,
    },
};

static th_allocator* user_default_allocator = NULL;

TH_PUBLIC(th_allocator*)
th_default_allocator_get(void)
{
    if (user_default_allocator)
        return user_default_allocator;
    return &default_allocator.base;
}

TH_PUBLIC(void)
th_default_allocator_set(th_allocator* allocator)
{
    user_default_allocator = allocator;
}

/* th_arena_allocator implementation begin */

TH_LOCAL(void*)
th_arena_allocator_alloc(void* self, size_t size)
{
    th_arena_allocator* allocator = self;
    if (allocator->pos + size > allocator->size) {
        if (!allocator->allocator)
            return NULL;
        return th_allocator_alloc(allocator->allocator, size);
    }
    void* ptr = (char*)allocator->buf + allocator->pos;
    allocator->prev_pos = allocator->pos;
    allocator->pos += (size_t)TH_ALIGNAS(allocator->alignment, size);
    return ptr;
}

TH_LOCAL(void*)
th_arena_allocator_realloc(void* self, void* ptr, size_t size)
{
    th_arena_allocator* allocator = self;
    if (ptr == NULL)
        return th_arena_allocator_alloc(self, size);
    if ((char*)ptr < (char*)allocator->buf || (char*)ptr >= (char*)allocator->buf + allocator->size)
        return th_allocator_realloc(allocator->allocator, ptr, size);
    if (ptr == (char*)allocator->buf + allocator->prev_pos) {
        if (allocator->prev_pos + size > allocator->size) {
            if (!allocator->allocator)
                return NULL;
            void* newp = th_allocator_alloc(allocator->allocator, size);
            if (!newp)
                return NULL;
            memcpy(newp, ptr, allocator->pos - allocator->prev_pos);
            allocator->pos = allocator->prev_pos;
            return newp;
        }
        allocator->pos = allocator->prev_pos + size;
        return ptr;
    }
    void* newp = th_allocator_alloc(self, size);
    if (!newp)
        return NULL;
    size_t max_possible = (size_t)(((uint8_t*)allocator->buf + allocator->prev_pos) - (uint8_t*)ptr);
    memcpy(newp, ptr, max_possible);
    return newp;
}

TH_LOCAL(void)
th_arena_allocator_free(void* self, void* ptr)
{
    th_arena_allocator* allocator = self;
    if ((uint8_t*)ptr == (uint8_t*)allocator->buf + allocator->prev_pos) {
        allocator->pos = allocator->prev_pos;
        return;
    }
    if ((char*)ptr < (char*)allocator->buf || (char*)ptr >= (char*)allocator->buf + allocator->size) {
        th_allocator_free(allocator->allocator, ptr);
        return;
    }
}

TH_PRIVATE(void)
th_arena_allocator_init_with_alignment(th_arena_allocator* allocator, void* buf, size_t size, size_t alignment, th_allocator* fallback)
{
    allocator->base.alloc = th_arena_allocator_alloc;
    allocator->base.realloc = th_arena_allocator_realloc;
    allocator->base.free = th_arena_allocator_free;
    allocator->allocator = fallback;
    allocator->alignment = (uint16_t)alignment;
    void* aligned = TH_ALIGNAS(alignment, buf);
    allocator->size = size - (size_t)((uint8_t*)aligned - (uint8_t*)buf);
    allocator->buf = aligned;
    allocator->pos = 0;
    allocator->prev_pos = 0;
}

TH_PRIVATE(void)
th_arena_allocator_init(th_arena_allocator* allocator, void* buf, size_t size, th_allocator* fallback)
{
    th_arena_allocator_init_with_alignment(allocator, buf, size, TH_ALIGNOF(th_max_align), fallback);
}

/* th_arena_allocator implementation end */
/* End of src/th_allocator.c */
/* Start of src/th_task.c */

#include <assert.h>
#include <stdlib.h>

/* th_task functions begin */

TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void*))
{
    TH_ASSERT(task);
    task->fn = fn;
    task->next = NULL;
}

TH_PRIVATE(void)
th_task_complete(th_task* task)
{
    if (task->fn)
        task->fn(task);
}

/* th_task functions end */
/* End of src/th_task.c */
/* Start of src/th_poll.c */

#if !defined(TH_CONFIG_OS_WIN)

#include <errno.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "poll"

/* th_pollops_os begin */

TH_LOCAL(int)
th_pollops_os_poll(void* self, struct pollfd* fds, nfds_t nfds, int timeout_ms)
{
    (void)self;
    return poll(fds, nfds, timeout_ms);
}

TH_PRIVATE(th_pollops*)
th_pollops_os(void)
{
    static th_pollops ops = {
        .poll = th_pollops_os_poll,
    };
    return &ops;
}

/* th_pollops_os end */
/* Forward declarations begin */

typedef struct th_poll_reactor th_poll_reactor;
typedef struct th_poll_handle th_poll_handle;
typedef struct th_poll_handle_map th_poll_handle_map;

/* Forward declarations end */
/* th_poll_fd_to_idx_map begin */

TH_INLINE(uint32_t)
th_poll_fd_hash(int fd)
{
    return (uint32_t)fd;
}

TH_INLINE(bool)
th_poll_fd_eq(int a, int b)
{
    return a == b;
}

TH_DEFINE_HASHMAP(th_poll_fd_to_idx_map, int, size_t, th_poll_fd_hash, th_poll_fd_eq, -1)

/* th_poll_fd_to_idx_map end */
/* th_poll_handle begin */

struct th_poll_handle {
    th_handle base;
    th_timer timer;
    th_poll_handle* next;
    th_poll_handle* prev;
    th_allocator* allocator;
    th_poll_reactor* reactor;
    th_op* pending[TH_OP_MAX];
    int fd;
    bool timeout_enabled;
};

TH_DEFINE_POOL_ALLOCATOR(th_poll_handle_pool, th_poll_handle, prev, next)
TH_DEFINE_VEC(th_pollfd_vec, struct pollfd, (void))

/* th_poll_handle end */
/* th_poll_handle_map begin */

struct th_poll_handle_map {
    th_poll_fd_to_idx_map fd_to_idx_map;
    th_allocator* allocator;
    th_poll_handle** handles;
    size_t size;
    size_t capacity;
};

TH_LOCAL(void)
th_poll_handle_map_init(th_poll_handle_map* map, th_allocator* allocator)
{
    th_poll_fd_to_idx_map_init(&map->fd_to_idx_map, allocator);
    map->allocator = allocator;
    map->handles = NULL;
    map->size = 0;
    map->capacity = 0;
}

TH_LOCAL(void)
th_poll_handle_map_deinit(th_poll_handle_map* map)
{
    th_poll_fd_to_idx_map_deinit(&map->fd_to_idx_map);
    th_allocator_free(map->allocator, map->handles);
}

TH_LOCAL(void)
th_poll_handle_map_set(th_poll_handle_map* map, int fd, th_poll_handle* handle)
{
    size_t idx = 0;
    th_poll_fd_to_idx_map_iter iter = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    if (iter == NULL) {
        if (map->size == map->capacity) {
            size_t new_capacity = (map->capacity == 0) ? 16 : map->capacity * 2;
            th_poll_handle** new_handles = th_allocator_realloc(map->allocator, map->handles, new_capacity * sizeof(th_poll_handle*));
            if (!new_handles) {
                return;
            }
            map->handles = new_handles;
            map->capacity = new_capacity;
        }
        idx = map->size++;
        th_poll_fd_to_idx_map_set(&map->fd_to_idx_map, fd, idx);
    } else {
        idx = iter->value;
    }
    map->handles[idx] = handle;
}

TH_LOCAL(th_poll_handle*)
th_poll_handle_map_try_get(th_poll_handle_map* map, int fd)
{
    th_poll_handle* handle = NULL;
    th_poll_fd_to_idx_map_iter iter = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    if (iter) {
        handle = map->handles[iter->value];
    }
    return handle;
}

TH_LOCAL(void)
th_poll_handle_map_remove(th_poll_handle_map* map, int fd)
{
    th_poll_fd_to_idx_map_iter iter = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    TH_ASSERT(iter && "Must not remove a non-existent handle");
    if (iter) {
        size_t idx = iter->value;
        th_poll_fd_to_idx_map_erase(&map->fd_to_idx_map, iter);
        if (idx != map->size - 1) {
            th_poll_fd_to_idx_map_iter last = th_poll_fd_to_idx_map_find(&map->fd_to_idx_map, map->handles[map->size - 1]->fd);
            last->value = idx;
            map->handles[idx] = map->handles[map->size - 1];
        }
        --map->size;
    }
}

/* th_poll_handle_map implementation end */
/* th_poll_reactor begin */

struct th_poll_reactor {
    th_reactor base;
    th_loop* loop;
    th_allocator* allocator;
    th_clock* clock;
    th_pollops* ops;
    th_poll_handle_pool handle_allocator;
    th_poll_handle_map handles;
    th_pollfd_vec fds;
};

/* th_poll_reactor end */
/* th_poll_handle implementation begin */

TH_LOCAL(th_err)
th_poll_handle_submit(void* self, th_op* op)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    th_poll_reactor* reactor = handle->reactor;
    TH_ASSERT(handle->pending[op->type] == NULL && "Handle already has a pending op for this op type");
    if (th_op_get_flags(op) & TH_OP_IMMEDIATE) {
        th_op_perform(op);
        return TH_ERR_OK;
    }
    handle->pending[op->type] = op;
    struct pollfd pfd = {.fd = handle->fd, .events = (op->type == TH_OP_READ) ? POLLIN : POLLOUT};
    if (handle->timeout_enabled) {
        th_timer_set(&handle->timer, th_seconds(TH_CONFIG_IO_TIMEOUT));
    }
    th_err err = TH_ERR_OK;
    if ((err = th_pollfd_vec_push_back(&reactor->fds, pfd)) != TH_ERR_OK) {
        handle->pending[op->type] = NULL;
        return err;
    }
    th_loop_increase_task_count(reactor->loop);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_poll_handle_cancel(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    for (int i = 0; i < TH_OP_MAX; ++i) {
        th_op* op = handle->pending[i];
        if (op) {
            handle->pending[i] = NULL;
            th_op_abort(op, TH_ERR_SYSTEM(TH_ECANCELED));
            th_loop_decrease_task_count(handle->reactor->loop);
        }
    }
}

TH_LOCAL(int)
th_poll_handle_get_fd(const void* self)
{
    const th_poll_handle* handle = (const th_poll_handle*)self;
    return handle->fd;
}

TH_LOCAL(void)
th_poll_handle_enable_timeout(void* self, bool enable)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    handle->timeout_enabled = enable;
}

TH_LOCAL(void)
th_poll_handle_destroy(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    th_poll_handle_map_remove(&handle->reactor->handles, handle->fd);
    close(handle->fd);
    th_allocator_free(handle->allocator, handle);
}

static const th_handle_methods th_poll_handle_methods = {
    .cancel = th_poll_handle_cancel,
    .submit = th_poll_handle_submit,
    .enable_timeout = th_poll_handle_enable_timeout,
    .get_fd = th_poll_handle_get_fd,
    .destroy = th_poll_handle_destroy,
};

TH_LOCAL(void)
th_poll_handle_init(th_poll_handle* handle, th_poll_reactor* reactor, int fd, th_allocator* allocator)
{
    handle->base.methods = &th_poll_handle_methods;
    th_timer_init(&handle->timer, reactor->clock);
    handle->pending[TH_OP_READ] = NULL;
    handle->pending[TH_OP_WRITE] = NULL;
    handle->allocator = allocator;
    handle->reactor = reactor;
    handle->fd = fd;
    handle->timeout_enabled = false;
}

/* th_poll_handle implementation end */
/* th_poll_reactor implementation begin */

TH_LOCAL(th_err)
th_poll_reactor_create_handle(void* self, th_handle** out, int fd)
{
    th_poll_reactor* reactor = (th_poll_reactor*)self;
    th_poll_handle* handle = th_poll_handle_pool_alloc(&reactor->handle_allocator, sizeof(th_poll_handle));
    if (!handle) {
        return TH_ERR_BAD_ALLOC;
    }
    th_poll_handle_init(handle, reactor, fd, &reactor->handle_allocator.base);
    th_poll_handle_map_set(&reactor->handles, handle->fd, handle);
    *out = (th_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_poll_reactor_run(void* self, int timeout_ms)
{
    th_poll_reactor* reactor = (th_poll_reactor*)self;
    nfds_t nfds = (nfds_t)th_pollfd_vec_size(&reactor->fds);
    int ret = reactor->ops->poll(reactor->ops, th_pollfd_vec_begin(&reactor->fds), nfds, timeout_ms);
    if (ret == -1) {
        TH_LOG_WARN("poll failed: %s", strerror(errno));
        return;
    }

    size_t reenqueue = 0;
    for (size_t i = 0; i < nfds; ++i) {
        struct pollfd* pfd = th_pollfd_vec_at(&reactor->fds, i);
        th_poll_handle* handle = th_poll_handle_map_try_get(&reactor->handles, pfd->fd);
        if (!handle) // handle was removed
            continue;
        short revents = pfd->revents;
        th_op_type type = (pfd->events & POLLIN) ? TH_OP_READ : TH_OP_WRITE;
        th_op* op = handle->pending[type];
        if (revents && op) {
            handle->pending[type] = NULL;
            th_loop_decrease_task_count(reactor->loop);
            if (revents & pfd->events) {
                th_op_perform(op);
            } else if (revents & POLLHUP) {
                th_op_abort(op, TH_ERR_EOF);
            } else if (revents & (POLLERR | POLLPRI)) {
                th_op_abort(op, TH_ERR_SYSTEM(TH_EIO));
            } else if (revents & POLLNVAL) {
                th_op_abort(op, TH_ERR_SYSTEM(TH_EBADF));
            } else {
                TH_LOG_ERROR("Unknown poll event: %d", revents);
                th_op_abort(op, TH_ERR_UNKNOWN);
            }
        } else if (op) { // reenqueue
            if (handle->timeout_enabled && th_timer_expired(&handle->timer)) {
                handle->pending[type] = NULL;
                th_loop_decrease_task_count(reactor->loop);
                th_op_abort(op, TH_ERR_SYSTEM(TH_ETIMEDOUT));
            } else {
                if (reenqueue < i)
                    *th_pollfd_vec_at(&reactor->fds, reenqueue) = *pfd;
                ++reenqueue;
            }
        }
        // handles without a pending op were cancelled, don't reenqueue
    }
    /* th_op_perform above may have synchronously resubmitted an op,
     * pushing a new pollfd past index nfds (the size we polled on).
     * Those entries must survive the compaction below, not just the
     * ones inside [0, nfds). */
    size_t total = th_pollfd_vec_size(&reactor->fds);
    for (size_t i = nfds; i < total; ++i, ++reenqueue) {
        if (reenqueue < i)
            *th_pollfd_vec_at(&reactor->fds, reenqueue) = *th_pollfd_vec_at(&reactor->fds, i);
    }
    th_pollfd_vec_resize(&reactor->fds, reenqueue);
}

TH_LOCAL(void)
th_poll_reactor_deinit(th_poll_reactor* reactor)
{
    th_poll_handle_map_deinit(&reactor->handles);
    th_poll_handle_pool_deinit(&reactor->handle_allocator);
    th_pollfd_vec_deinit(&reactor->fds);
}

TH_LOCAL(void)
th_poll_reactor_destroy(void* self)
{
    th_poll_reactor* reactor = (th_poll_reactor*)self;
    th_allocator* allocator = reactor->allocator;
    th_poll_reactor_deinit(reactor);
    th_allocator_free(allocator, reactor);
}

static const th_reactor_methods th_poll_reactor_methods = {
    .run = th_poll_reactor_run,
    .create_handle = th_poll_reactor_create_handle,
    .destroy = th_poll_reactor_destroy,
};

TH_LOCAL(void)
th_poll_reactor_init(th_poll_reactor* reactor, th_loop* loop, th_allocator* allocator, th_clock* clock, th_pollops* ops)
{
    reactor->base.methods = &th_poll_reactor_methods;
    reactor->loop = loop;
    reactor->allocator = allocator;
    reactor->clock = clock;
    reactor->ops = ops;
    th_pollfd_vec_init(&reactor->fds, allocator);
    th_poll_handle_map_init(&reactor->handles, allocator);
    th_poll_handle_pool_init(&reactor->handle_allocator, allocator, 16, 8 * 1024);
}

TH_PRIVATE(th_err)
th_poll_create(th_reactor** out, th_loop* loop, th_allocator* allocator, th_clock* clock, th_pollops* ops)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_poll_reactor* reactor = th_allocator_alloc(allocator, sizeof(th_poll_reactor));
    if (!reactor) {
        return TH_ERR_BAD_ALLOC;
    }
    th_poll_reactor_init(reactor, loop, allocator, clock, ops);
    *out = &reactor->base;
    return TH_ERR_OK;
}

/* th_poll_reactor implementation end */

#endif /* !TH_CONFIG_OS_WIN */
/* End of src/th_poll.c */
/* Start of src/th_loop.c */

TH_PRIVATE(void)
th_loop_init(th_loop* loop, th_reactor* reactor)
{
    loop->reactor = reactor;
    loop->queue = th_task_queue_make();
    loop->num_tasks = 0;
    th_task_init(&loop->reactor_task, NULL);
    th_task_queue_push(&loop->queue, &loop->reactor_task);
}

TH_PRIVATE(void)
th_loop_push_task(th_loop* loop, th_task* task)
{
    ++loop->num_tasks;
    th_task_queue_push(&loop->queue, task);
}

TH_PRIVATE(void)
th_loop_push_uncounted_task(th_loop* loop, th_task* task)
{
    th_task_queue_push(&loop->queue, task);
}

TH_PRIVATE(void)
th_loop_increase_task_count(th_loop* loop)
{
    ++loop->num_tasks;
}

TH_PRIVATE(void)
th_loop_decrease_task_count(th_loop* loop)
{
    --loop->num_tasks;
}

TH_PRIVATE(th_err)
th_loop_poll(th_loop* loop, int timeout_ms)
{
    if (loop->num_tasks == 0) {
        return TH_ERR_EOF;
    }
    while (1) {
        th_task* task = th_task_queue_pop(&loop->queue);
        TH_ASSERT(task && "Task queue must never be empty");
        bool empty = th_task_queue_empty(&loop->queue);
        if (task == &loop->reactor_task) {
            th_reactor_run(loop->reactor, empty ? timeout_ms : 0);
            th_task_queue_push(&loop->queue, &loop->reactor_task);
            if (empty)
                return TH_ERR_OK;
        } else {
            th_task_complete(task);
            --loop->num_tasks;
            return TH_ERR_OK;
        }
    }
}

TH_PRIVATE(void)
th_loop_run(th_loop* loop)
{
    while (th_loop_poll(loop, 0) == TH_ERR_OK) {
    }
}

TH_PRIVATE(void)
th_loop_deinit(th_loop* loop)
{
    while (th_task_queue_pop(&loop->queue)) {
    }
}
/* End of src/th_loop.c */
/* Start of src/th_error.c */
#include <string.h>


TH_PUBLIC(const char*)
th_strerror(th_err err)
{
    switch (TH_ERR_CATEGORY(err)) {
    case TH_ERR_CATEGORY_OTHER:
        switch (TH_ERR_CODE(err)) {
        case 0:
            return "success";
        case TH_ERRC_BAD_ALLOC:
            return "out of memory";
        case TH_ERRC_INVALID_ARG:
            return "invalid argument";
        case TH_ERRC_EOF:
            return "end of file";
        default:
            return "unknown error";
        }
        break;
    case TH_ERR_CATEGORY_SYSTEM:
        return strerror(TH_ERR_CODE(err));
    case TH_ERR_CATEGORY_HTTP:
        return th_http_strerror(TH_ERR_CODE(err));
    case TH_ERR_CATEGORY_SSL:
#if TH_WITH_SSL
        return th_ssl_strerror(TH_ERR_CODE(err));
#else
        TH_ASSERT(0 && "SSL not enabled");
        return NULL;
#endif
    default:
        break;
    }
    return "Unknown error category";
}
/* End of src/th_error.c */
/* Start of src/th_socket.c */

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <winsock2.h>
#endif

#if defined(TH_CONFIG_OS_OSX)
#include <limits.h>
#endif

#if defined(TH_CONFIG_OS_POSIX)

TH_LOCAL(th_err)
th_socket_ops_os_send(void* self, int fd, const void* addr, size_t len, size_t* result)
{
    (void)self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    ssize_t ret = send(fd, addr, len, flags);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_socket_ops_os_sendvec(void* self, int fd, const th_iov* iov, size_t iovcnt, size_t* result)
{
    (void)self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = (struct iovec*)iov;
#if defined(TH_CONFIG_OS_OSX)
    TH_ASSERT(iovcnt <= INT_MAX);
    msg.msg_iovlen = (int)iovcnt;
#else
    msg.msg_iovlen = iovcnt;
#endif
    ssize_t ret = sendmsg(fd, &msg, flags);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_socket_ops_os_recv(void* self, int fd, void* addr, size_t len, size_t* result)
{
    (void)self;
    ssize_t ret = recv(fd, addr, len, 0);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    if (ret == 0)
        return TH_ERR_EOF;
    *result = (size_t)ret;
    return TH_ERR_OK;
}

/* Builds header iov + one trailing iov (extra) into vec, capped at
 * TH_SOCKET_SENDFILE_MAX_IOV entries; returns the combined iovec count. */
#define TH_SOCKET_SENDFILE_MAX_IOV 64

TH_LOCAL(size_t)
th_socket_build_sendfile_iov(struct iovec* vec, const th_iov* iov, size_t iovcnt, void* extra_base, size_t extra_len)
{
    size_t veclen = 0;
    for (size_t i = 0; i < iovcnt && veclen < TH_SOCKET_SENDFILE_MAX_IOV - 1; ++i, ++veclen) {
        vec[veclen].iov_base = iov[i].base;
        vec[veclen].iov_len = iov[i].len;
    }
    vec[veclen].iov_base = extra_base;
    vec[veclen].iov_len = extra_len;
    ++veclen;
    return veclen;
}

#define TH_SOCKET_SENDFILE_BUFFERED_MAX (8 * 1024)

/* Read a chunk of the file into a stack buffer, then send header +
 * buffer in one sendmsg. The chunk is capped at
 * TH_SOCKET_SENDFILE_BUFFERED_MAX regardless of len - th_sendfile_op
 * drives further chunks via its own retry loop. */
TH_LOCAL(th_err)
th_socket_ops_os_sendfile(void* self, int fd, const th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, size_t* result)
{
    (void)self;
    uint8_t buffer[TH_SOCKET_SENDFILE_BUFFERED_MAX];
    size_t toread = TH_MIN(sizeof(buffer), len);
    ssize_t readlen = pread(file->fd, buffer, toread, (off_t)offset);
    if (readlen < 0)
        return TH_ERR_SYSTEM(errno);

    struct iovec vec[TH_SOCKET_SENDFILE_MAX_IOV];
    size_t veclen = th_socket_build_sendfile_iov(vec, iov, iovcnt, buffer, (size_t)readlen);

    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = vec;
#if defined(TH_CONFIG_OS_OSX)
    TH_ASSERT(veclen <= INT_MAX);
    msg.msg_iovlen = (int)veclen;
#else
    msg.msg_iovlen = veclen;
#endif
    ssize_t ret = sendmsg(fd, &msg, flags);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return TH_ERR_OK;
}

TH_PRIVATE(th_socket_ops*)
th_socket_ops_os(void)
{
    static th_socket_ops ops = {
        .send = th_socket_ops_os_send,
        .sendvec = th_socket_ops_os_sendvec,
        .recv = th_socket_ops_os_recv,
        .sendfile = th_socket_ops_os_sendfile,
    };
    return &ops;
}

#endif /* TH_CONFIG_OS_POSIX */

TH_PRIVATE(void)
th_socket_init(th_socket* socket, th_loop* loop, th_socket_ops* ops)
{
    socket->loop = loop;
    socket->handle = NULL;
    socket->ops = ops;
}

TH_PRIVATE(th_err)
th_socket_set_fd(th_socket* socket, int fd)
{
    th_socket_close(socket);
    th_err err = th_reactor_create_handle(socket->loop->reactor, &socket->handle, fd);
    if (err != TH_ERR_OK)
        return err;
    th_handle_enable_timeout(socket->handle, true);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_close(th_socket* socket)
{
    if (socket->handle) {
        th_handle_destroy(socket->handle);
        socket->handle = NULL;
    }
}

TH_PRIVATE(void)
th_socket_deinit(th_socket* socket)
{
    th_socket_close(socket);
}
/* End of src/th_socket.c */
/* Start of src/th_recv.c */

TH_LOCAL(bool)
th_recv_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_recv_op_finalize(th_recv_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_recv_op_complete(th_recv_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_recv_op_perform(th_recv_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t result = 0;
    th_err err = th_socket_recv(op->socket, (char*)op->addr + op->pos, op->len - op->pos, &result);
    if (err != TH_ERR_OK)
        return err;
    op->pos += result;
    if (!op->exact || op->pos == op->len)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_recv_op_fn(void* self)
{
    th_recv_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_recv_op_finalize(op);
        return;
    }
    th_err err = th_recv_op_perform(op);
    if (th_recv_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_recv_op_complete(op, err);
}

TH_LOCAL(void)
th_recv_op_abort(void* self, th_err err)
{
    th_recv_op_complete(self, err);
}

TH_PRIVATE(void)
th_recv_op_init(th_recv_op* op, th_socket* socket, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_READ, th_recv_op_fn, th_recv_op_abort);
    op->socket = socket;
    op->addr = addr;
    op->len = len;
    op->pos = 0;
    op->exact = exact;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
/* End of src/th_recv.c */
/* Start of src/th_send.c */

TH_LOCAL(bool)
th_send_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_send_op_finalize(th_send_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_send_op_complete(th_send_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_send_op_perform(th_send_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t result = 0;
    th_err err = th_socket_send(op->socket, (const char*)op->addr + op->pos, op->len - op->pos, &result);
    if (err != TH_ERR_OK)
        return err;
    op->pos += result;
    if (op->pos == op->len)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_send_op_fn(void* self)
{
    th_send_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_send_op_finalize(op);
        return;
    }
    th_err err = th_send_op_perform(op);
    if (th_send_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_send_op_complete(op, err);
}

TH_LOCAL(void)
th_send_op_abort(void* self, th_err err)
{
    th_send_op_complete(self, err);
}

TH_PRIVATE(void)
th_send_op_init(th_send_op* op, th_socket* socket, const void* addr, size_t len, th_send_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_WRITE, th_send_op_fn, th_send_op_abort);
    op->socket = socket;
    op->addr = addr;
    op->len = len;
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
/* End of src/th_send.c */
/* Start of src/th_sendvec.c */

TH_LOCAL(bool)
th_sendvec_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_sendvec_op_finalize(th_sendvec_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_sendvec_op_complete(th_sendvec_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_sendvec_op_perform(th_sendvec_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t result = 0;
    th_err err = th_socket_sendvec(op->socket, op->iov, op->iovcnt, &result);
    if (err != TH_ERR_OK)
        return err;
    op->pos += result;
    th_iov_consume(&op->iov, &op->iovcnt, result);
    if (op->iovcnt == 0)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_sendvec_op_fn(void* self)
{
    th_sendvec_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_sendvec_op_finalize(op);
        return;
    }
    th_err err = th_sendvec_op_perform(op);
    if (th_sendvec_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_sendvec_op_complete(op, err);
}

TH_LOCAL(void)
th_sendvec_op_abort(void* self, th_err err)
{
    th_sendvec_op_complete(self, err);
}

TH_PRIVATE(void)
th_sendvec_op_init(th_sendvec_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_send_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_WRITE, th_sendvec_op_fn, th_sendvec_op_abort);
    op->socket = socket;
    op->iov = iov;
    op->iovcnt = iovcnt;
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
/* End of src/th_sendvec.c */
/* Start of src/th_sendfile.c */

TH_LOCAL(bool)
th_sendfile_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_sendfile_op_finalize(th_sendfile_op* op)
{
    op->callback(op->user_data, op->pos, op->err);
}

TH_LOCAL(void)
th_sendfile_op_complete(th_sendfile_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

TH_LOCAL(th_err)
th_sendfile_op_perform(th_sendfile_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    size_t file_pos = op->pos > op->header_len ? op->pos - op->header_len : 0;
    size_t remaining = op->len - file_pos;
    size_t chunk = TH_MIN(remaining, TH_CONFIG_SENDFILE_CHUNK_LEN);

    size_t result = 0;
    th_err err = th_socket_sendfile(op->socket, op->iov, op->iovcnt, op->file, op->offset + file_pos, chunk, &result);
    if (err != TH_ERR_OK)
        return err;

    op->pos += result;
    th_iov_consume(&op->iov, &op->iovcnt, result);
    if (op->pos == op->header_len + op->len)
        return TH_ERR_OK;
    return TH_ERR_SYSTEM(TH_EAGAIN);
}

TH_LOCAL(void)
th_sendfile_op_fn(void* self)
{
    th_sendfile_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_sendfile_op_finalize(op);
        return;
    }
    th_err err = th_sendfile_op_perform(op);
    if (th_sendfile_op_is_retryable(err)) {
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_sendfile_op_complete(op, err);
}

TH_LOCAL(void)
th_sendfile_op_abort(void* self, th_err err)
{
    th_sendfile_op_complete(self, err);
}

TH_PRIVATE(void)
th_sendfile_op_init(th_sendfile_op* op, th_socket* socket, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_WRITE, th_sendfile_op_fn, th_sendfile_op_abort);
    op->socket = socket;
    op->iov = iov;
    op->iovcnt = iovcnt;
    op->file = file;
    op->offset = offset;
    op->len = len;
    op->header_len = th_iov_bytes(iov, iovcnt);
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
/* End of src/th_sendfile.c */
/* Start of src/th_acceptor.c */


#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

TH_LOCAL(th_err)
th_acceptor_ops_os_set_nonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_open(void* self, const char* addr, const char* port, int* out_fd)
{
    (void)self;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* res = NULL;
    if (getaddrinfo(addr, port, &hints, &res) != 0)
        return TH_ERR_SYSTEM(errno);

    th_err err = TH_ERR_OK;
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_addrinfo;
    }
#if TH_CONFIG_REUSE_ADDR
    {
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            err = TH_ERR_SYSTEM(errno);
            goto cleanup_fd;
        }
    }
#endif
#if TH_CONFIG_REUSE_PORT
    {
#if defined(SO_REUSEPORT)
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
            err = TH_ERR_SYSTEM(errno);
            goto cleanup_fd;
        }
#else
        TH_LOG_FATAL("SO_REUSEPORT is not supported on this platform");
        err = TH_ERR_NOSUPPORT;
        goto cleanup_fd;
#endif
    }
#endif
    if ((err = th_acceptor_ops_os_set_nonblocking(fd)) != TH_ERR_OK)
        goto cleanup_fd;
    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if (listen(fd, 1024) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    freeaddrinfo(res);
    *out_fd = fd;
    return TH_ERR_OK;
cleanup_fd:
    close(fd);
cleanup_addrinfo:
    freeaddrinfo(res);
    return err;
}

TH_LOCAL(th_err)
th_acceptor_ops_os_accept(void* self, int fd, th_address* addr, int* out_fd)
{
    (void)self;
    int conn_fd = accept(fd, (struct sockaddr*)&addr->addr, &addr->addrlen);
    if (conn_fd < 0)
        return TH_ERR_SYSTEM(errno);
    th_err err = th_acceptor_ops_os_set_nonblocking(conn_fd);
    if (err != TH_ERR_OK) {
        close(conn_fd);
        return err;
    }
    *out_fd = conn_fd;
    return TH_ERR_OK;
}

TH_PRIVATE(th_acceptor_ops*)
th_acceptor_ops_os(void)
{
    static th_acceptor_ops ops = {
        .open = th_acceptor_ops_os_open,
        .accept = th_acceptor_ops_os_accept,
    };
    return &ops;
}

#endif /* TH_CONFIG_OS_POSIX */

TH_PRIVATE(void)
th_acceptor_init(th_acceptor* acceptor, th_loop* loop, th_acceptor_ops* ops)
{
    acceptor->loop = loop;
    acceptor->handle = NULL;
    acceptor->ops = ops;
}

TH_PRIVATE(th_err)
th_acceptor_open(th_acceptor* acceptor, const char* addr, const char* port)
{
    int fd = -1;
    th_err err = acceptor->ops->open(acceptor->ops, addr, port, &fd);
    if (err != TH_ERR_OK)
        return err;
    th_acceptor_close(acceptor);
    err = th_reactor_create_handle(acceptor->loop->reactor, &acceptor->handle, fd);
    if (err != TH_ERR_OK) {
#if defined(TH_CONFIG_OS_POSIX)
        close(fd);
#endif
        return err;
    }
    th_handle_enable_timeout(acceptor->handle, false);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_acceptor_close(th_acceptor* acceptor)
{
    if (acceptor->handle) {
        th_handle_destroy(acceptor->handle);
        acceptor->handle = NULL;
    }
}

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor)
{
    th_acceptor_close(acceptor);
}

TH_PRIVATE(th_err)
th_acceptor_accept(th_acceptor* acceptor, th_address* addr, th_socket* out_socket)
{
    int fd = -1;
    th_err err = acceptor->ops->accept(acceptor->ops, th_acceptor_get_fd(acceptor), addr, &fd);
    if (err != TH_ERR_OK)
        return err;
    return th_socket_set_fd(out_socket, fd);
}
/* End of src/th_acceptor.c */
/* Start of src/th_accept.c */

TH_LOCAL(bool)
th_accept_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_accept_op_finalize(th_accept_op* op)
{
    op->callback(op->user_data, op->err);
}

TH_LOCAL(void)
th_accept_op_complete(th_accept_op* op, th_err err)
{
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_acceptor_post(op->acceptor, &op->base.base);
}

TH_LOCAL(th_err)
th_accept_op_perform(th_accept_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    th_address_init(op->addr);
    return th_acceptor_accept(op->acceptor, op->addr, op->socket);
}

TH_LOCAL(void)
th_accept_op_fn(void* self)
{
    th_accept_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_accept_op_finalize(op);
        return;
    }
    th_err err = th_accept_op_perform(op);
    if (th_accept_op_is_retryable(err)) {
        err = th_acceptor_submit(op->acceptor, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_accept_op_complete(op, err);
}

TH_LOCAL(void)
th_accept_op_abort(void* self, th_err err)
{
    th_accept_op_complete(self, err);
}

TH_PRIVATE(void)
th_accept_op_init(th_accept_op* op, th_acceptor* acceptor, th_address* addr,
                  th_socket* socket, th_accept_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_READ, th_accept_op_fn, th_accept_op_abort);
    op->acceptor = acceptor;
    op->addr = addr;
    op->socket = socket;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}
/* End of src/th_accept.c */
/* Start of src/th_tcp_conn.c */


#undef TH_LOG_TAG
#define TH_LOG_TAG "tcp_conn"

/** th_tcp_conn_op
 * @brief At most one recv and one send are ever in flight at a time on
 * an HTTP connection (request read, then response write), so a single
 * union covers every th_conn_methods.recv/send call without allocating.
 */
typedef union th_tcp_conn_op {
    th_recv_op recv;
    th_sendvec_op sendvec;
    th_sendfile_op sendfile;
} th_tcp_conn_op;

typedef struct th_tcp_conn {
    th_conn_observable base;
    th_socket socket;
    th_address addr;
    th_tcp_conn_op recv_op;
    th_tcp_conn_op send_op;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
} th_tcp_conn;

TH_LOCAL(th_address*)
th_tcp_conn_get_address(void* self)
{
    th_tcp_conn* conn = self;
    return &conn->addr;
}

TH_LOCAL(th_socket*)
th_tcp_conn_get_socket(void* self)
{
    th_tcp_conn* conn = self;
    return &conn->socket;
}

TH_LOCAL(void)
th_tcp_conn_start(void* self)
{
    th_tcp_conn* conn = self;
    TH_LOG_TRACE("%p: Starting", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_tcp_conn_recv(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    th_tcp_conn* conn = self;
    th_recv_op_init(&conn->recv_op.recv, &conn->socket, addr, len, exact, callback, user_data);
    th_op_perform(&conn->recv_op.recv.base);
}

TH_LOCAL(void)
th_tcp_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_tcp_conn* conn = self;
    if (file) {
        th_sendfile_op_init(&conn->send_op.sendfile, &conn->socket, iov, iovcnt, file, offset, len, callback, user_data);
        th_op_perform(&conn->send_op.sendfile.base);
    } else {
        th_sendvec_op_init(&conn->send_op.sendvec, &conn->socket, iov, iovcnt, callback, user_data);
        th_op_perform(&conn->send_op.sendvec.base);
    }
}

TH_LOCAL(void)
th_tcp_conn_cancel(void* self)
{
    th_tcp_conn* conn = self;
    th_socket_cancel(&conn->socket);
}

TH_LOCAL(void)
th_tcp_conn_free(void* self)
{
    th_tcp_conn* conn = self;
    TH_LOG_TRACE("%p: Destroying connection", conn);
    th_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

static const th_conn_methods th_tcp_conn_methods = {
    .get_address = th_tcp_conn_get_address,
    .get_socket = th_tcp_conn_get_socket,
    .start = th_tcp_conn_start,
    .recv = th_tcp_conn_recv,
    .send = th_tcp_conn_send,
    .cancel = th_tcp_conn_cancel,
    .destroy = th_conn_observable_destroy,
};

TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_socket* socket,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_tcp_conn* conn = th_allocator_alloc(allocator, sizeof(th_tcp_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_conn_observable_init(&conn->base, &th_tcp_conn_methods, th_tcp_conn_free, observer);
    conn->upgrader = upgrader;
    conn->allocator = allocator;
    conn->socket = *socket;
    th_address_init(&conn->addr);
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}
/* End of src/th_tcp_conn.c */
/* Start of src/th_request_parser.c */


#undef TH_LOG_TAG
#define TH_LOG_TAG "request_parser"

TH_PRIVATE(void)
th_request_parser_init(th_request_parser* parser)
{
    parser->state = TH_REQUEST_PARSER_STATE_METHOD;
    parser->content_len = 0;
    parser->body_encoding = TH_REQUEST_BODY_ENCODING_NONE;
}

TH_PRIVATE(void)
th_request_parser_reset(th_request_parser* parser)
{
    parser->state = TH_REQUEST_PARSER_STATE_METHOD;
    parser->content_len = 0;
    parser->body_encoding = TH_REQUEST_BODY_ENCODING_NONE;
}

TH_PRIVATE(size_t)
th_request_parser_content_len(th_request_parser* parser)
{
    return parser->content_len;
}

TH_LOCAL(th_err)
th_request_parser_do_cookie_list(th_request* request, th_str cookie_list)
{
    th_cookie_parser parser;
    th_cookie_parser_init(&parser, cookie_list);
    while (!th_cookie_parser_done(&parser)) {
        th_str key, value;
        th_err err = th_cookie_parser_next(&parser, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if ((err = th_request_add_cookie(request, key, value)) != TH_ERR_OK) {
            return err;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_next_queryvar(th_str string, size_t* pos, th_str* key, th_str* value)
{
    size_t eq = th_str_find_first(string, *pos, '=');
    if (eq == th_str_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    *key = th_str_trim(th_str_substr(string, *pos, eq - *pos));
    *pos = th_str_find_first(string, eq + 1, '&');
    if (*pos != th_str_npos) {
        *value = th_str_trim(th_str_substr(string, eq + 1, *pos - eq - 1));
        (*pos)++;
        return TH_ERR_OK;
    } else {
        *value = th_str_trim(th_str_substr(string, eq + 1, *pos));
        return TH_ERR_OK;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_bodyvars(th_request* request, th_str body)
{
    th_err err = TH_ERR_OK;
    size_t pos = 0;
    while (pos != th_str_npos) {
        th_str key;
        th_str value;
        err = th_request_parser_do_next_queryvar(body, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if ((err = th_request_add_formvar(request, key, value)) != TH_ERR_OK) {
            return err;
        }
    }
    return err;
}

/* Get the next HTTP token from the buffer, stopping at the given character */
TH_LOCAL(th_err)
th_request_parser_next_token(th_str buffer, th_str* token, char until, size_t* parsed)
{
    static const int token_char[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 32-47 (don't allow space, ")
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 48-57 (0-9)
        1, 1, 0, 1, 0, 1, 1,                            // 58-64 (don't allow <,>)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 65-80 (A-P)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 81-90 (Q-Z)
        0, 0, 0, 1, 1, 1,                               // 91-96 (don't allow [, \, ])
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 97-112 (a-p)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 113-122 (q-z)
        0, 1, 0, 1, 0,                                  // 123-127 (don't allow {, }, DEL)
        // implicitely set to 0 for 128-255
    };
    size_t i = 0;
    while (i < buffer.len && buffer.ptr[i] != until) {
        if (token_char[(unsigned char)buffer.ptr[i]] == 0)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        i++;
    }
    if (i == buffer.len)
        return TH_ERR_OK;
    if (i == 0)
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *token = th_str_substr(buffer, 0, i);
    *parsed = i + 1;
    return TH_ERR_OK;
}

TH_LOCAL(bool)
th_request_parser_is_printable_string(th_str input)
{
    for (size_t i = 0; i < input.len; i++) {
        if (input.ptr[i] < 32 || input.ptr[i] > 126) {
            return false;
        }
    }
    return true;
}

TH_LOCAL(th_err)
th_request_parser_do_method(th_request_parser* parser, th_request* request, th_str buffer, size_t* parsed_out)
{
    th_str method;
    size_t parsed = 0;
    th_err err = th_request_parser_next_token(buffer, &method, ' ', &parsed);
    if (err != TH_ERR_OK || parsed == 0) {
        return err;
    }
    struct th_method_mapping* mm = th_method_mapping_find(method.ptr, method.len);
    if (!mm) {
        return TH_ERR_HTTP(TH_CODE_NOT_IMPLEMENTED);
    }
    th_request_set_method(request, mm->method);
    *parsed_out = parsed;
    parser->state = TH_REQUEST_PARSER_STATE_PATH;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_uri_query(th_request* request, th_str path)
{
    size_t pos = 0;
    while (pos != th_str_npos) {
        th_str key;
        th_str value;
        th_err err = th_request_parser_do_next_queryvar(path, &pos, &key, &value);
        if (err != TH_ERR_OK) {
            return err;
        }
        if (th_request_add_queryvar(request, key, value) != TH_ERR_OK) {
            return TH_ERR_BAD_ALLOC;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_next_path_segment(th_str buffer, th_str* segment, size_t* parsed)
{
    static const int uri_char[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0-15
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 16-31
        0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 32-47 (don't allow space, ")
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 48-57 (0-9)
        1, 1, 0, 1, 0, 1, 1,                            // 58-64 (don't allow <,>)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 65-80 (A-P)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 81-90 (Q-Z)
        0, 0, 0, 0, 1, 0,                               // 91-96 (don't allow [, \, ], ^, `)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 97-112 (a-p)
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                   // 113-122 (q-z)
        0, 0, 0, 1, 0,                                  // 123-127 (don't allow {, |, }, DEL)
        // implicitely set to 0 for 128-255
    };
    size_t i = 0;
    while (i < buffer.len && buffer.ptr[i] != ' ' && buffer.ptr[i] != '?') {
        if (uri_char[(unsigned char)buffer.ptr[i]] == 0)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        i++;
    }
    if (i == buffer.len)
        return TH_ERR_OK;
    *segment = th_str_substr(buffer, 0, i);
    *parsed = i + 1;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_path(th_request_parser* parser, th_request* request, th_str path, size_t* parsed)
{
    th_str segment;
    size_t uri_parsed = 0;
    th_err err = th_request_parser_next_path_segment(path, &segment, &uri_parsed);
    if (err != TH_ERR_OK || uri_parsed == 0)
        return err;
    if ((err = th_request_set_uri_path(request, segment)) != TH_ERR_OK)
        return err;
    if (segment.ptr[segment.len] == '?') { // got a query
        size_t query_parsed = 0;
        err = th_request_parser_next_path_segment(th_str_substr(path, uri_parsed, th_str_npos), &segment, &query_parsed);
        if (err != TH_ERR_OK || query_parsed == 0)
            return err;
        if ((err = th_request_set_uri_query(request, segment)) != TH_ERR_OK)
            return err;
        if ((err = th_request_parser_do_uri_query(request, segment)) != TH_ERR_OK) {
            // If we can't parse the query, that's ok, we just ignore it
            // restore the original state and continue
            th_request_clear_queryvars(request);
        }
        uri_parsed += query_parsed;
    } else {
        if ((err = th_request_set_uri_query(request, TH_STR(""))) != TH_ERR_OK)
            return err;
    }
    *parsed = uri_parsed;
    parser->state = TH_REQUEST_PARSER_STATE_VERSION;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_version(th_request_parser* parser, th_request* request, th_str buffer, size_t* parsed)
{
    size_t n = th_str_find_first(buffer, 0, '\r');
    if (n == th_str_npos || n + 1 == buffer.len)
        return TH_ERR_OK;
    if (buffer.ptr[n + 1] != '\n')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_str version = th_str_substr(buffer, 0, n);
    if (version.len != 8)
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[0] != 'H')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[1] != 'T')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[2] != 'T')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[3] != 'P')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[4] != '/')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[5] != '1')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[6] != '.')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (version.ptr[7] < '0' || version.ptr[7] > '9')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_request_set_version(request, version.ptr[7] - '0');
    *parsed = n + 2;
    parser->state = TH_REQUEST_PARSER_STATE_HEADERS;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parse_handle_header(th_request_parser* parser, th_request* request, th_str name, th_str value)
{
    char arena[1024] = {0};
    th_arena_allocator arena_allocator;
    th_arena_allocator_init(&arena_allocator, arena, sizeof(arena), NULL);
    th_string normalized_name;
    th_string_init(&normalized_name, &arena_allocator.base);
    if (th_string_set(&normalized_name, name) != TH_ERR_OK) {
        // This can only happen if the name is too long
        return TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    th_string_to_lower(&normalized_name);
    th_header_id id = th_header_id_from_string(th_string_data(&normalized_name), th_string_len(&normalized_name));
    switch (id) {
    case TH_HEADER_ID_COOKIE:
        return th_request_parser_do_cookie_list(request, value);
    case TH_HEADER_ID_CONTENT_LENGTH: {
        unsigned int content_len = 0;
        th_err err = th_str_to_uint(value, &content_len);
        parser->content_len = content_len;
        return err;
    }
    case TH_HEADER_ID_CONNECTION:
        if (th_str_eq(value, TH_STR("close"))) {
            request->close = true;
        } else if (th_str_eq(value, TH_STR("keep-alive"))) {
            request->close = false;
        }
        return TH_ERR_OK;
    case TH_HEADER_ID_CONTENT_TYPE:
        if (th_str_eq(value, TH_STR("application/x-www-form-urlencoded"))) {
            parser->body_encoding = TH_REQUEST_BODY_ENCODING_FORM_URL_ENCODED;
        } else if (th_str_eq(th_str_substr(value, 0, 19), TH_STR("multipart/form-data"))) {
            parser->body_encoding = TH_REQUEST_BODY_ENCODING_MULTIPART_FORM_DATA;
        }
        break;
    default:
        break;
    }
    return th_request_add_header(request, th_string_view(&normalized_name), value);
}

TH_LOCAL(th_err)
th_request_parser_do_header(th_request_parser* parser, th_request* request, th_str buffer, size_t* parsed)
{
    size_t n = th_str_find_first(buffer, 0, '\r');
    if (n == th_str_npos || n + 1 == buffer.len)
        return TH_ERR_OK;
    if (buffer.ptr[n + 1] != '\n')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (n == 0) {
        *parsed = 2;
        if (parser->content_len == 0) {
            th_request_set_body(request, th_str_make(&buffer.ptr[2], 0));
            parser->state = TH_REQUEST_PARSER_STATE_DONE;
        } else {
            if (request->method == TH_METHOD_GET || request->method == TH_METHOD_HEAD)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            parser->state = TH_REQUEST_PARSER_STATE_BODY;
        }
        return TH_ERR_OK;
    }
    size_t key_parsed = 0;
    th_str key;
    th_err err = TH_ERR_OK;
    if ((err = th_request_parser_next_token(buffer, &key, ':', &key_parsed)) != TH_ERR_OK
        || key_parsed == 0)
        return err;
    th_str value = th_str_substr(buffer, key_parsed, n - key_parsed);
    if (!th_request_parser_is_printable_string(value))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if ((err = th_request_parse_handle_header(parser, request, th_str_trim(key), th_str_trim(value)))
        != TH_ERR_OK)
        return err;
    *parsed = n + 2;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_multipart_form_data(th_request* request, th_str body)
{
    th_str content_type = th_request_get_header(request, TH_STR("content-type"));
    if (th_str_empty(content_type)) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    th_err err = TH_ERR_OK;
    th_str boundary = th_str_make_empty();
    if ((err = th_multipart_parser_boundary(content_type, &boundary)) != TH_ERR_OK)
        return err;

    th_multipart_parser parser;
    if ((err = th_multipart_parser_init(&parser, body, boundary)) != TH_ERR_OK)
        return err;
    while (!th_multipart_parser_done(&parser)) {
        th_multipart_part part;
        if ((err = th_multipart_parser_next(&parser, &part)) != TH_ERR_OK)
            return err;
        if (th_request_add_part(request, part.content, part.name, part.filename, part.content_type) != TH_ERR_OK)
            return TH_ERR_BAD_ALLOC;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_body(th_request_parser* parser, th_request* request, th_str buffer, size_t* parsed)
{
    if (buffer.len < parser->content_len) {
        *parsed = 0;
        return TH_ERR_OK;
    }
    // Got the whole body
    th_str body = th_str_substr(buffer, 0, parser->content_len);
    if (parser->body_encoding == TH_REQUEST_BODY_ENCODING_FORM_URL_ENCODED) {
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_do_bodyvars(request, body)) != TH_ERR_OK)
            return err;
    } else if (parser->body_encoding == TH_REQUEST_BODY_ENCODING_MULTIPART_FORM_DATA) {
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_do_multipart_form_data(request, body)) != TH_ERR_OK)
            return err;
    }
    th_request_set_body(request, body);
    *parsed = parser->content_len;
    parser->state = TH_REQUEST_PARSER_STATE_DONE;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_parse_next(th_request_parser* parser, th_request* request, th_str data, size_t* parsed)
{
    switch (parser->state) {
    case TH_REQUEST_PARSER_STATE_METHOD:
        return th_request_parser_do_method(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_PATH:
        return th_request_parser_do_path(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_VERSION:
        return th_request_parser_do_version(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_HEADERS:
        return th_request_parser_do_header(parser, request, data, parsed);
    case TH_REQUEST_PARSER_STATE_BODY:
        return th_request_parser_do_body(parser, request, data, parsed);
    default:
        *parsed = 0;
        break;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_request_parser_parse(th_request_parser* parser, th_request* request, th_str data, size_t* parsed)
{
    th_err err = TH_ERR_OK;
    while (data.len > 0) {
        size_t p = 0;
        if ((err = th_request_parser_parse_next(parser, request, th_str_substr(data, p, data.len), &p)) != TH_ERR_OK) {
            *parsed = p;
            return err;
        }
        data.ptr += p;
        data.len -= p;
        *parsed += p;
        if (p == 0 || parser->state == TH_REQUEST_PARSER_STATE_DONE) {
            return TH_ERR_OK;
        }
    }
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_request_parser_header_done(th_request_parser* parser)
{
    return parser->state > TH_REQUEST_PARSER_STATE_HEADERS;
}

TH_PRIVATE(bool)
th_request_parser_done(th_request_parser* parser)
{
    return parser->state == TH_REQUEST_PARSER_STATE_DONE;
}
/* End of src/th_request_parser.c */
/* Start of src/th_cookie_parser.c */

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
/* End of src/th_cookie_parser.c */
/* Start of src/th_multipart_parser.c */


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
/* End of src/th_multipart_parser.c */
/* Start of src/th_part.c */

TH_PRIVATE(void)
th_part_init(th_part* part, th_str content, th_allocator* allocator)
{
    th_string_init(&part->name, allocator);
    th_string_init(&part->filename, allocator);
    th_string_init(&part->content_type, allocator);
    part->content = content;
}

TH_PRIVATE(void)
th_part_deinit(th_part* part)
{
    th_string_deinit(&part->name);
    th_string_deinit(&part->filename);
    th_string_deinit(&part->content_type);
}

TH_PRIVATE(th_err)
th_part_set_name(th_part* part, th_str name)
{
    return th_string_set(&part->name, name);
}

TH_PRIVATE(th_err)
th_part_set_filename(th_part* part, th_str filename)
{
    return th_string_set(&part->filename, filename);
}

TH_PRIVATE(th_err)
th_part_set_content_type(th_part* part, th_str content_type)
{
    return th_string_set(&part->content_type, content_type);
}

// Public API

TH_PUBLIC(const char*)
th_part_name(const th_part* part)
{
    return th_string_data(&part->name);
}

TH_PUBLIC(const char*)
th_part_filename(const th_part* part)
{
    return th_string_data(&part->filename);
}

TH_PUBLIC(const char*)
th_part_content_type(const th_part* part)
{
    return th_string_data(&part->content_type);
}

TH_PUBLIC(th_buffer)
th_part_content(const th_part* part)
{
    return (th_buffer){part->content.ptr, part->content.len};
}
/* End of src/th_part.c */
/* Start of src/th_request.c */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "request"

/* hstr iterator begin */

TH_INLINE(bool)
th_hstr_iter_next(th_iter* it)
{
    it->ptr = ((const th_hstr_pair*)it->ptr) + 1;
    return it->ptr < it->end;
}

TH_INLINE(const char*)
th_hstr_iter_key(const th_iter* it)
{
    return th_string_data(&((const th_hstr_pair*)it->ptr)->key);
}

TH_INLINE(const void*)
th_hstr_iter_val(const th_iter* it)
{
    return th_string_data(&((const th_hstr_pair*)it->ptr)->value);
}

static th_iter_methods th_hstr_iter_methods = {
    .next = th_hstr_iter_next,
    .key = th_hstr_iter_key,
    .val = th_hstr_iter_val,
};

// hstr iterator end
// part iterator begin

TH_INLINE(bool)
th_part_iter_next(th_iter* it)
{
    it->ptr = ((const th_part*)it->ptr) + 1;
    return it->ptr < it->end;
}

TH_INLINE(const char*)
th_part_iter_key(const th_iter* it)
{
    return th_string_data(&((const th_part*)it->ptr)->name);
}

TH_INLINE(const void*)
th_part_iter_val(const th_iter* it)
{
    return it->ptr;
}

static th_iter_methods th_part_iter_methods = {
    .next = th_part_iter_next,
    .key = th_part_iter_key,
    .val = th_part_iter_val,
};

// part iterator end

TH_LOCAL(th_err)
th_request_map_store(th_request* request, th_hstr_vec* vec, th_str key, th_str value)
{
    th_err err = TH_ERR_OK;
    th_string k;
    th_string v;
    if ((err = th_string_init_with(&k, key, request->allocator)) != TH_ERR_OK)
        return err;
    if ((err = th_string_init_with(&v, value, request->allocator)) != TH_ERR_OK)
        goto cleanup_key;
    if ((err = th_hstr_vec_push_back(vec, (th_hstr_pair){k, v})) != TH_ERR_OK)
        goto cleanup_value;
    return TH_ERR_OK;
cleanup_value:
    th_string_deinit(&v);
cleanup_key:
    th_string_deinit(&k);
    return err;
}

TH_LOCAL(th_err)
th_request_map_store_url_decoded(th_request* request, th_hstr_vec* vec, th_str key, th_str value, th_url_decode_type type)
{
    th_err err = TH_ERR_OK;
    th_string k;
    th_string v;
    th_string_init(&k, request->allocator);
    th_string_init(&v, request->allocator);
    if ((err = th_url_decode_string(key, &k, type)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_url_decode_string(value, &v, type)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_hstr_vec_push_back(vec, (th_hstr_pair){k, v})) != TH_ERR_OK)
        goto cleanup;
    return TH_ERR_OK;
cleanup:
    th_string_deinit(&v);
    th_string_deinit(&k);
    return err;
}

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_str key, th_str value)
{
    return th_request_map_store(request, &request->cookies, key, value);
}

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_str key, th_str value)
{
    return th_request_map_store(request, &request->headers, key, value);
}

TH_PRIVATE(th_err)
th_request_add_part(th_request* request, th_str content, th_str name, th_str filename, th_str content_type)
{
    th_part part;
    th_part_init(&part, content, request->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_part_set_name(&part, name)) != TH_ERR_OK)
        goto cleanup_part;
    if ((err = th_part_set_filename(&part, filename)) != TH_ERR_OK)
        goto cleanup_part;
    if ((err = th_part_set_content_type(&part, content_type)) != TH_ERR_OK)
        goto cleanup_part;
    if ((err = th_part_vec_push_back(&request->parts, part)) != TH_ERR_OK)
        goto cleanup_part;
    return TH_ERR_OK;
cleanup_part:
    th_part_deinit(&part);
    return err;
}

TH_PRIVATE(th_err)
th_request_add_queryvar(th_request* request, th_str key, th_str value)
{
    return th_request_map_store_url_decoded(request, &request->queryvars, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_str key, th_str value)
{
    return th_request_map_store_url_decoded(request, &request->formvars, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_str key, th_str value)
{
    return th_request_map_store(request, &request->pathvars, key, value);
}

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_str path)
{
    return th_string_set(&request->uri_path, path);
}

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_str query)
{
    return th_string_set(&request->uri_query, query);
}

TH_PRIVATE(void)
th_request_set_version(th_request* request, int version)
{
    request->version = version;
}

TH_PRIVATE(void)
th_request_set_method(th_request* request, th_method method)
{
    request->method = method;
}

TH_PRIVATE(void)
th_request_clear_queryvars(th_request* request)
{
    th_hstr_vec_clear(&request->queryvars);
    // TODO: clear heap strings
}

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_str body)
{
    request->body = body;
}

TH_PRIVATE(void)
th_request_init(th_request* request, th_allocator* allocator)
{
    request->allocator = allocator ? allocator : th_default_allocator_get();
    th_string_init(&request->uri_path, request->allocator);
    th_string_init(&request->uri_query, request->allocator);
    th_part_vec_init(&request->parts, request->allocator);
    th_hstr_vec_init(&request->cookies, request->allocator);
    th_hstr_vec_init(&request->headers, request->allocator);
    th_hstr_vec_init(&request->queryvars, request->allocator);
    th_hstr_vec_init(&request->formvars, request->allocator);
    th_hstr_vec_init(&request->pathvars, request->allocator);
    request->body = (th_str){0};
    request->version = 0;
    request->close = false;
}

TH_PRIVATE(void)
th_request_deinit(th_request* request)
{
    th_string_deinit(&request->uri_path);
    th_string_deinit(&request->uri_query);
    th_part_vec_deinit(&request->parts);
    th_hstr_vec_deinit(&request->cookies);
    th_hstr_vec_deinit(&request->headers);
    th_hstr_vec_deinit(&request->queryvars);
    th_hstr_vec_deinit(&request->formvars);
    th_hstr_vec_deinit(&request->pathvars);
}

TH_PRIVATE(void)
th_request_reset(th_request* request)
{
    th_string_clear(&request->uri_path);
    th_string_clear(&request->uri_query);
    th_part_vec_clear(&request->parts);
    th_hstr_vec_clear(&request->cookies);
    th_hstr_vec_clear(&request->headers);
    th_hstr_vec_clear(&request->queryvars);
    th_hstr_vec_clear(&request->formvars);
    th_hstr_vec_clear(&request->pathvars);
    request->body = (th_str){0};
    request->version = 0;
    request->close = false;
}

TH_LOCAL(th_str)
th_request_vec_get(th_hstr_vec* vec, th_str key)
{
    size_t num = th_hstr_vec_size(vec);
    for (size_t i = 0; i < num; i++) {
        if (th_string_eq(&vec->data[i].key, key))
            return th_string_view(&vec->data[i].value);
    }
    return TH_STR("");
}

TH_PRIVATE(th_str)
th_request_get_header(th_request* request, th_str key)
{
    return th_request_vec_get(&request->headers, key);
}

TH_PRIVATE(th_str)
th_request_get_pathvar(th_request* request, th_str key)
{
    return th_request_vec_get(&request->pathvars, key);
}

TH_PRIVATE(th_str)
th_request_get_queryvar(th_request* request, th_str key)
{
    return th_request_vec_get(&request->queryvars, key);
}

TH_PRIVATE(th_str)
th_request_get_formvar(th_request* request, th_str key)
{
    return th_request_vec_get(&request->formvars, key);
}

TH_PRIVATE(th_part*)
th_request_get_part(th_request* request, th_str key)
{
    size_t num = th_part_vec_size(&request->parts);
    for (size_t i = 0; i < num; i++) {
        if (th_string_eq(&request->parts.data[i].name, key))
            return th_part_vec_at(&request->parts, i);
    }
    return NULL;
}

/* Public iterator API begin */

TH_PUBLIC(bool)
th_next(th_iter* it)
{
    return it->methods->next(it);
}

TH_PUBLIC(const char*)
th_key(const th_iter* it)
{
    return it->methods->key(it);
}

TH_PUBLIC(const void*)
th_val(const th_iter* it)
{
    return it->methods->val(it);
}

TH_PUBLIC(const char*)
th_cval(const th_iter* it)
{
    return (const char*)it->methods->val(it);
}

/* Public iterator API end */
/* Public request API begin */

TH_PUBLIC(const char*)
th_get_path(const th_request* req)
{
    return th_string_data(&req->uri_path);
}

TH_PUBLIC(const char*)
th_get_query(const th_request* req)
{
    return th_string_data(&req->uri_query);
}

TH_PUBLIC(th_buffer)
th_get_body(const th_request* req)
{
    return (th_buffer){req->body.ptr, req->body.len};
}

TH_PUBLIC(th_method)
th_get_method(const th_request* req)
{
    return req->method;
}

TH_PUBLIC(th_prot_version)
th_get_version(const th_request* req)
{
    return (th_prot_version)req->version;
}

TH_PUBLIC(const char*)
th_find_header(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->headers);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_string_data(&req->headers.data[i].key), th_string_len(&req->headers.data[i].key)) == 0) {
            return th_string_data(&req->headers.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_header_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->headers.data,
        .end = req->headers.data + req->headers.size,
    };
}

TH_PUBLIC(const char*)
th_find_cookie(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->cookies);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_string_data(&req->cookies.data[i].key), th_string_len(&req->cookies.data[i].key)) == 0) {
            return th_string_data(&req->cookies.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_cookie_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->cookies.data,
        .end = req->cookies.data + req->cookies.size,
    };
}

TH_PUBLIC(const char*)
th_find_queryvar(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->queryvars);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_string_data(&req->queryvars.data[i].key), th_string_len(&req->queryvars.data[i].key)) == 0) {
            return th_string_data(&req->queryvars.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_queryvar_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->queryvars.data,
        .end = req->queryvars.data + req->queryvars.size,
    };
}

TH_PUBLIC(const char*)
th_find_formvar(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->formvars);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_string_data(&req->formvars.data[i].key), th_string_len(&req->formvars.data[i].key)) == 0) {
            return th_string_data(&req->formvars.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_formvar_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->formvars.data,
        .end = req->formvars.data + req->formvars.size,
    };
}

TH_PUBLIC(const char*)
th_find_pathvar(const th_request* req, const char* key)
{
    size_t num = th_hstr_vec_size(&req->pathvars);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(key, th_string_data(&req->pathvars.data[i].key), th_string_len(&req->pathvars.data[i].key)) == 0) {
            return th_string_data(&req->pathvars.data[i].value);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_pathvar_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_hstr_iter_methods,
        .ptr = req->pathvars.data,
        .end = req->pathvars.data + req->pathvars.size,
    };
}

TH_PUBLIC(const th_part*)
th_find_part(const th_request* req, const char* name)
{
    size_t num = th_part_vec_size(&req->parts);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(name, th_string_data(&req->parts.data[i].name), th_string_len(&req->parts.data[i].name))
            == 0) {
            return th_part_vec_cat(&req->parts, i);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_part_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_part_iter_methods,
        .ptr = req->parts.data,
        .end = req->parts.data + req->parts.size,
    };
}

/* Public request API end */
/* End of src/th_request.c */
/* Start of src/th_response.c */


#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "response"

/* th_response implementation begin */

TH_PRIVATE(void)
th_response_init(th_response* response, th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_string_init(&response->headers, allocator);
    th_string_init(&response->body, allocator);
    response->iov[0] = (th_iov){0};
    response->iov[1] = (th_iov){0};
    response->iov[2] = (th_iov){0};
    response->allocator = allocator;
    response->dir_mgr = dir_mgr;
    response->fcache = fcache;
    response->fcache_entry = NULL;
    response->file_len = 0;
    response->code = TH_CODE_OK;
    memset(response->header_is_set, 0, sizeof(response->header_is_set));
    response->is_file = false;
    response->only_headers = false;
}

TH_PRIVATE(void)
th_response_deinit(th_response* response)
{
    th_string_deinit(&response->headers);
    th_string_deinit(&response->body);
    if (response->fcache_entry) {
        th_fcache_entry_unref(response->fcache_entry);
        response->fcache_entry = NULL;
    }
}

TH_PRIVATE(void)
th_response_reset(th_response* response)
{
    th_string_clear(&response->headers);
    th_string_clear(&response->body);
    response->iov[0] = (th_iov){0};
    response->iov[1] = (th_iov){0};
    response->iov[2] = (th_iov){0};
    if (response->fcache_entry) {
        th_fcache_entry_unref(response->fcache_entry);
        response->fcache_entry = NULL;
    }
    response->file_len = 0;
    response->code = TH_CODE_OK;
    memset(response->header_is_set, 0, sizeof(response->header_is_set));
    response->is_file = false;
    response->only_headers = false;
}

TH_PRIVATE(void)
th_response_set_code(th_response* response, th_code code)
{
    response->code = code;
}

TH_PUBLIC(th_err)
th_response_add_header(th_response* response, th_str key, th_str value)
{
    th_header_id header_id = th_header_id_from_string(key.ptr, key.len);
    if (header_id != TH_HEADER_ID_UNKNOWN && response->header_is_set[header_id]) {
        return TH_ERR_INVALID_ARG;
    }
    th_err err = TH_ERR_OK;
    size_t old_len = th_string_len(&response->headers);
    if ((err = th_string_append(&response->headers, key)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_string_append(&response->headers, TH_STR(": "))) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_string_append(&response->headers, value)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_string_append(&response->headers, TH_STR("\r\n"))) != TH_ERR_OK)
        goto cleanup;
    if (header_id != TH_HEADER_ID_UNKNOWN) {
        response->header_is_set[header_id] = 1;
    }
    return TH_ERR_OK;
cleanup:
    th_string_resize(&response->headers, old_len, '\0');
    return err;
}

TH_LOCAL(th_str)
th_response_get_mime_type(th_str filename)
{
    char ext[256];
    size_t ei = 0;
    size_t max = filename.len < sizeof(ext) ? filename.len : sizeof(ext);
    for (size_t i = 0; i < max; ++i) {
        size_t ri = filename.len - i - 1;
        ei = max - i - 1;
        ext[ei] = filename.ptr[ri];
        if (filename.ptr[ri] == '.' || filename.ptr[ri] == '/') {
            break;
        }
    }
    struct th_mime_mapping* mm = NULL;
    if (ext[ei] == '.') {
        mm = th_mime_mapping_find(&ext[ei + 1], max - ei - 1);
        return mm ? mm->mime : TH_STR("application/octet-stream");
    } else {
        return TH_STR("application/octet-stream");
    }
}

TH_LOCAL(th_err)
th_response_set_body_from_file(th_response* response, th_str root, th_str path)
{
    th_dir* dir = th_dir_mgr_get(response->dir_mgr, root);
    if (!dir)
        return TH_ERR_INVALID_ARG;
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_get(response->fcache, dir, path, &response->fcache_entry)) != TH_ERR_OK) {
        return err;
    }
    // Set the content type, if not already set
    if (response->header_is_set[TH_HEADER_ID_CONTENT_TYPE] == 0) {
        th_str mime_type = th_response_get_mime_type(path);
        if ((err = th_response_add_header(response, TH_STR("Content-Type"), mime_type)) != TH_ERR_OK)
            goto cleanup_fcache_entry;
    }
    response->is_file = 1;
    return TH_ERR_OK;
cleanup_fcache_entry:
    th_fcache_entry_unref(response->fcache_entry);
    response->fcache_entry = NULL;
    return err;
}

TH_PRIVATE(th_err)
th_response_set_body(th_response* response, th_str body)
{
    th_err err = TH_ERR_OK;
    if ((err = th_string_set(&response->body, body)) != TH_ERR_OK)
        return err;
    response->is_file = 0;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
TH_PRINTF_FMT(2, 0)
th_response_set_body_va(th_response* response, const char* fmt, va_list args)
{
    char buffer[512];
    th_err err = TH_ERR_OK;
    va_list va;
    va_copy(va, args);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);
    if (len < 0) {
        return TH_ERR_INVALID_ARG;
    } else if ((size_t)len < sizeof(buffer)) {
        if ((err = th_string_set(&response->body, th_str_make(buffer, (size_t)len))) != TH_ERR_OK) {
            return err;
        }
    } else {
        th_string_resize(&response->body, (size_t)len, ' ');
        vsnprintf(th_string_at(&response->body, 0), (size_t)len, fmt, args);
    }
    response->is_file = 0;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_finalize_headers(th_response* response)
{
    th_err err = TH_ERR_OK;
    if ((err = th_string_append(&response->headers, TH_STR("\r\n"))) != TH_ERR_OK)
        return err;
    size_t headers_len = th_string_len(&response->headers);

    // Set the start line
    char int_buffer[128]; // Buffer for the integer to string conversion
    if ((err = th_string_append(&response->headers, TH_STR("HTTP/1.1 "))) != TH_ERR_OK)
        return err;
    if ((err = th_string_append_cstr(&response->headers, th_fmt_uint_to_str(int_buffer, sizeof(int_buffer), response->code))) != TH_ERR_OK)
        return err;
    if ((err = th_string_append(&response->headers, TH_STR(" "))) != TH_ERR_OK)
        return err;
    if ((err = th_string_append_cstr(&response->headers, th_http_strerror((int)response->code))) != TH_ERR_OK)
        return err;
    if ((err = th_string_append(&response->headers, TH_STR("\r\n"))) != TH_ERR_OK)
        return err;
    response->iov[0].base = th_string_at(&response->headers, headers_len);
    response->iov[0].len = th_string_len(&response->headers) - headers_len;
    response->iov[1].base = th_string_at(&response->headers, 0);
    response->iov[1].len = headers_len;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_set_default_headers(th_response* response)
{
    th_err err = TH_ERR_OK;
    char buffer[256];
    if (response->is_file) {
        size_t len = 0;
        const char* content_len = th_fmt_uint_to_str_ex(buffer, sizeof(buffer), (unsigned int)response->file_len, &len);
        if ((err = th_response_add_header(response, TH_STR("Content-Length"), th_str_make(content_len, len))) != TH_ERR_OK)
            return err;
    } else {
        size_t len = 0;
        const char* body_len = th_fmt_uint_to_str_ex(buffer, sizeof(buffer), (unsigned int)th_string_len(&response->body), &len);
        if ((err = th_response_add_header(response, TH_STR("Content-Length"), th_str_make(body_len, len))) != TH_ERR_OK)
            return err;
    }
    if (!response->header_is_set[TH_HEADER_ID_SERVER]) {
        if ((err = th_response_add_header(response, TH_STR("Server"), TH_STR("TinyHTTP"))) != TH_ERR_OK)
            return err;
    }
    if (!response->header_is_set[TH_HEADER_ID_DATE]) {
        th_date now = th_date_now();
        char date[64];
        size_t len = th_fmt_strtime(date, sizeof(date), now);
        if ((err = th_response_add_header(response, TH_STR("Date"), th_str_make(date, len))) != TH_ERR_OK)
            return err;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_response_async_write(th_response* response, th_conn* conn, th_send_cb callback, void* user_data)
{
    th_err err = TH_ERR_OK;
    size_t iovcnt = 2; // start line + headers
    if (response->is_file) {
        response->file_len = response->fcache_entry->stream.size;
    }
    if ((err = th_response_set_default_headers(response)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_response_finalize_headers(response)) != TH_ERR_OK)
        goto cleanup;
    if (!response->only_headers && response->is_file == 0 && th_string_len(&response->body) > 0) {
        response->iov[iovcnt].base = (void*)th_string_data(&response->body);
        response->iov[iovcnt].len = th_string_len(&response->body);
        iovcnt++;
    }
    if (!response->only_headers && response->is_file != 0) {
        th_conn_send(conn, response->iov, iovcnt, &response->fcache_entry->stream, 0, (size_t)response->file_len, callback, user_data);
    } else {
        th_conn_send(conn, response->iov, iovcnt, NULL, 0, 0, callback, user_data);
    }
    return;
cleanup:
    // Header formatting failed before any I/O was attempted (out of
    // memory); safe to call back synchronously since no op is pending.
    callback(user_data, 0, err);
}

/* Public response API begin */

TH_PUBLIC(th_err)
th_set_body(th_response* response, const char* body)
{
    return th_response_set_body(response, th_str_from_cstr(body));
}

TH_PUBLIC(th_err)
TH_PRINTF_FMT(2, 3)
th_printf_body(th_response* resp, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    th_err err = th_response_set_body_va(resp, fmt, args);
    va_end(args);
    return err;
}

TH_PUBLIC(th_err)
th_set_body_from_file(th_response* response, const char* root, const char* filepath)
{
    (void)root;
    return th_response_set_body_from_file(response, th_str_from_cstr(root), th_str_from_cstr(filepath));
}

TH_PUBLIC(th_err)
th_add_header(th_response* response, const char* key, const char* value)
{
    return th_response_add_header(response, th_str_from_cstr(key), th_str_from_cstr(value));
}

TH_PUBLIC(th_err)
th_add_cookie(th_response* response, const char* key, const char* value, th_cookie_attr* attr)
{
    char buffer[512];
    size_t len = 0;
    len += th_fmt_str_append(buffer, len, sizeof(buffer), key);
    len += th_fmt_str_append(buffer, len, sizeof(buffer), "=");
    len += th_fmt_str_append(buffer, len, sizeof(buffer), value);
    if (attr) {
        th_date empty_date = {0};
        if (memcmp(&attr->expires, &empty_date, sizeof(th_date)) != 0) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Expires=");
            len += th_fmt_strtime(buffer + len, sizeof(buffer) - len, attr->expires);
        }
        if (attr->max_age.seconds) {
            char max_age[32];
            const char* max_age_str = th_fmt_uint_to_str(max_age, sizeof(max_age), (unsigned int)attr->max_age.seconds);
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Max-Age=");
            len += th_fmt_str_append(buffer, len, sizeof(buffer), max_age_str);
        }
        if (attr->domain) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Domain=");
            len += th_fmt_str_append(buffer, len, sizeof(buffer), attr->domain);
        }
        if (attr->path) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Path=");
            len += th_fmt_str_append(buffer, len, sizeof(buffer), attr->path);
        }
        if (attr->secure) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; Secure");
        }
        if (attr->http_only) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; HttpOnly");
        }
        if (attr->same_site) {
            len += th_fmt_str_append(buffer, len, sizeof(buffer), "; SameSite=");
            switch (attr->same_site) {
            case TH_COOKIE_SAME_SITE_NONE:
                if (attr->secure) {
                    len += th_fmt_str_append(buffer, len, sizeof(buffer), "None");
                } else {
                    return TH_ERR_INVALID_ARG;
                }
                break;
            case TH_COOKIE_SAME_SITE_LAX:
                len += th_fmt_str_append(buffer, len, sizeof(buffer), "Lax");
                break;
            case TH_COOKIE_SAME_SITE_STRICT:
                len += th_fmt_str_append(buffer, len, sizeof(buffer), "Strict");
                break;
            default:
                return TH_ERR_INVALID_ARG;
                break;
            }
        }
    }
    return th_response_add_header(response, TH_STR("Set-Cookie"), th_str_make(buffer, len));
}
/* End of src/th_response.c */
/* Start of src/th_conn.c */

/* th_conn_observable begin */

TH_PRIVATE(void)
th_conn_observable_destroy(void* self)
{
    th_conn_observable* observable = self;
    th_conn_observer_on_deinit(observable->observer, observable);
    observable->destroy(observable);
}

TH_PRIVATE(void)
th_conn_observable_init(th_conn_observable* observable, const th_conn_methods* methods,
                        void (*destroy)(void* self), th_conn_observer* observer)
{
    /* methods->destroy must already be th_conn_observable_destroy: the
     * concrete conn type's static methods table points destroy there
     * so th_conn_destroy always notifies the observer first, then this
     * calls the type's real destructor (the destroy param below). */
    observable->base.methods = methods;
    th_conn_observer_on_init(observer, observable);
    observable->destroy = destroy;
    observable->observer = observer;
}

/* th_conn_observable end */
/* End of src/th_conn.c */
/* Start of src/th_header_id.c */
/* ANSI-C code produced by gperf version 3.2.1 */
/* Computed positions: -k'' */


#include <stddef.h>
#include <string.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wconversion"
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif
struct th_header_id_mapping;

#define TH_HEADER_ID_TOTAL_KEYWORDS 6
#define TH_HEADER_ID_MIN_WORD_LENGTH 5
#define TH_HEADER_ID_MAX_WORD_LENGTH 17
#define TH_HEADER_ID_MIN_HASH_VALUE 5
#define TH_HEADER_ID_MAX_HASH_VALUE 17
/* maximum key range = 13, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
/*ARGSUSED*/
static unsigned int
th_header_id_hash (register const char *str, register size_t len)
{
  (void) str;
  return len;
}

struct th_header_id_mapping *
th_header_id_mapping_find (register const char *str, register size_t len)
{
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
  static struct th_header_id_mapping wordlist[] =
    {
      {""}, {""}, {""}, {""}, {""},
      {"range", TH_HEADER_ID_RANGE},
      {"cookie", TH_HEADER_ID_COOKIE},
      {""}, {""}, {""},
      {"connection", TH_HEADER_ID_CONNECTION},
      {""},
      {"content-type", TH_HEADER_ID_CONTENT_TYPE},
      {""},
      {"content-length", TH_HEADER_ID_CONTENT_LENGTH},
      {""}, {""},
      {"transfer-encoding", TH_HEADER_ID_TRANSFER_ENCODING}
    };
#if (defined __GNUC__ && __GNUC__ + (__GNUC_MINOR__ >= 6) > 4) || (defined __clang__ && __clang_major__ >= 3)
#pragma GCC diagnostic pop
#endif

  if (len <= TH_HEADER_ID_MAX_WORD_LENGTH && len >= TH_HEADER_ID_MIN_WORD_LENGTH)
    {
      register unsigned int key = th_header_id_hash (str, len);

      if (key <= TH_HEADER_ID_MAX_HASH_VALUE)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return (struct th_header_id_mapping *) 0;
}

#pragma GCC diagnostic pop
/* End of src/th_header_id.c */
/* Start of src/th_filepath.c */

#include <string.h>

TH_PRIVATE(th_err)
th_filepath_init(th_filepath* path, th_str str)
{
    if (str.len == 0 || str.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    if (str.ptr[0] == '/' || str.ptr[str.len - 1] == '/')
        return TH_ERR_INVALID_ARG;
    if (th_str_find_first(str, 0, '\0') != th_str_npos)
        return TH_ERR_INVALID_ARG;
    size_t out = 0;
    size_t start = 0;
    while (start < str.len) {
        size_t sep = th_str_find_first(str, start, '/');
        size_t end = sep == th_str_npos ? str.len : sep;
        size_t len = end - start;
        if (len == 2 && str.ptr[start] == '.' && str.ptr[start + 1] == '.')
            return TH_ERR_INVALID_ARG;
        bool is_dot = len == 1 && str.ptr[start] == '.';
        if (len > 0 && !is_dot) {
            if (out > 0)
                path->buf[out++] = '/';
            memcpy(path->buf + out, str.ptr + start, len);
            out += len;
        }
        start = end + 1;
    }
    if (out == 0)
        return TH_ERR_INVALID_ARG;
    path->buf[out] = '\0';
    return TH_ERR_OK;
}
/* End of src/th_filepath.c */
/* Start of src/th_file.c */

#include <stdio.h>

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#undef TH_LOG_TAG
#define TH_LOG_TAG "file"

/* th_file_ops implementation begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(th_err)
th_file_ops_os_openat(void* self, int dirfd, const char* path, int flags, int* fd)
{
    (void)self;
    int ret = openat(dirfd, path, flags, 0644);
    if (ret == -1)
        return TH_ERR_SYSTEM(errno);
    *fd = ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_file_ops_os_seek(void* self, int fd, int whence, size_t* pos)
{
    (void)self;
    off_t ret = lseek(fd, 0, whence);
    if (ret == -1)
        return TH_ERR_SYSTEM(errno);
    *pos = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_file_ops_os_read(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read)
{
    (void)self;
    off_t ret = pread(fd, addr, len, (off_t)offset);
    if (ret == -1) {
        *read = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *read = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_file_ops_os_write(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written)
{
    (void)self;
    off_t ret = pwrite(fd, addr, len, (off_t)offset);
    if (ret == -1) {
        *written = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *written = (size_t)ret;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_file_ops_os_stat(void* self, int fd, struct stat* out)
{
    (void)self;
    if (fstat(fd, out) == -1)
        return TH_ERR_SYSTEM(errno);
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_file_ops_os_close(void* self, int fd)
{
    (void)self;
    close(fd);
}

TH_PRIVATE(th_file_ops*)
th_file_ops_os(void)
{
    static th_file_ops ops = {
        .openat = th_file_ops_os_openat,
        .seek = th_file_ops_os_seek,
        .read = th_file_ops_os_read,
        .write = th_file_ops_os_write,
        .stat = th_file_ops_os_stat,
        .close = th_file_ops_os_close,
    };
    return &ops;
}
#endif

/* th_file_ops implementation end */
/* th_file implementation begin */

TH_PRIVATE(void)
th_file_init(th_file* stream, th_file_ops* ops)
{
    stream->ops = ops;
    stream->fd = -1;
}

TH_LOCAL(int)
th_open_opt_to_flags(th_open_opt opt)
{
    int flags = O_NOFOLLOW;
    if (opt.read && opt.write)
        flags |= O_RDWR;
    else if (opt.read)
        flags |= O_RDONLY;
    else if (opt.write)
        flags |= O_WRONLY;
    if (opt.create)
        flags |= O_CREAT;
    if (opt.truncate)
        flags |= O_TRUNC;
    return flags;
}

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, const th_filepath* path, th_open_opt opt)
{
    int fd = -1;
    th_err err = stream->ops->openat(stream->ops, dir->fd, th_filepath_cstr(path), th_open_opt_to_flags(opt), &fd);
    if (err != TH_ERR_OK)
        return err;
    size_t size = 0;
    size_t unused = 0;
    if ((err = stream->ops->seek(stream->ops, fd, SEEK_END, &size)) != TH_ERR_OK)
        goto cleanup;
    if ((err = stream->ops->seek(stream->ops, fd, SEEK_SET, &unused)) != TH_ERR_OK)
        goto cleanup;
    stream->fd = fd;
    stream->size = size;
    return TH_ERR_OK;
cleanup:
    stream->ops->close(stream->ops, fd);
    return err;
}

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read)
{
    return stream->ops->read(stream->ops, stream->fd, addr, len, offset, read);
}

TH_PRIVATE(th_err)
th_file_write(th_file* stream, const void* addr, size_t len, size_t offset, size_t* written)
{
    return stream->ops->write(stream->ops, stream->fd, addr, len, offset, written);
}

/**
 * We use DJB2 hash function, without multiplication,
 * as it's faster and good enough for our purposes.
 */
#define FSTAT_HASH_INIT 5381
#define FSTAT_HASH_NEXT(hash, val) ((hash << 5) + hash + val)

TH_PRIVATE(uint32_t)
th_file_stat_hash(th_file* stream)
{
    struct stat st = {0};
    th_err err = stream->ops->stat(stream->ops, stream->fd, &st);
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("stat failed: %s, can't calculate hash", th_strerror(err));
        TH_ASSERT(0 && "stat failed");
        return 0;
    }
#if defined(TH_CONFIG_OS_OSX)
    int64_t mtime_sec = st.st_mtimespec.tv_sec;
    int64_t mtime_nsec = st.st_mtimespec.tv_nsec;
#else
    int64_t mtime_sec = st.st_mtime;
    int64_t mtime_nsec = 0;
#endif
    uint32_t hash = FSTAT_HASH_INIT;
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)mtime_sec);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)mtime_nsec);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_size);
    hash = FSTAT_HASH_NEXT(hash, st.st_mode);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_ino);
    hash = FSTAT_HASH_NEXT(hash, st.st_uid);
    hash = FSTAT_HASH_NEXT(hash, st.st_gid);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)(st.st_nlink != 0));
    return hash;
}
#undef FSTAT_HASH_INIT
#undef FSTAT_HASH_NEXT

TH_PRIVATE(void)
th_file_close(th_file* stream)
{
    if (stream->fd != -1)
        stream->ops->close(stream->ops, stream->fd);
    stream->fd = -1;
}

TH_PRIVATE(void)
th_file_deinit(th_file* stream)
{
    th_file_close(stream);
}
/* End of src/th_file.c */
/* Start of src/th_fcache.c */

#undef TH_LOG_TAG
#define TH_LOG_TAG "fcache"

TH_LOCAL(th_fcache_id)
th_fcache_entry_id(th_fcache_entry* entry)
{
    return (th_fcache_id){th_string_view(&entry->path), entry->dir};
}

TH_LOCAL(void)
th_fcache_entry_actual_destroy(void* self)
{
    th_fcache_entry* entry = self;
    // Remove entry from cache
    th_fcache_map_iter it = th_fcache_map_find(&entry->cache->map, th_fcache_entry_id(entry));
    if (it != NULL) {
        th_fcache_map_erase(&entry->cache->map, it);
    }
    th_file_deinit(&entry->stream);
    th_string_deinit(&entry->path);
    th_allocator_free(entry->allocator, entry);
}

TH_LOCAL(void)
th_fcache_entry_init(th_fcache_entry* entry, th_fcache* cache, th_allocator* allocator)
{
    entry->allocator = allocator ? allocator : th_default_allocator_get();
    th_refcounted_init(&entry->base, th_fcache_entry_actual_destroy);
    th_file_init(&entry->stream, cache->file_ops);
    th_string_init(&entry->path, entry->allocator);
    entry->cache = cache;
    entry->next = NULL;
    entry->prev = NULL;
}

TH_LOCAL(th_err)
th_fcache_entry_open(th_fcache_entry* entry, th_dir* dir, th_str path)
{
    th_err err = TH_ERR_OK;
    th_filepath filepath;
    th_open_opt opt = {.read = true};
    if ((err = th_filepath_init(&filepath, path)) != TH_ERR_OK) {
        TH_LOG_INFO("Invalid file path %.*s: %s", (int)path.len, path.ptr, th_strerror(err));
        goto cleanup;
    }
    if ((err = th_file_openat(&entry->stream, dir, &filepath, opt)) != TH_ERR_OK) {
        TH_LOG_INFO("Failed to open file at %.*s: %s", (int)path.len, path.ptr, th_strerror(err));
        goto cleanup;
    }
    if ((err = th_string_set(&entry->path, path)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to set path: %s", th_strerror(err));
        goto cleanup_fstream;
    }
    entry->stat_hash = th_file_stat_hash(&entry->stream);
    entry->dir = dir;
    return TH_ERR_OK;
cleanup_fstream:
    th_file_deinit(&entry->stream);
cleanup:
    return err;
}

TH_LOCAL(th_fcache_entry*)
th_fcache_entry_ref(th_fcache_entry* entry)
{
    th_refcounted_ref(&entry->base);
    return entry;
}

TH_PRIVATE(void)
th_fcache_entry_unref(th_fcache_entry* entry)
{
    th_refcounted_unref(&entry->base);
}

TH_PRIVATE(void)
th_fcache_init(th_fcache* cache, th_file_ops* file_ops, th_allocator* allocator)
{
    cache->allocator = allocator ? allocator : th_default_allocator_get();
    cache->file_ops = file_ops;
    th_fcache_map_init(&cache->map, cache->allocator);
    cache->list = (th_fcache_list){NULL, NULL};
    cache->num_cached = 0;
    cache->max_cached = TH_CONFIG_MAX_CACHED_FDS;
}

TH_LOCAL(void)
th_fcache_erase(th_fcache* cache, th_fcache_entry* entry)
{
    th_fcache_list_erase(&cache->list, entry);
    th_fcache_entry_unref(entry);
    --cache->num_cached;
}

TH_LOCAL(th_fcache_entry*)
th_fcache_try_get(th_fcache* cache, th_dir* dir, th_str path)
{
    th_fcache_entry** v = th_fcache_map_try_get(&cache->map, (th_fcache_id){path, dir});
    if (!v)
        return NULL;
    th_fcache_entry* entry = *v;
    // Check if the file has been modified
    uint32_t hash = th_file_stat_hash(&entry->stream);
    if (hash != entry->stat_hash) {
        TH_LOG_TRACE("File has been modified, don't use cached entry");
        th_fcache_erase(cache, entry);
        return NULL;
    }
    // Move entry to the back of the list
    th_fcache_list_erase(&cache->list, entry);
    th_fcache_list_push_back(&cache->list, entry);
    return th_fcache_entry_ref(entry);
}

TH_LOCAL(th_err)
th_fcache_insert(th_fcache* cache, th_fcache_entry* entry)
{
    if (cache->num_cached == cache->max_cached) {
        // Evict the first entry
        th_fcache_entry* first = th_fcache_list_front(&cache->list);
        th_fcache_erase(cache, first);
    }
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_map_set(&cache->map, th_fcache_entry_id(entry), entry)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to insert entry into map: %s", th_strerror(err));
        return err;
    }
    th_fcache_list_push_back(&cache->list, th_fcache_entry_ref(entry));
    cache->num_cached++;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_fcache_get(th_fcache* cache, th_dir* dir, th_str path, th_fcache_entry** out)
{
    th_fcache_entry* entry = th_fcache_try_get(cache, dir, path);
    if (entry) {
        *out = entry;
        return TH_ERR_OK;
    }
    entry = th_allocator_alloc(cache->allocator, sizeof(th_fcache_entry));
    if (!entry)
        return TH_ERR_BAD_ALLOC;
    th_fcache_entry_init(entry, cache, cache->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_entry_open(entry, dir, path)) != TH_ERR_OK) {
        th_allocator_free(cache->allocator, entry);
        return err;
    }
    if ((err = th_fcache_insert(cache, entry)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to insert fcache entry");
        th_fcache_entry_unref(entry);
        return err;
    }
    *out = entry;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_fcache_deinit(th_fcache* cache)
{
    th_fcache_entry* entry = NULL;
    while ((entry = th_fcache_list_pop_front(&cache->list))) {
        th_fcache_entry_unref(entry);
    }
    th_fcache_map_deinit(&cache->map);
}
/* End of src/th_fcache.c */
/* Start of src/th_dir.c */

#if defined(TH_CONFIG_OS_POSIX)
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

TH_LOCAL(th_err)
th_dir_ops_os_open(void* self, const char* path, int* fd)
{
    (void)self;
    int ret = open(path, O_RDONLY | O_DIRECTORY);
    if (ret < 0)
        return TH_ERR_SYSTEM(errno);
    *fd = ret;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_dir_ops_os_close(void* self, int fd)
{
    (void)self;
    int ret = close(fd);
    (void)ret;
    TH_ASSERT(ret == 0 && "This should not happen");
}

TH_PRIVATE(th_dir_ops*)
th_dir_ops_os(void)
{
    static th_dir_ops ops = {
        .open = th_dir_ops_os_open,
        .close = th_dir_ops_os_close,
    };
    return &ops;
}
#endif

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_dir_ops* ops)
{
    dir->ops = ops;
    dir->fd = -1;
}

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_str path)
{
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int fd = -1;
    th_err err = TH_ERR_OK;
    if ((err = dir->ops->open(dir->ops, path_buf, &fd)) != TH_ERR_OK)
        return err;
    dir->fd = fd;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir)
{
    if (dir->fd >= 0)
        dir->ops->close(dir->ops, dir->fd);
}
/* End of src/th_dir.c */
/* Start of src/th_dir_mgr.c */

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator)
{
    mgr->allocator = allocator ? allocator : th_default_allocator_get();
    th_dir_map_init(&mgr->map, allocator);
    th_string_vec_init(&mgr->strings, allocator);
}

TH_LOCAL(bool)
th_dir_mgr_label_exists(th_dir_mgr* mgr, th_str label)
{
    return th_dir_map_find(&mgr->map, label) != NULL;
}

TH_LOCAL(th_err)
th_dir_mgr_store_string(th_dir_mgr* mgr, th_str str)
{
    th_string owned = {0};
    th_string_init(&owned, mgr->allocator);
    if (th_string_set(&owned, str) != TH_ERR_OK) {
        return TH_ERR_BAD_ALLOC;
    }
    if (th_string_vec_push_back(&mgr->strings, owned) != TH_ERR_OK) {
        th_string_deinit(&owned);
        return TH_ERR_BAD_ALLOC;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_str)
th_dir_mgr_get_last_string(th_dir_mgr* mgr)
{
    return th_string_view(th_string_vec_end(&mgr->strings) - 1);
}

TH_LOCAL(void)
th_dir_mgr_remove_last_string(th_dir_mgr* mgr)
{
    th_string_deinit(th_string_vec_end(&mgr->strings) - 1);
    th_string_vec_resize(&mgr->strings, th_string_vec_size(&mgr->strings) - 1);
}

TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_str label, th_dir dir)
{
    th_err err = TH_ERR_OK;
    if (th_dir_mgr_label_exists(mgr, label)) {
        th_dir_deinit(&dir);
        return TH_ERR_INVALID_ARG;
    }
    if ((err = th_dir_mgr_store_string(mgr, label)) != TH_ERR_OK) {
        th_dir_deinit(&dir);
        return err;
    }
    if ((err = th_dir_map_set(&mgr->map, th_dir_mgr_get_last_string(mgr), dir)) != TH_ERR_OK) {
        th_dir_mgr_remove_last_string(mgr);
        th_dir_deinit(&dir);
        return err;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_str label)
{
    th_dir_map_iter it = th_dir_map_find(&mgr->map, label);
    if (it == NULL)
        return NULL;
    return &it->value;
}

TH_PRIVATE(void)
th_dir_mgr_deinit(th_dir_mgr* mgr)
{
    th_dir_map_iter it = th_dir_map_begin(&mgr->map);
    while (it != NULL) {
        th_dir_deinit(&it->value);
        it = th_dir_map_next(&mgr->map, it);
    }
    th_dir_map_deinit(&mgr->map);
    th_string_vec_deinit(&mgr->strings);
}
/* End of src/th_dir_mgr.c */
/* Start of src/th_str.c */

#include <stdbool.h>
#include <string.h>


size_t th_str_npos = (size_t)-1;

TH_PRIVATE(bool)
th_str_is_uint(th_str str)
{
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9') {
            return false;
        }
    }
    return true;
}

TH_PRIVATE(th_err)
th_str_to_uint(th_str str, unsigned int* out)
{
    *out = 0;
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9')
            return TH_ERR_INVALID_ARG;
        *out = *out * 10 + (unsigned int)(str.ptr[i] - '0');
    }
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_str_eq(th_str a, th_str b)
{
    if (a.len != b.len) {
        return 0;
    }
    return memcmp(a.ptr, b.ptr, a.len) == 0;
}

TH_PRIVATE(size_t)
th_str_find_first(th_str str, size_t start, char c)
{
    if (start >= str.len) {
        return th_str_npos;
    }
    const char* found = memchr(str.ptr + start, c, str.len - start);
    return found ? (size_t)(found - str.ptr) : th_str_npos;
}

TH_PRIVATE(size_t)
th_str_find_first_not(th_str str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[i] != c) {
            return i;
        }
    }
    return th_str_npos;
}

TH_PRIVATE(size_t)
th_str_find_first_of(th_str str, size_t start, const char* chars)
{
    size_t chars_len = strlen(chars);
    for (size_t i = start; i < str.len; i++) {
        for (size_t j = 0; j < chars_len; j++) {
            if (str.ptr[i] == chars[j]) {
                return i;
            }
        }
    }
    return th_str_npos;
}

TH_PRIVATE(size_t)
th_str_find_last(th_str str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[str.len - i - 1] == c) {
            return i;
        }
    }
    return th_str_npos;
}

TH_PRIVATE(th_str)
th_str_substr(th_str str, size_t start, size_t len)
{
    if (start >= str.len) {
        return th_str_make(str.ptr + len, 0);
    }
    if (len == th_str_npos || start + len > str.len) {
        len = str.len - start;
    }
    return th_str_make(str.ptr + start, len);
}

TH_PRIVATE(th_str)
th_str_trim(th_str str)
{
    size_t start = 0;
    while (start < str.len && (str.ptr[start] == ' ' || str.ptr[start] == '\t')) {
        start++;
    }
    size_t end = str.len;
    while (end > start && (str.ptr[end - 1] == ' ' || str.ptr[end - 1] == '\t')) {
        end--;
    }
    return th_str_substr(str, start, end - start);
}

TH_PRIVATE(size_t)
th_str_hash(th_str str)
{
    return th_hash_bytes(str.ptr, str.len);
}
/* End of src/th_str.c */
/* Start of src/th_string.c */

#include <ctype.h>

#define TH_STRING_SMALL (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 2)
#define TH_STRING_ALIGNUP(size) TH_ALIGNUP(size, 16)
TH_LOCAL(void)
th_detail_small_string_init(th_detail_small_string* self, th_allocator* allocator)
{
    self->small = 1;
    self->len = 0;
    self->buf[0] = '\0';
    self->allocator = allocator;
    if (self->allocator == NULL) {
        self->allocator = th_default_allocator_get();
    }
}

TH_PRIVATE(void)
th_string_init(th_string* self, th_allocator* allocator)
{
    th_detail_small_string_init(&self->impl.small, allocator);
}

TH_PRIVATE(th_err)
th_string_init_with(th_string* self, th_str str, th_allocator* allocator)
{
    th_string_init(self, allocator);
    return th_string_set(self, str);
}

TH_LOCAL(void)
th_detail_small_string_set(th_detail_small_string* self, th_str str)
{
    TH_ASSERT(str.len <= TH_STRING_SMALL_MAX_LEN);
    if (str.len > 0)
        memcpy(self->buf, str.ptr, str.len);
    self->buf[str.len] = '\0';
    self->len = str.len & 0x7F;
}

TH_LOCAL(th_err)
th_detail_large_string_set(th_detail_large_string* self, th_str str)
{
    size_t required_capacity = str.len + 1;
    if (self->capacity < required_capacity) {
        size_t new_capacity = TH_STRING_ALIGNUP(required_capacity);
        char* new_ptr = th_allocator_realloc(self->allocator, self->ptr, new_capacity);
        if (new_ptr == NULL) {
            return TH_ERR_BAD_ALLOC;
        }
        self->ptr = new_ptr;
        self->capacity = new_capacity;
    }
    self->len = str.len;
    if (str.len > 0)
        memcpy(self->ptr, str.ptr, str.len);
    self->ptr[str.len] = '\0';
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_string_small_to_large(th_string* self, size_t capacity)
{
    TH_ASSERT(self->impl.small.small);
    th_detail_large_string large = {0};
    capacity = TH_STRING_ALIGNUP(capacity);
    large.capacity = capacity;
    large.len = self->impl.small.len;
    large.ptr = th_allocator_alloc(self->impl.small.allocator, capacity);
    if (large.ptr == NULL) {
        return TH_ERR_BAD_ALLOC;
    }
    large.allocator = self->impl.small.allocator;
    memcpy(large.ptr, self->impl.small.buf, self->impl.small.len);
    large.ptr[self->impl.small.len] = '\0';
    self->impl.large = large;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_string_set(th_string* self, th_str str)
{
    TH_ASSERT(str.ptr != NULL && "Invalid string");
    if (self->impl.small.small) {
        if (str.len <= TH_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_set(&self->impl.small, str);
            return TH_ERR_OK;
        } else {
            th_err err = th_string_small_to_large(self, str.len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_set(&self->impl.large, str);
}

TH_LOCAL(void)
th_detail_small_string_append(th_detail_small_string* self, th_str str)
{
    TH_ASSERT(self->len + str.len <= TH_STRING_SMALL_MAX_LEN);
    memcpy(self->buf + self->len, str.ptr, str.len);
    self->len += str.len & 0x7F;
    self->buf[self->len] = '\0';
}

TH_LOCAL(th_err)
th_detail_large_string_append(th_detail_large_string* self, th_str str)
{
    size_t required_capacity = self->len + str.len + 1;
    if (required_capacity > self->capacity) {
        size_t new_capacity = TH_STRING_ALIGNUP(required_capacity);
        char* new_ptr = th_allocator_realloc(self->allocator, self->ptr, new_capacity);
        if (new_ptr == NULL) {
            return TH_ERR_BAD_ALLOC;
        }
        self->ptr = new_ptr;
        self->capacity = new_capacity;
    }
    memcpy(self->ptr + self->len, str.ptr, str.len);
    self->len += str.len;
    self->ptr[self->len] = '\0';
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_string_append(th_string* self, th_str str)
{
    if (self->impl.small.small) {
        if (self->impl.small.len + str.len <= TH_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_append(&self->impl.small, str);
            return TH_ERR_OK;
        } else {
            th_err err = th_string_small_to_large(self, self->impl.small.len + str.len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_append(&self->impl.large, str);
}

TH_PRIVATE(th_err)
th_string_append_cstr(th_string* self, const char* str)
{
    return th_string_append(self, th_str_make(str, strlen(str)));
}

TH_PRIVATE(th_err)
th_string_push_back(th_string* self, char c)
{
    return th_string_append(self, (th_str){&c, 1});
}

TH_LOCAL(void)
th_detail_small_string_resize(th_detail_small_string* self, size_t new_len, char fill)
{
    TH_ASSERT(new_len <= TH_STRING_SMALL_MAX_LEN && "Invalid length");
    if (new_len > self->len)
        memset(self->buf + self->len, fill, new_len - self->len);
    self->len = new_len & 0x7F;
    self->buf[new_len] = '\0';
}

TH_LOCAL(th_err)
th_detail_large_string_resize(th_detail_large_string* self, size_t new_len, char fill)
{
    size_t required_capacity = new_len + 1;
    if (required_capacity > self->capacity) {
        size_t new_capacity = TH_STRING_ALIGNUP(required_capacity);
        char* new_ptr = th_allocator_realloc(self->allocator, self->ptr, new_capacity);
        if (new_ptr == NULL) {
            return TH_ERR_BAD_ALLOC;
        }
        self->ptr = new_ptr;
        self->capacity = new_capacity;
    }
    if (new_len > self->len)
        memset(self->ptr + self->len, fill, new_len - self->len);
    self->len = new_len;
    self->ptr[new_len] = '\0';
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_string_resize(th_string* self, size_t new_len, char fill)
{
    if (self->impl.small.small) {
        if (new_len <= TH_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_resize(&self->impl.small, new_len, fill);
            return TH_ERR_OK;
        } else {
            th_err err = th_string_small_to_large(self, new_len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_resize(&self->impl.large, new_len, fill);
}

TH_PRIVATE(th_str)
th_string_view(const th_string* self)
{
    if (self->impl.small.small) {
        return (th_str){self->impl.small.buf, self->impl.small.len};
    } else {
        return (th_str){self->impl.large.ptr, self->impl.large.len};
    }
}

TH_PRIVATE(const char*)
th_string_data(const th_string* self)
{
    if (self->impl.small.small) {
        return self->impl.small.buf;
    } else {
        return self->impl.large.ptr;
    }
}

TH_PRIVATE(char*)
th_string_at(th_string* self, size_t index)
{
    TH_ASSERT(index < th_string_len(self) && "Index out of bounds");
    if (self->impl.small.small) {
        return &self->impl.small.buf[index];
    } else {
        return &self->impl.large.ptr[index];
    }
}

TH_PRIVATE(size_t)
th_string_len(const th_string* self)
{
    if (self->impl.small.small) {
        return self->impl.small.len;
    } else {
        return self->impl.large.len;
    }
}

TH_PRIVATE(void)
th_string_clear(th_string* self)
{
    if (self->impl.small.small) {
        self->impl.small.len = 0;
        self->impl.small.buf[0] = '\0';
    } else {
        self->impl.large.len = 0;
        self->impl.large.ptr[0] = '\0';
    }
}

TH_PRIVATE(void)
th_string_to_lower(th_string* self)
{
    char* ptr = th_string_at(self, 0);
    size_t n = th_string_len(self);
    for (size_t i = 0; i < n; i++) {
        ptr[i] = (char)tolower((int)ptr[i]);
    }
}

TH_PRIVATE(bool)
th_string_eq(const th_string* self, th_str other)
{
    const char* ptr = NULL;
    size_t n = 0;
    if (self->impl.small.small) {
        ptr = self->impl.small.buf;
        n = self->impl.small.len;
    } else {
        ptr = self->impl.large.ptr;
        n = self->impl.large.len;
    }
    return n == other.len && (n == 0 || memcmp(ptr, other.ptr, n) == 0);
}

// TH_PRIVATE(uint32_t)
// th_string_hash(const th_string* self)
//{
//     const char* ptr = NULL;
//     size_t n = 0;
//     if (self->impl.small.small) {
//         ptr = self->impl.small.buf;
//         n = self->impl.small.len;
//     } else {
//         ptr = self->impl.large.ptr;
//         n = self->impl.large.len;
//     }
//     return th_hash_bytes(ptr, n);
// }

TH_PRIVATE(void)
th_string_deinit(th_string* self)
{
    if (!self->impl.small.small) {
        th_allocator_free(self->impl.large.allocator, self->impl.large.ptr);
    }
}
/* End of src/th_string.c */
/* Start of src/th_log.c */

#include <stdio.h>

/* global log instance */

static th_log* th_user_log = NULL;

TH_PUBLIC(void)
th_log_set(th_log* log)
{
    th_user_log = log;
}

/** th_log_get
 * @brief  Get the current user log instance.
 * @return The current user log instance, or the default log instance if no user log is set.
 */
TH_LOCAL(th_log*)
th_log_get(void)
{
    return th_user_log ? th_user_log : th_default_log_get();
}

/* th_default_log implementation begin */

/** th_default_log
 * @brief Default log implementation, simply prints log messages to stderr.
 */
typedef struct th_default_log {
    th_log base;
} th_default_log;

TH_LOCAL(void)
th_default_log_print(void* self, int level, const char* msg)
{
    (void)self;
    (void)level;
    fprintf(stderr, "%s\n", msg);
}

TH_PRIVATE(th_log*)
th_default_log_get(void)
{
    static th_default_log log = {
        .base = {
            .print = th_default_log_print,
        }};
    return (th_log*)&log;
}

TH_PRIVATE(void)
TH_PRINTF_FMT(2, 3)
th_log_printf(int level, const char* fmt, ...)
{
    th_log* log = th_log_get();
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if (ret < 0 || (size_t)ret >= sizeof(buffer))
        goto on_error;
    log->print(log, level, buffer);
    return;
on_error:
    log->print(log, TH_LOG_LEVEL_ERROR, "ERROR: [th_log] Failed to format log message");
}

/* th_default_log implementation end */
/* End of src/th_log.c */
/* Start of src/th_http.c */

#include <stdbool.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "http"

#define TH_HTTP_CLOSE true
#define TH_HTTP_KEEP_ALIVE false

TH_LOCAL(void)
th_http_destroy(void* self)
{
    th_http* http = self;
    TH_LOG_TRACE("%p: Destroying http protocol instance", http);
    th_conn_destroy(http->conn);
    th_request_deinit(&http->request);
    th_response_deinit(&http->response);
    th_buf_vec_deinit(&http->buf);
    th_allocator_free(http->allocator, http);
}

TH_LOCAL(void)
th_http_handle_read_request(void* user_data, size_t len, th_err err);

TH_LOCAL(void)
th_http_handle_write_response(void* user_data, size_t len, th_err err);

TH_LOCAL(void)
th_http_restart(th_http* http)
{
    http->read_bytes = 0;
    http->parsed_bytes = 0;
    th_request_parser_reset(&http->parser);
    th_request_reset(&http->request);
    th_response_reset(&http->response);
    th_conn_recv(http->conn, th_buf_vec_at(&http->buf, 0), th_buf_vec_size(&http->buf), false, th_http_handle_read_request, http);
}

TH_LOCAL(void)
th_http_complete(th_http* http)
{
    if (http->close) {
        th_http_destroy(http);
    } else {
        th_http_restart(http);
    }
}

TH_LOCAL(void)
th_http_write_response(th_http* http)
{
    th_response_async_write(&http->response, http->conn, th_http_handle_write_response, http);
}

TH_LOCAL(void)
th_http_write_error_response(th_http* http, th_err err)
{
    th_response_set_code(&http->response, TH_ERR_CODE(err));
    if (th_string_len(&http->request.uri_path) == 0) {
        // Set default error message
        th_printf_body(&http->response, "%d %s", TH_ERR_CODE(err), th_http_strerror((int)err));
    }
    if (http->close) {
        th_response_add_header(&http->response, TH_STR("Connection"), TH_STR("close"));
        http->close = TH_HTTP_CLOSE;
    }
    th_http_write_response(http);
}

TH_LOCAL(void)
th_http_handle_error(th_http* http, th_err err)
{
    th_http_code_type type = th_http_code_get_type(TH_ERR_CODE(err));
    switch (type) {
    case TH_HTTP_CODE_TYPE_SERVER_ERROR:
        http->close = TH_HTTP_CLOSE;
        break;
    case TH_HTTP_CODE_TYPE_CLIENT_ERROR:
        break;
    default:
        TH_ASSERT(0 && "Invalid error type");
        break;
    }
    th_http_write_error_response(http, err);
}

TH_LOCAL(void)
th_http_handle_require_1_1(th_http* http)
{
    TH_LOG_ERROR("%p: Trying send a HTTP/1.1 response to a HTTP/1.0 client, sending 400 Bad Request instead", (void*)http);
    th_response_set_body(&http->response, TH_STR("HTTP/1.1 required for this request"));
    th_http_handle_error(http, TH_ERR_HTTP(TH_CODE_BAD_REQUEST));
}

TH_LOCAL(th_err)
th_http_handle_options(th_router* router, th_request* request, th_response* response)
{
    // All the methods we gotta check
    static const struct {
        th_method method;
        const char* allow;
    } methods[] = {
        {TH_METHOD_GET, "GET, HEAD"},
        {TH_METHOD_POST, "POST"},
        {TH_METHOD_PUT, "PUT"},
        {TH_METHOD_DELETE, "DELETE"},
        {TH_METHOD_PATCH, "PATCH"},
    };
    char allow[512] = {0};
    size_t pos = th_fmt_str_append(allow, 0, sizeof(allow), "OPTIONS"); // OPTIONS is always allowed
    if (strcmp(th_string_data(&request->uri_path), "*") != 0) {
        for (size_t i = 0; i < TH_ARRAY_SIZE(methods); i++) {
            if (th_router_would_handle(router, methods[i].method, request)) {
                pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, ", ");
                pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, methods[i].allow);
            }
        }
    } else {
        for (size_t i = 0; i < TH_ARRAY_SIZE(methods); i++) {
            pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, ", ");
            pos += th_fmt_str_append(allow, pos, sizeof(allow) - pos, methods[i].allow);
        }
    }
    th_err err = TH_ERR_OK;
    if ((err = th_response_add_header(response, TH_STR("Allow"), th_str_make(allow, pos))) != TH_ERR_OK)
        return err;
    if ((err = th_response_add_header(response, TH_STR("Content-Type"), TH_STR("text/plain"))) != TH_ERR_OK)
        return err;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_http_handle_route(th_router* router, th_request* request, th_response* response)
{
    if (request->method == TH_METHOD_OPTIONS) {
        return th_http_handle_options(router, request, response);
    } else {
        return th_router_handle(router, request, response);
    }
}

TH_LOCAL(void)
th_http_prehandle_request(th_http* http)
{
    th_request* request = &http->request;
    th_response* response = &http->response;
    if (request->method == TH_METHOD_HEAD) {
        response->only_headers = true;   // only write headers
        request->method = TH_METHOD_GET; // pretend it's a GET request
    }
}

TH_LOCAL(void)
th_http_handle_request_and_write_response(th_http* http)
{
    th_request* request = &http->request;
    th_response* response = &http->response;
    th_http_prehandle_request(http);
    th_err err = th_http_error(th_http_handle_route(http->router, &http->request, &http->response));
    switch (th_http_code_get_type(TH_ERR_CODE(err))) {
    case TH_HTTP_CODE_TYPE_INFORMATIONAL:
        if (request->version == 0) {
            th_http_handle_require_1_1(http);
            return;
        }
        break;
    case TH_HTTP_CODE_TYPE_SERVER_ERROR:
    case TH_HTTP_CODE_TYPE_CLIENT_ERROR:
        th_http_handle_error(http, err);
        return;
    default:
        // All other types don't require any special handling
        break;
    }
    // All good, write the response
    if (request->close) {
        th_response_add_header(response, TH_STR("Connection"), TH_STR("close"));
        http->close = true;
    } else {
        th_response_add_header(response, TH_STR("Connection"), TH_STR("keep-alive"));
    }
    TH_LOG_TRACE("%p: Write response %p", http, response);
    th_http_write_response(http);
}

TH_LOCAL(void)
th_http_handle_read_request(void* user_data, size_t len, th_err err)
{
    th_http* http = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: Read error: %s", http, th_strerror(err));
        http->close = TH_HTTP_CLOSE; // No other choice if we can't even read the request
        th_http_complete(http);
        return;
    }
    http->read_bytes += len;
    size_t parsed = 0;
    th_str parser_input = (th_str){.ptr = th_buf_vec_at(&http->buf, http->parsed_bytes),
                                   .len = http->read_bytes - http->parsed_bytes};
    if ((err = th_request_parser_parse(&http->parser, &http->request, parser_input, &parsed)) != TH_ERR_OK) {
        th_http_write_error_response(http, th_http_error(err));
        return;
    }
    if (th_request_parser_done(&http->parser)) {
        th_http_handle_request_and_write_response(http);
        return;
    }
    // If we haven't parsed the whole request, we need to read more data
    http->parsed_bytes += parsed;
    if (!th_request_parser_header_done(&http->parser)) {
        if (http->read_bytes == th_buf_vec_size(&http->buf)) {
            if (th_buf_vec_size(&http->buf) < TH_CONFIG_LARGE_HEADER_LEN) {
                th_buf_vec_resize(&http->buf, TH_CONFIG_LARGE_HEADER_LEN);
            } else {
                th_http_write_error_response(http, TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE));
                return;
            }
        }
        th_conn_recv(http->conn, th_buf_vec_at(&http->buf, http->read_bytes),
                     th_buf_vec_size(&http->buf) - http->read_bytes, false, th_http_handle_read_request, http);
    } else {
        if (th_conn_tracker_count(http->tracker) > TH_CONFIG_MAX_CONNECTIONS) {
            TH_LOG_WARN("Too many connections, rejecting new connection");
            th_http_write_error_response(http, TH_ERR_HTTP(TH_CODE_SERVICE_UNAVAILABLE));
            return;
        }
        size_t content_received = http->read_bytes - http->parsed_bytes;
        size_t content_len = th_request_parser_content_len(&http->parser);
        if (content_len > TH_MAX_BODY_LEN) {
            TH_LOG_WARN("Request body too large, rejecting request");
            th_http_write_error_response(http, TH_ERR_HTTP(TH_CODE_PAYLOAD_TOO_LARGE));
            return;
        }
        size_t remaining = content_len - content_received;
        if (http->read_bytes + remaining > th_buf_vec_size(&http->buf)) {
            memcpy(th_buf_vec_at(&http->buf, 0), th_buf_vec_at(&http->buf, http->parsed_bytes), content_received);
            http->read_bytes = content_received;
            http->parsed_bytes = 0;
            if (content_len > th_buf_vec_size(&http->buf)) {
                th_buf_vec_resize(&http->buf, content_len);
            }
        }
        th_conn_recv(http->conn, th_buf_vec_at(&http->buf, http->read_bytes),
                     remaining, true, th_http_handle_read_request, http);
    }
}

TH_LOCAL(void)
th_http_handle_write_response(void* user_data, size_t len, th_err err)
{
    th_http* http = user_data;
    (void)len;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p: Write error: %s", (void*)http, th_strerror(err));
        http->close = TH_HTTP_CLOSE; // Connection is broken, close it
    } else {
        TH_LOG_TRACE("%p: Write response of %d bytes", http, (int)len);
    }
    th_http_complete(http);
}

TH_LOCAL(void)
th_http_start(void* self)
{
    th_http* http = self;
    TH_LOG_TRACE("%p: Starting", http);
    th_buf_vec_resize(&http->buf, TH_CONFIG_SMALL_HEADER_LEN);
    th_conn_recv(http->conn, th_buf_vec_at(&http->buf, 0), th_buf_vec_size(&http->buf), false, th_http_handle_read_request, http);
}

TH_LOCAL(void)
th_http_init(th_http* http, const th_conn_tracker* tracker, th_conn* conn,
             th_router* router, th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_request_parser_init(&http->parser);
    th_request_init(&http->request, allocator);
    th_response_init(&http->response, dir_mgr, fcache, allocator);
    th_buf_vec_init(&http->buf, allocator);
    http->tracker = tracker;
    http->conn = conn;
    http->router = router;
    http->dir_mgr = dir_mgr;
    http->fcache = fcache;
    http->allocator = allocator;
    http->read_bytes = 0;
    http->parsed_bytes = 0;
    http->close = TH_HTTP_KEEP_ALIVE;
}

TH_LOCAL(th_err)
th_http_create(th_http** out, const th_conn_tracker* tracker, th_conn* conn,
               th_router* router, th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator)
{
    th_http* http = th_allocator_alloc(allocator, sizeof(th_http));
    if (!http)
        return TH_ERR_BAD_ALLOC;
    th_http_init(http, tracker, conn, router, dir_mgr, fcache, allocator);
    *out = http;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_http_upgrader_upgrade(void* self, th_conn* conn)
{
    th_http_upgrader* upgrader = self;
    th_http* http = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_http_create(&http, upgrader->tracker, conn, upgrader->router, upgrader->dir_mgr, upgrader->fcache, upgrader->allocator)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to create http instance: %s", th_strerror(err));
        th_conn_destroy(conn);
        return;
    }
    th_http_start(http);
}

TH_PRIVATE(void)
th_http_upgrader_init(th_http_upgrader* upgrader, const th_conn_tracker* tracker, th_router* router,
                      th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator)
{
    th_conn_upgrader_init(&upgrader->base, th_http_upgrader_upgrade);
    upgrader->tracker = tracker;
    upgrader->router = router;
    upgrader->dir_mgr = dir_mgr;
    upgrader->fcache = fcache;
    upgrader->allocator = allocator;
}
/* End of src/th_http.c */
/* Start of src/th_fmt.c */


static const char* th_fmt_num_table[] =
    {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
     "10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
     "20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
     "30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
     "40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
     "50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
     "60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
     "70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
     "80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
     "90", "91", "92", "93", "94", "95", "96", "97", "98", "99",
     "100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
     "110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
     "120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
     "130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
     "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
     "150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
     "160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
     "170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
     "180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
     "190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
     "200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
     "210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
     "220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
     "230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
     "240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
     "250", "251", "252", "253", "254", "255", "256", "257", "258", "259",
     "260", "261", "262", "263", "264", "265", "266", "267", "268", "269",
     "270", "271", "272", "273", "274", "275", "276", "277", "278", "279",
     "280", "281", "282", "283", "284", "285", "286", "287", "288", "289",
     "290", "291", "292", "293", "294", "295", "296", "297", "298", "299",
     "300", "301", "302", "303", "304", "305", "306", "307", "308", "309",
     "310", "311", "312", "313", "314", "315", "316", "317", "318", "319",
     "320", "321", "322", "323", "324", "325", "326", "327", "328", "329",
     "330", "331", "332", "333", "334", "335", "336", "337", "338", "339",
     "340", "341", "342", "343", "344", "345", "346", "347", "348", "349",
     "350", "351", "352", "353", "354", "355", "356", "357", "358", "359",
     "360", "361", "362", "363", "364", "365", "366", "367", "368", "369",
     "370", "371", "372", "373", "374", "375", "376", "377", "378", "379",
     "380", "381", "382", "383", "384", "385", "386", "387", "388", "389",
     "390", "391", "392", "393", "394", "395", "396", "397", "398", "399",
     "400", "401", "402", "403", "404", "405", "406", "407", "408", "409",
     "410", "411", "412", "413", "414", "415", "416", "417", "418", "419",
     "420", "421", "422", "423", "424", "425", "426", "427", "428", "429",
     "430", "431", "432", "433", "434", "435", "436", "437", "438", "439",
     "440", "441", "442", "443", "444", "445", "446", "447", "448", "449",
     "450", "451", "452", "453", "454", "455", "456", "457", "458", "459",
     "460", "461", "462", "463", "464", "465", "466", "467", "468", "469",
     "470", "471", "472", "473", "474", "475", "476", "477", "478", "479",
     "480", "481", "482", "483", "484", "485", "486", "487", "488", "489",
     "490", "491", "492", "493", "494", "495", "496", "497", "498", "499",
     "500", "501", "502", "503", "504", "505", "506", "507", "508", "509",
     "510", "511", "512", "513", "514", "515", "516", "517", "518", "519",
     "520", "521", "522", "523", "524", "525", "526", "527", "528", "529",
     "530", "531", "532", "533", "534", "535", "536", "537", "538", "539",
     "540", "541", "542", "543", "544", "545", "546", "547", "548", "549",
     "550", "551", "552", "553", "554", "555", "556", "557", "558", "559",
     "560", "561", "562", "563", "564", "565", "566", "567", "568", "569",
     "570", "571", "572", "573", "574", "575", "576", "577", "578", "579",
     "580", "581", "582", "583", "584", "585", "586", "587", "588", "589",
     "590", "591", "592", "593", "594", "595", "596", "597", "598", "599"};

TH_PRIVATE(const char*)
th_fmt_uint_to_str(char* buf, size_t len, unsigned int value)
{
    if (value < TH_ARRAY_SIZE(th_fmt_num_table)) {
        return th_fmt_num_table[value];
    }

    buf[len - 1] = '\0';
    size_t i = len - 2;
    unsigned int v = value;
    while (v > 0 && i > 0) {
        buf[i--] = '0' + (char)(v % 10);
        v /= 10;
    }
    return &buf[i + 1];
}

TH_PRIVATE(const char*)
th_fmt_uint_to_str_ex(char* buf, size_t len, unsigned int val, size_t* out_len)
{
    if (val < TH_ARRAY_SIZE(th_fmt_num_table)) {
        *out_len = val < 10 ? 1 : (val < 100 ? 2 : 3);
        return th_fmt_num_table[val];
    }

    buf[len - 1] = '\0';
    size_t i = len - 2;
    unsigned int v = val;
    while (v > 0 && i > 0) {
        buf[i--] = '0' + (char)(v % 10);
        v /= 10;
    }
    *out_len = len - i - 2;
    return &buf[i + 1];
}

TH_PRIVATE(size_t)
th_fmt_str_append(char* buf, size_t pos, size_t len, const char* str)
{
    size_t i = 0;
    while (str[i] != '\0' && pos < len - 1) {
        buf[pos++] = str[i++];
    }
    buf[pos] = '\0';
    return i;
}

TH_PRIVATE(size_t)
th_fmt_strn_append(char* buf, size_t pos, size_t len, const char* str, size_t n)
{
    size_t i = 0;
    while (str[i] != '\0' && i < n && pos < len - 1) {
        buf[pos++] = str[i++];
    }
    buf[pos] = '\0';
    return i;
}

TH_PRIVATE(size_t)
th_fmt_strtime(char* buf, size_t len, th_date date)
{
    static const char* weekday_table[] =
        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    static const char* month_table[] =
        {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    size_t pos = 0;
#define ADVANCE_POS() pos += (pos < len - 1)
    // Weekday
    pos += th_fmt_strn_append(buf, pos, len, weekday_table[date.weekday], 3);
    buf[pos] = ',';
    ADVANCE_POS();
    buf[pos] = ' ';
    ADVANCE_POS();

    // Day
    char numbuf[16] = {0};
    size_t numlen = 0;
    const char* day = th_fmt_uint_to_str_ex(numbuf, sizeof(numbuf), date.day, &numlen);
    pos += th_fmt_strn_append(buf, pos, len, day, numlen);
    buf[pos] = ' ';
    ADVANCE_POS();

    // Month
    pos += th_fmt_strn_append(buf, pos, len, month_table[date.month], 3);
    buf[pos] = ' ';
    ADVANCE_POS();

    // Year
    const char* year = th_fmt_uint_to_str_ex(numbuf, sizeof(numbuf), date.year + 1900, &numlen);
    pos += th_fmt_strn_append(buf, pos, len, year, numlen);
    buf[pos] = ' ';
    ADVANCE_POS();

    // Hour
    const char* hour = th_fmt_uint_to_str_ex(numbuf, sizeof(numbuf), date.hour, &numlen);
    pos += th_fmt_strn_append(buf, pos, len, hour, numlen);
    buf[pos] = ':';
    ADVANCE_POS();

    // Minute
    const char* min = th_fmt_uint_to_str_ex(numbuf, sizeof(numbuf), date.minute, &numlen);
    pos += th_fmt_strn_append(buf, pos, len, min, numlen);
    buf[pos] = ':';
    ADVANCE_POS();

    // Second
    const char* sec = th_fmt_uint_to_str_ex(numbuf, sizeof(numbuf), date.second, &numlen);
    pos += th_fmt_strn_append(buf, pos, len, sec, numlen);
    buf[pos] = ' ';
    ADVANCE_POS();

    // Timezone
    pos += th_fmt_strn_append(buf, pos, len, "GMT", 3);
    buf[pos] = '\0';
    return pos;
#undef ADVANCE_POS
}
/* End of src/th_fmt.c */
/* Start of src/th_date.c */

#include <time.h>


TH_PUBLIC(th_duration)
th_seconds(int seconds)
{
    return (th_duration){.seconds = seconds};
}

TH_PUBLIC(th_duration)
th_minutes(int minutes)
{
    return th_seconds(minutes * 60);
}

TH_PUBLIC(th_duration)
th_hours(int hours)
{
    return th_minutes(hours * 60);
}

TH_PUBLIC(th_duration)
th_days(int days)
{
    return th_hours(days * 24);
}

TH_PUBLIC(th_date)
th_date_now(void)
{
    time_t t = time(NULL);
    struct tm tm = {0};
    gmtime_r(&t, &tm);
    th_date date = {0};
    date.year = (unsigned int)tm.tm_year & 0xFFFF;
    date.month = (unsigned int)tm.tm_mon & 0xFF;
    date.day = (unsigned int)tm.tm_mday & 0xFF;
    date.weekday = (unsigned int)tm.tm_wday & 0xFF;
    date.hour = (unsigned int)tm.tm_hour & 0xFF;
    date.minute = (unsigned int)tm.tm_min & 0xFF;
    date.second = (unsigned int)tm.tm_sec & 0xFF;
    return date;
}

TH_PUBLIC(th_date)
th_date_add(th_date date, th_duration d)
{
    struct tm tm = {0};
    tm.tm_year = date.year;
    tm.tm_mon = date.month;
    tm.tm_mday = date.day;
    tm.tm_hour = date.hour;
    tm.tm_min = date.minute;
    tm.tm_sec = date.second;
    time_t t = mktime(&tm);
    t += d.seconds;
    gmtime_r(&t, &tm);
    th_date new_date = {0};
    new_date.year = (unsigned int)tm.tm_year & 0xFFFF;
    new_date.month = (unsigned int)tm.tm_mon & 0xFF;
    new_date.day = (unsigned int)tm.tm_mday & 0xFF;
    new_date.weekday = (unsigned int)tm.tm_wday & 0xFF;
    new_date.hour = (unsigned int)tm.tm_hour & 0xFF;
    new_date.minute = (unsigned int)tm.tm_min & 0xFF;
    new_date.second = (unsigned int)tm.tm_sec & 0xFF;
    return new_date;
}
/* End of src/th_date.c */
/* Start of src/th_clock.c */

#ifdef TH_CONFIG_OS_POSIX
#include <errno.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <windows.h>
#endif

TH_LOCAL(th_err)
th_os_clock_monotonic_now(void* self, time_t* out)
{
    (void)self;
#if defined(TH_CONFIG_OS_POSIX)
    struct timespec ts = {0};
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return TH_ERR_SYSTEM(errno);
    }
    *out = ts.tv_sec;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_WIN)
    *out = (time_t)(GetTickCount64() / 1000);
    return TH_ERR_OK;
#else
    (void)out;
    return TH_ERR_NOSUPPORT;
#endif
}

TH_PRIVATE(th_clock*)
th_clock_os(void)
{
    static th_clock os_clock = {
        .monotonic_now = th_os_clock_monotonic_now,
    };
    return &os_clock;
}
/* End of src/th_clock.c */
/* Start of src/th_timer.c */

TH_PRIVATE(void)
th_timer_init(th_timer* timer, th_clock* clock)
{
    timer->clock = clock;
    timer->expire = 0;
}

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration)
{
    time_t now = 0;
    th_err err = timer->clock->monotonic_now(timer->clock, &now);
    TH_ASSERT(err == TH_ERR_OK && "clock->monotonic_now failed");
    if (err != TH_ERR_OK)
        return err;
    timer->expire = now + duration.seconds;
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer)
{
    time_t now = 0;
    th_err err = timer->clock->monotonic_now(timer->clock, &now);
    TH_ASSERT(err == TH_ERR_OK && "clock->monotonic_now failed");
    /* We don't return the error here, as it's already handled in th_timer_set
     * and we can safely assume that the error won't happen here. */
    if (err != TH_ERR_OK)
        return true;
    return now >= timer->expire;
}

TH_PRIVATE(th_timer)
th_timer_from_duration(th_clock* clock, th_duration duration)
{
    th_timer timer;
    th_timer_init(&timer, clock);
    th_timer_set(&timer, duration);
    return timer;
}

TH_PRIVATE(th_duration)
th_timer_remaining(const th_timer* timer)
{
    time_t now = 0;
    th_err err = timer->clock->monotonic_now(timer->clock, &now);
    TH_ASSERT(err == TH_ERR_OK && "clock->monotonic_now failed");
    if (err != TH_ERR_OK)
        return th_seconds(0);
    return th_seconds(TH_MAX((int)(timer->expire - now), 0));
}

TH_PRIVATE(bool)
th_timer_less(const th_timer* a, const th_timer* b)
{
    return a->expire < b->expire;
}
/* End of src/th_timer.c */
/* Start of src/th_conn_tracker.c */

TH_LOCAL(void)
th_conn_tracker_on_conn_init(th_conn_observer* observer, th_conn_observable* observable)
{
    th_conn_tracker* tracker = (th_conn_tracker*)observer;
    th_conn_observable_list_push_back(&tracker->observables, observable);
    ++tracker->count;
}

TH_LOCAL(void)
th_conn_tracker_on_conn_deinit(th_conn_observer* observer, th_conn_observable* observable)
{
    th_conn_tracker* tracker = (th_conn_tracker*)observer;
    th_conn_observable_list_erase(&tracker->observables, observable);
    --tracker->count;
    if (tracker->task) {
        th_task* task = TH_MOVE_PTR(tracker->task);
        th_task_complete(task);
    }
}

TH_PRIVATE(void)
th_conn_tracker_init(th_conn_tracker* tracker)
{
    tracker->base.on_init = th_conn_tracker_on_conn_init;
    tracker->base.on_deinit = th_conn_tracker_on_conn_deinit;
    tracker->observables = (th_conn_observable_list){0};
    tracker->task = NULL;
    tracker->count = 0;
}

TH_PRIVATE(void)
th_conn_tracker_cancel_all(th_conn_tracker* conn_tracker)
{
    th_conn_observable* observable = NULL;
    for (observable = th_conn_observable_list_front(&conn_tracker->observables);
         observable != NULL;
         observable = th_conn_observable_list_next(observable)) {
        th_conn* client = &observable->base;
        th_conn_cancel(client);
    }
}

TH_PRIVATE(void)
th_conn_tracker_async_wait(th_conn_tracker* conn_tracker, th_task* task)
{
    TH_ASSERT(conn_tracker->task == NULL && "Task already set");
    TH_ASSERT(th_conn_observable_list_front(&conn_tracker->observables) != NULL && "No clients to wait for");
    conn_tracker->task = task;
}

TH_PRIVATE(size_t)
th_conn_tracker_count(const th_conn_tracker* conn_tracker)
{
    return conn_tracker->count;
}

TH_PRIVATE(void)
th_conn_tracker_deinit(th_conn_tracker* tracker)
{
    (void)tracker;
    TH_ASSERT(th_conn_observable_list_front(&tracker->observables) == NULL && "All clients must be destroyed before deinit");
}
/* End of src/th_conn_tracker.c */
/* Start of src/th_url_decode.c */

TH_LOCAL(th_err)
th_url_decode_next(th_str str, size_t* pos, char* out, th_url_decode_type type)
{
    size_t i = *pos;
    if (str.ptr[i] == '%') {
        int c = 0;
        for (size_t k = 0; k < 2; k++) {
            if (i + 1 + k >= str.len)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            c <<= 4;
            if (str.ptr[i + 1 + k] >= '0' && str.ptr[i + 1 + k] <= '9') {
                c |= str.ptr[i + 1 + k] - '0';
            } else if (str.ptr[i + 1 + k] >= 'a' && str.ptr[i + 1 + k] <= 'f') {
                c |= str.ptr[i + 1 + k] - 'a' + 10;
            } else if (str.ptr[i + 1 + k] >= 'A' && str.ptr[i + 1 + k] <= 'F') {
                c |= str.ptr[i + 1 + k] - 'A' + 10;
            } else {
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            }
        }
        *out = (char)c;
        i += 3;
    } else if (type == TH_URL_DECODE_TYPE_QUERY && str.ptr[i] == '+') {
        *out = ' ';
        i++;
    } else {
        *out = str.ptr[i++];
    }
    *pos = i;
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_url_decode_literal_run(th_str input, size_t pos, th_url_decode_type type)
{
    size_t start = pos;
    while (pos < input.len && input.ptr[pos] != '%'
           && !(type == TH_URL_DECODE_TYPE_QUERY && input.ptr[pos] == '+'))
        pos++;
    return pos - start;
}

TH_PRIVATE(th_err)
th_url_decode_string(th_str input, th_string* output, th_url_decode_type type)
{
    th_string_clear(output);

    th_err err = TH_ERR_OK;
    if (input.len == 0)
        return TH_ERR_OK;
    size_t i = 0;
    while (i < input.len) {
        size_t run = th_url_decode_literal_run(input, i, type);
        if (run > 0) {
            if ((err = th_string_append(output, th_str_substr(input, i, run))) != TH_ERR_OK)
                return err;
            i += run;
            continue;
        }
        char c;
        if ((err = th_url_decode_next(input, &i, &c, type)) != TH_ERR_OK) {
            return err;
        }
        if ((err = th_string_push_back(output, c)) != TH_ERR_OK) {
            return err;
        }
    }
    return TH_ERR_OK;
}
/* End of src/th_url_decode.c */
/* Start of src/th_ssl_smem_bio.c */
#if TH_WITH_SSL

#include <assert.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <string.h>


typedef struct th_static_bio_data {
    th_buf_vec buf;
    size_t max_len;
    size_t read_pos;
    size_t write_pos;
    int eof;
} th_static_bio_data;

TH_LOCAL(int)
th_smem_new(BIO* bio);

TH_LOCAL(int)
th_smem_free(BIO* bio);

TH_LOCAL(int)
th_smem_read(BIO* bio, char* out, int outl);

TH_LOCAL(int)
th_smem_write(BIO* bio, const char* in, int inl);

TH_LOCAL(long)
th_smem_ctrl(BIO* bio, int cmd, long num, void* ptr);

TH_PRIVATE(BIO_METHOD*)
th_smem_bio(th_ssl_context* ssl_context)
{
    if (ssl_context->smem_method == NULL) {
        ssl_context->smem_method = BIO_meth_new(BIO_TYPE_MEM, "th_smem");
        BIO_meth_set_write(ssl_context->smem_method, th_smem_write);
        BIO_meth_set_read(ssl_context->smem_method, th_smem_read);
        BIO_meth_set_ctrl(ssl_context->smem_method, th_smem_ctrl);
        BIO_meth_set_create(ssl_context->smem_method, th_smem_new);
        BIO_meth_set_destroy(ssl_context->smem_method, th_smem_free);
    }
    return ssl_context->smem_method;
}

TH_PRIVATE(void)
th_smem_bio_setup_buf(BIO* bio, th_allocator* allocator, size_t max_len)
{
    th_static_bio_data* data = BIO_get_data(bio);
    th_buf_vec_init(&data->buf, allocator);
    data->max_len = max_len;
    data->read_pos = 0;
    data->write_pos = 0;
    BIO_set_init(bio, 1);
}

TH_PRIVATE(size_t)
th_smem_ensure_buf_size(BIO* bio, size_t size)
{
    th_static_bio_data* data = BIO_get_data(bio);
    size = TH_MIN(size, data->max_len);
    if (th_buf_vec_size(&data->buf) < size)
        (void)th_buf_vec_resize(&data->buf, size);
    return th_buf_vec_size(&data->buf);
}

TH_PRIVATE(void)
th_smem_bio_get_rdata(BIO* bio, th_iov* buf)
{
    th_static_bio_data* bio_data = BIO_get_data(bio);
    buf->len = (bio_data->write_pos - bio_data->read_pos);
    buf->base = th_buf_vec_begin(&bio_data->buf) + bio_data->read_pos;
}

TH_PRIVATE(void)
th_smem_bio_get_wbuf(BIO* bio, th_iov* buf)
{
    th_static_bio_data* bio_data = BIO_get_data(bio);
    buf->len = th_buf_vec_size(&bio_data->buf) - bio_data->write_pos;
    buf->base = th_buf_vec_begin(&bio_data->buf) + bio_data->write_pos;
}

TH_PRIVATE(void)
th_smem_bio_inc_read_pos(BIO* bio, size_t len)
{
    th_static_bio_data* data = BIO_get_data(bio);
    data->read_pos += len;
    if (data->read_pos == data->write_pos) {
        data->read_pos = 0;
        data->write_pos = 0;
    }
}

TH_PRIVATE(void)
th_smem_bio_inc_write_pos(BIO* bio, size_t len)
{
    th_static_bio_data* data = BIO_get_data(bio);
    data->write_pos += len;
}

TH_PRIVATE(void)
th_smem_bio_set_eof(BIO* bio)
{
    th_static_bio_data* data = BIO_get_data(bio);
    data->eof = 1;
}

TH_LOCAL(int)
th_smem_new(BIO* bio)
{
    th_static_bio_data* data = OPENSSL_malloc(sizeof(th_static_bio_data));
    if (data == NULL)
        return 0;
    data->eof = 0;
    data->read_pos = 0;
    data->write_pos = 0;
    BIO_set_data(bio, data);
    return 1;
}

TH_LOCAL(int)
th_smem_free(BIO* bio)
{
    TH_ASSERT(bio);
    if (BIO_get_init(bio))
        th_buf_vec_deinit(&((th_static_bio_data*)BIO_get_data(bio))->buf);
    BIO_set_init(bio, 0);
    OPENSSL_free(BIO_get_data(bio));
    BIO_set_data(bio, NULL);
    return 1;
}

TH_LOCAL(int)
th_smem_read(BIO* bio, char* out, int outl)
{
    th_static_bio_data* data = BIO_get_data(bio);
    TH_ASSERT(data);
    TH_ASSERT(out);
    TH_ASSERT(outl > 0);
    size_t s = TH_MIN((size_t)outl, data->write_pos - data->read_pos);
    if (s == 0) {
        if (data->eof)
            return 0;
        BIO_set_retry_read(bio);
        return -1;
    }
    memcpy(out, th_buf_vec_begin(&data->buf) + data->read_pos, s);
    th_smem_bio_inc_read_pos(bio, s);
    return (int)s;
}

TH_LOCAL(int)
th_smem_write(BIO* bio, const char* in, int inlen)
{
    th_static_bio_data* data = BIO_get_data(bio);
    if (data->eof) // no more writing after eof
        return 0;
    TH_ASSERT(data);
    TH_ASSERT(in);
    TH_ASSERT(inlen > 0);
    size_t buflen = th_buf_vec_size(&data->buf);
    if (data->write_pos + (size_t)inlen > buflen)
        buflen = th_smem_ensure_buf_size(bio, data->write_pos + (size_t)inlen);
    size_t s = TH_MIN((size_t)inlen, buflen - data->write_pos);
    if (s == 0) {
        BIO_set_retry_write(bio);
        return -1;
    }
    memcpy(th_buf_vec_begin(&data->buf) + data->write_pos, in, s);
    data->write_pos += s;
    return (int)s;
}

TH_LOCAL(long)
th_smem_ctrl(BIO* bio, int cmd, long num, void* ptr)
{
    (void)num;
    th_static_bio_data* data = BIO_get_data(bio);
    TH_ASSERT(data);
    long ret = 1;

    switch (cmd) {
    case BIO_CTRL_RESET:
        data->read_pos = 0;
        data->write_pos = 0;
        data->eof = 0;
        break;
    case BIO_CTRL_EOF:
        ret = (data->eof && data->read_pos == data->write_pos);
        break;
    case BIO_CTRL_INFO: {
        ret = (long)th_buf_vec_size(&data->buf);
        if (ptr != NULL)
            *(void**)ptr = th_buf_vec_begin(&data->buf);
        break;
    }
    case BIO_CTRL_PENDING:
        ret = (long)(data->write_pos - data->read_pos);
        break;
    case BIO_CTRL_WPENDING:
        ret = 0;
        break;
    case BIO_CTRL_DUP:
    case BIO_CTRL_FLUSH:
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}
#endif
/* End of src/th_ssl_smem_bio.c */
/* Start of src/th_ssl_context.c */

#if TH_WITH_SSL


#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_context"

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, th_ssl_ops* ops, const char* key, const char* cert)
{
    context->ops = ops;
    context->smem_method = NULL;

    context->ctx = ops->ctx_new(ops);
    if (!context->ctx) {
        TH_LOG_FATAL("Failed to create SSL context");
        goto cleanup;
    }

    if (ops->ctx_use_certificate_chain_file(ops, context->ctx, cert) <= 0) {
        TH_LOG_FATAL("Failed to load certificate file");
        goto cleanup;
    }

    if (ops->ctx_use_private_key_file(ops, context->ctx, key) <= 0) {
        TH_LOG_FATAL("Failed to load private key file");
        goto cleanup;
    }

    if (!ops->ctx_set_min_proto_version(ops, context->ctx)) {
        TH_LOG_FATAL("Failed to set minimum protocol version");
        goto cleanup;
    }

    if (ops->ctx_set_cipher_list(ops, context->ctx, "MEDIUM:HIGH:!aNULL!MD5:!RC4!3DES") <= 0) {
        TH_LOG_FATAL("Failed to set cipher list");
        goto cleanup;
    }

    ops->ctx_set_session_cache_off(ops, context->ctx);
    return TH_ERR_OK;
cleanup:
    if (context->ctx) {
        ops->ctx_free(ops, context->ctx);
        context->ctx = NULL;
    }
    return TH_ERR_SSL(SSL_ERROR_SSL);
}

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context)
{
    if (context->smem_method)
        BIO_meth_free(context->smem_method);
    if (context->ctx)
        context->ops->ctx_free(context->ops, context->ctx);
}
#endif
/* End of src/th_ssl_context.c */
/* Start of src/th_ssl_error.c */

#if TH_WITH_SSL

#include <openssl/err.h>
#include <openssl/ssl.h>

TH_PRIVATE(const char*)
th_ssl_strerror(int code)
{
    switch (code) {
    case SSL_ERROR_NONE:
        return "Success";
        break;
    case SSL_ERROR_SSL:
        return "SSL library error, enable logging for more details";
        break;
    default:
        break;
    }
    return ERR_reason_error_string((unsigned long)code);
}

#endif // TH_WITH_SSL
/* End of src/th_ssl_error.c */
/* Start of src/th_ssl_ops.c */

#if TH_WITH_SSL


#include <openssl/err.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl"

/** th_ssl_ops_os_log_error_stack
 * @brief Drains and logs OpenSSL's per-thread error queue. Call right
 * after a real OpenSSL call reports failure — WANT_READ/WANT_WRITE never
 * push queue entries, so calling this unconditionally on ret<=0/NULL is
 * safe and simply logs nothing for those.
 */
TH_LOCAL(void)
th_ssl_ops_os_log_error_stack(void)
{
    unsigned long code;
    while ((code = ERR_get_error())) {
        TH_LOG_ERROR("%s", ERR_reason_error_string(code));
    }
}

TH_LOCAL(SSL_CTX*)
th_ssl_ops_os_ctx_new(void* self)
{
    (void)self;
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx)
        th_ssl_ops_os_log_error_stack();
    return ctx;
}

TH_LOCAL(void)
th_ssl_ops_os_ctx_free(void* self, SSL_CTX* ctx)
{
    (void)self;
    SSL_CTX_free(ctx);
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_use_certificate_chain_file(void* self, SSL_CTX* ctx, const char* cert)
{
    (void)self;
    int ret = SSL_CTX_use_certificate_chain_file(ctx, cert);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_use_private_key_file(void* self, SSL_CTX* ctx, const char* key)
{
    (void)self;
    int ret = SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_set_min_proto_version(void* self, SSL_CTX* ctx)
{
    (void)self;
    int ret = SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION) != 0;
    if (!ret)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_ctx_set_cipher_list(void* self, SSL_CTX* ctx, const char* ciphers)
{
    (void)self;
    int ret = SSL_CTX_set_cipher_list(ctx, ciphers);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(void)
th_ssl_ops_os_ctx_set_session_cache_off(void* self, SSL_CTX* ctx)
{
    (void)self;
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
}

TH_LOCAL(SSL*)
th_ssl_ops_os_new_ssl(void* self, SSL_CTX* ctx)
{
    (void)self;
    SSL* ssl = SSL_new(ctx);
    if (!ssl)
        th_ssl_ops_os_log_error_stack();
    return ssl;
}

TH_LOCAL(void)
th_ssl_ops_os_free_ssl(void* self, SSL* ssl)
{
    (void)self;
    SSL_free(ssl);
}

TH_LOCAL(void)
th_ssl_ops_os_set_bio(void* self, SSL* ssl, BIO* rbio, BIO* wbio)
{
    (void)self;
    SSL_set_bio(ssl, rbio, wbio);
}

TH_LOCAL(void)
th_ssl_ops_os_set_accept_state(void* self, SSL* ssl)
{
    (void)self;
    SSL_set_accept_state(ssl);
}

TH_LOCAL(void)
th_ssl_ops_os_set_partial_write(void* self, SSL* ssl)
{
    (void)self;
    SSL_set_mode(ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
}

TH_LOCAL(int)
th_ssl_ops_os_do_handshake(void* self, SSL* ssl)
{
    (void)self;
    int ret = SSL_do_handshake(ssl);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_read(void* self, SSL* ssl, void* buf, int len)
{
    (void)self;
    int ret = SSL_read(ssl, buf, len);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_write(void* self, SSL* ssl, const void* buf, int len)
{
    (void)self;
    int ret = SSL_write(ssl, buf, len);
    if (ret <= 0)
        th_ssl_ops_os_log_error_stack();
    return ret;
}

TH_LOCAL(int)
th_ssl_ops_os_get_error(void* self, SSL* ssl, int ret)
{
    (void)self;
    return SSL_get_error(ssl, ret);
}

TH_PRIVATE(th_ssl_ops*)
th_ssl_ops_os(void)
{
    static th_ssl_ops ops = {
        .ctx_new = th_ssl_ops_os_ctx_new,
        .ctx_free = th_ssl_ops_os_ctx_free,
        .ctx_use_certificate_chain_file = th_ssl_ops_os_ctx_use_certificate_chain_file,
        .ctx_use_private_key_file = th_ssl_ops_os_ctx_use_private_key_file,
        .ctx_set_min_proto_version = th_ssl_ops_os_ctx_set_min_proto_version,
        .ctx_set_cipher_list = th_ssl_ops_os_ctx_set_cipher_list,
        .ctx_set_session_cache_off = th_ssl_ops_os_ctx_set_session_cache_off,
        .new_ssl = th_ssl_ops_os_new_ssl,
        .free_ssl = th_ssl_ops_os_free_ssl,
        .set_bio = th_ssl_ops_os_set_bio,
        .set_accept_state = th_ssl_ops_os_set_accept_state,
        .set_partial_write = th_ssl_ops_os_set_partial_write,
        .do_handshake = th_ssl_ops_os_do_handshake,
        .read = th_ssl_ops_os_read,
        .write = th_ssl_ops_os_write,
        .get_error = th_ssl_ops_os_get_error,
    };
    return &ops;
}

#endif
/* End of src/th_ssl_ops.c */
/* Start of src/th_ssl_session.c */

#if TH_WITH_SSL


#include <openssl/err.h>

TH_PRIVATE(th_err)
th_ssl_session_init(th_ssl_session* session, th_ssl_context* context, th_ssl_ops* ops, th_allocator* allocator)
{
    session->ops = ops;
    th_err err = TH_ERR_OK;
    session->ssl = session->ops->new_ssl(session->ops, context->ctx);
    if (!session->ssl) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_none;
    }
    session->wbio = BIO_new(th_smem_bio(context));
    if (!session->wbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_ssl;
    }
    session->rbio = BIO_new(th_smem_bio(context));
    if (!session->rbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_wbio;
    }
    th_smem_bio_setup_buf(session->wbio, allocator, TH_CONFIG_MAX_SSL_WRITE_BUF_LEN);
    th_smem_bio_setup_buf(session->rbio, allocator, TH_CONFIG_MAX_SSL_READ_BUF_LEN);
    session->ops->set_bio(session->ops, session->ssl, session->rbio, session->wbio);
    session->ops->set_accept_state(session->ops, session->ssl);
    session->ops->set_partial_write(session->ops, session->ssl);
    return TH_ERR_OK;
cleanup_wbio:
    BIO_free(session->wbio);
cleanup_ssl:
    session->ops->free_ssl(session->ops, session->ssl);
cleanup_none:
    return err;
}

TH_PRIVATE(void)
th_ssl_session_deinit(th_ssl_session* session)
{
    /* SSL_set_bio transferred ownership of rbio/wbio to session->ssl;
     * SSL_free (behind free_ssl) frees them, so don't BIO_free here. */
    session->ops->free_ssl(session->ops, session->ssl);
}

TH_LOCAL(th_ssl_result)
th_ssl_session_classify(th_ssl_session* session, int ret)
{
    if (BIO_pending(session->wbio) > 0)
        return TH_SSL_WANT_WRITE;
    int code = session->ops->get_error(session->ops, session->ssl, ret);
    if (code == SSL_ERROR_WANT_READ)
        return TH_SSL_WANT_READ;
    if (code == SSL_ERROR_WANT_WRITE)
        return TH_SSL_WANT_WRITE;
    return TH_SSL_ERROR;
}

TH_LOCAL(th_err)
th_ssl_session_error(th_ssl_session* session, int ret)
{
    int code = session->ops->get_error(session->ops, session->ssl, ret);
    if (code == SSL_ERROR_ZERO_RETURN)
        return TH_ERR_EOF;
    return TH_ERR_SSL(code);
}

TH_PRIVATE(th_ssl_result)
th_ssl_session_handshake(th_ssl_session* session, th_err* err)
{
    int ret = session->ops->do_handshake(session->ops, session->ssl);
    if (ret == 1) {
        *err = TH_ERR_OK;
        return BIO_pending(session->wbio) > 0 ? TH_SSL_WANT_WRITE : TH_SSL_DONE;
    }
    th_ssl_result result = th_ssl_session_classify(session, ret);
    if (result == TH_SSL_ERROR) {
        *err = th_ssl_session_error(session, ret);
        return result;
    }
    *err = TH_ERR_OK;
    return result;
}

TH_PRIVATE(th_ssl_result)
th_ssl_session_read(th_ssl_session* session, void* buf, size_t len, size_t* out, th_err* err)
{
    int ret = session->ops->read(session->ops, session->ssl, buf, (int)len);
    if (ret > 0) {
        *out = (size_t)ret;
        *err = TH_ERR_OK;
        return BIO_pending(session->wbio) > 0 ? TH_SSL_WANT_WRITE : TH_SSL_DONE;
    }
    *out = 0;
    th_ssl_result result = th_ssl_session_classify(session, ret);
    if (result == TH_SSL_ERROR) {
        *err = th_ssl_session_error(session, ret);
        return result;
    }
    *err = TH_ERR_OK;
    return result;
}

TH_PRIVATE(th_ssl_result)
th_ssl_session_write(th_ssl_session* session, const void* buf, size_t len, size_t* out, th_err* err)
{
    int ret = session->ops->write(session->ops, session->ssl, buf, (int)len);
    if (ret > 0) {
        *out = (size_t)ret;
        *err = TH_ERR_OK;
        return TH_SSL_WANT_WRITE;
    }
    *out = 0;
    int code = session->ops->get_error(session->ops, session->ssl, ret);
    if (code == SSL_ERROR_WANT_READ) {
        *err = TH_ERR_OK;
        return TH_SSL_WANT_READ;
    }
    *err = th_ssl_session_error(session, ret);
    return TH_SSL_ERROR;
}

TH_PRIVATE(void)
th_ssl_session_get_ciphertext_out(th_ssl_session* session, th_iov* iov)
{
    th_smem_bio_get_rdata(session->wbio, iov);
}

TH_PRIVATE(void)
th_ssl_session_consume_ciphertext_out(th_ssl_session* session, size_t n)
{
    th_smem_bio_inc_read_pos(session->wbio, n);
}

TH_PRIVATE(void)
th_ssl_session_get_ciphertext_in_buf(th_ssl_session* session, th_iov* iov)
{
    th_smem_ensure_buf_size(session->rbio, TH_CONFIG_MAX_SSL_READ_BUF_LEN);
    th_smem_bio_get_wbuf(session->rbio, iov);
}

TH_PRIVATE(void)
th_ssl_session_fed_ciphertext_in(th_ssl_session* session, size_t n)
{
    if (n == 0) {
        th_smem_bio_set_eof(session->rbio);
        return;
    }
    th_smem_bio_inc_write_pos(session->rbio, n);
}

#endif
/* End of src/th_ssl_session.c */
/* Start of src/th_ssl_io.c */

#if TH_WITH_SSL


TH_LOCAL(bool)
th_ssl_io_op_is_retryable(th_err err)
{
    return err == TH_ERR_SYSTEM(TH_EAGAIN)
           || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK);
}

TH_LOCAL(void)
th_ssl_io_op_finalize(th_ssl_io_op* op)
{
    op->callback(op->user_data, op->result, op->err);
}

TH_LOCAL(void)
th_ssl_io_op_complete(th_ssl_io_op* op, size_t result, th_err err)
{
    op->result = result;
    op->err = err;
    th_op_set_flags(&op->base, TH_OP_COMPLETED);
    th_socket_post(op->socket, &op->base.base);
}

/** th_ssl_io_op_step
 * @brief Calls the session step for op->kind once. Returns TH_ERR_OK when
 * done, TH_ERR_SYSTEM(TH_EAGAIN) when a raw ciphertext shuttle (recv or
 * send on op->socket) must run before retrying — op->shuttling_write says
 * which direction — or any other th_err on failure.
 */
TH_LOCAL(th_err)
th_ssl_io_op_step(th_ssl_io_op* op)
{
    th_err err = TH_ERR_OK;
    th_ssl_result result;
    switch (op->kind) {
    case TH_SSL_IO_HANDSHAKE:
        result = th_ssl_session_handshake(op->session, &err);
        break;
    case TH_SSL_IO_READ: {
        size_t out = 0;
        result = th_ssl_session_read(op->session, op->buf, op->len, &out, &err);
        op->result = out;
        break;
    }
    case TH_SSL_IO_WRITE: {
        size_t out = 0;
        result = th_ssl_session_write(op->session, op->buf, op->len, &out, &err);
        op->result = out;
        break;
    }
    default:
        TH_ASSERT(0 && "Invalid th_ssl_io_kind");
        return TH_ERR_SSL(0);
    }
    switch (result) {
    case TH_SSL_DONE:
        return TH_ERR_OK;
    case TH_SSL_WANT_READ:
        op->shuttling_write = false;
        return TH_ERR_SYSTEM(TH_EAGAIN);
    case TH_SSL_WANT_WRITE:
        op->shuttling_write = true;
        return TH_ERR_SYSTEM(TH_EAGAIN);
    default:
        return err;
    }
}

/** th_ssl_io_op_shuttle
 * @brief Drains pending ciphertext to op->socket (shuttling_write) or
 * reads more ciphertext in from it, once. Returns TH_ERR_OK when that
 * raw transfer completed (retry the session step next), or propagates
 * TH_EAGAIN/an error from the raw socket call.
 */
TH_LOCAL(th_err)
th_ssl_io_op_shuttle(th_ssl_io_op* op)
{
    th_iov iov;
    size_t result = 0;
    th_err err;
    if (op->shuttling_write) {
        th_ssl_session_get_ciphertext_out(op->session, &iov);
        if (iov.len == 0)
            return TH_ERR_OK;
        err = th_socket_sendvec(op->socket, &iov, 1, &result);
        if (err != TH_ERR_OK)
            return err;
        th_ssl_session_consume_ciphertext_out(op->session, result);
        return TH_ERR_OK;
    }
    th_ssl_session_get_ciphertext_in_buf(op->session, &iov);
    err = th_socket_recv(op->socket, iov.base, iov.len, &result);
    if (err != TH_ERR_OK)
        return err;
    th_ssl_session_fed_ciphertext_in(op->session, result);
    return TH_ERR_OK;
}

/** th_ssl_io_op_perform
 * @brief Alternates session steps with raw ciphertext shuttles until the
 * step is done/errors, or (for READ/WRITE) has made plaintext progress
 * and its last-requested shuttle has drained/fed — matching TCP recv/send
 * semantics where a short transfer is a valid completion, not something
 * to retry into the same buffer. op->draining marks that plaintext
 * progress already happened and only the shuttle remains, so a step that
 * gets interrupted by EAGAIN mid-shuttle resumes straight into the
 * shuttle on the next call instead of re-invoking SSL_read/SSL_write and
 * overwriting op->result.
 */
TH_LOCAL(th_err)
th_ssl_io_op_perform(th_ssl_io_op* op)
{
    th_op_clear_flags(&op->base, TH_OP_IMMEDIATE);
    for (;;) {
        if (!op->draining) {
            th_err err = th_ssl_io_op_step(op);
            if (err != TH_ERR_SYSTEM(TH_EAGAIN))
                return err;
            if (op->kind != TH_SSL_IO_HANDSHAKE && op->result > 0)
                op->draining = true;
        }
        th_err err = th_ssl_io_op_shuttle(op);
        if (err != TH_ERR_OK)
            return err;
        if (op->draining)
            return TH_ERR_OK;
    }
}

TH_LOCAL(void)
th_ssl_io_op_fn(void* self)
{
    th_ssl_io_op* op = self;
    if (th_op_get_flags(&op->base) & TH_OP_COMPLETED) {
        th_ssl_io_op_finalize(op);
        return;
    }
    op->base.type = op->shuttling_write ? TH_OP_WRITE : TH_OP_READ;
    th_err err = th_ssl_io_op_perform(op);
    if (th_ssl_io_op_is_retryable(err)) {
        op->base.type = op->shuttling_write ? TH_OP_WRITE : TH_OP_READ;
        err = th_socket_submit(op->socket, &op->base);
        if (err == TH_ERR_OK)
            return;
    }
    th_ssl_io_op_complete(op, op->result, err);
}

TH_LOCAL(void)
th_ssl_io_op_abort(void* self, th_err err)
{
    th_ssl_io_op_complete(self, 0, err);
}

TH_LOCAL(void)
th_ssl_io_op_init(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, th_ssl_io_kind kind, th_ssl_io_cb callback, void* user_data)
{
    th_op_init(&op->base, TH_OP_READ, th_ssl_io_op_fn, th_ssl_io_op_abort);
    op->socket = socket;
    op->session = session;
    op->kind = kind;
    op->buf = NULL;
    op->len = 0;
    op->result = 0;
    op->shuttling_write = false;
    op->draining = false;
    op->callback = callback;
    op->user_data = user_data;
    op->err = TH_ERR_OK;
}

TH_PRIVATE(void)
th_ssl_io_op_init_handshake(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, th_ssl_io_cb callback, void* user_data)
{
    th_ssl_io_op_init(op, socket, session, TH_SSL_IO_HANDSHAKE, callback, user_data);
}

TH_PRIVATE(void)
th_ssl_io_op_init_read(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, void* buf, size_t len, th_ssl_io_cb callback, void* user_data)
{
    th_ssl_io_op_init(op, socket, session, TH_SSL_IO_READ, callback, user_data);
    op->buf = buf;
    op->len = len;
}

TH_PRIVATE(void)
th_ssl_io_op_init_write(th_ssl_io_op* op, th_socket* socket, th_ssl_session* session, const void* buf, size_t len, th_ssl_io_cb callback, void* user_data)
{
    th_ssl_io_op_init(op, socket, session, TH_SSL_IO_WRITE, callback, user_data);
    op->buf = (void*)buf;
    op->len = len;
}

#endif
/* End of src/th_ssl_io.c */
/* Start of src/th_ssl_recv.c */

#if TH_WITH_SSL

TH_LOCAL(void)
th_ssl_recv_op_finalize(th_ssl_recv_op* op, th_err err)
{
    op->callback(op->user_data, op->pos, err);
}

TH_LOCAL(void)
th_ssl_recv_op_start(th_ssl_recv_op* op);

TH_LOCAL(void)
th_ssl_recv_op_io_complete(void* user_data, size_t size, th_err err)
{
    th_ssl_recv_op* op = user_data;
    if (err != TH_ERR_OK) {
        th_ssl_recv_op_finalize(op, err);
        return;
    }
    op->pos += size;
    if (!op->exact || op->pos == op->len) {
        th_ssl_recv_op_finalize(op, TH_ERR_OK);
        return;
    }
    th_ssl_recv_op_start(op);
}

TH_LOCAL(void)
th_ssl_recv_op_start(th_ssl_recv_op* op)
{
    th_ssl_io_op_init_read(&op->io, op->socket, op->session,
                           (char*)op->addr + op->pos, op->len - op->pos,
                           th_ssl_recv_op_io_complete, op);
    th_op_perform(&op->io.base);
}

TH_PRIVATE(void)
th_ssl_recv_op_init(th_ssl_recv_op* op, th_socket* socket, th_ssl_session* session, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    op->socket = socket;
    op->session = session;
    op->addr = addr;
    op->len = len;
    op->pos = 0;
    op->exact = exact;
    op->callback = callback;
    op->user_data = user_data;
    th_ssl_recv_op_start(op);
}

#endif
/* End of src/th_ssl_recv.c */
/* Start of src/th_ssl_send.c */

#if TH_WITH_SSL


#include <string.h>

TH_LOCAL(void)
th_ssl_send_op_finalize(th_ssl_send_op* op, th_err err)
{
    op->callback(op->user_data, op->pos, err);
}

TH_LOCAL(void)
th_ssl_send_op_start(th_ssl_send_op* op);

TH_LOCAL(void)
th_ssl_send_op_io_complete(void* user_data, size_t size, th_err err)
{
    th_ssl_send_op* op = user_data;
    if (err != TH_ERR_OK) {
        th_ssl_send_op_finalize(op, err);
        return;
    }
    op->pos += size;
    th_ssl_send_op_start(op);
}

/** th_ssl_send_op_fill
 * @brief Consumes iov (and then file, if the header didn't fill the
 * chunk) into op->buffer. Returns the number of bytes filled.
 */
TH_LOCAL(th_err)
th_ssl_send_op_fill(th_ssl_send_op* op, size_t* out)
{
    size_t bufpos = 0;
    while (op->iovcnt > 0 && bufpos < TH_SSL_SEND_CHUNK_LEN) {
        size_t avail = TH_SSL_SEND_CHUNK_LEN - bufpos;
        size_t to_copy = TH_MIN(avail, op->iov[0].len);
        memcpy(op->buffer + bufpos, op->iov[0].base, to_copy);
        bufpos += to_copy;
        th_iov_consume(&op->iov, &op->iovcnt, to_copy);
    }
    if (op->file && bufpos < TH_SSL_SEND_CHUNK_LEN) {
        size_t remaining = op->len - op->file_pos;
        size_t readlen = TH_MIN(TH_SSL_SEND_CHUNK_LEN - bufpos, remaining);
        if (readlen > 0) {
            size_t bytes_read = 0;
            th_err err = th_file_read(op->file, op->buffer + bufpos, readlen, op->offset + op->file_pos, &bytes_read);
            if (err != TH_ERR_OK && bufpos == 0)
                return err;
            op->file_pos += bytes_read;
            bufpos += bytes_read;
        }
    }
    *out = bufpos;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_send_op_start(th_ssl_send_op* op)
{
    size_t chunk_len = 0;
    th_err err = th_ssl_send_op_fill(op, &chunk_len);
    if (err != TH_ERR_OK) {
        th_ssl_send_op_finalize(op, err);
        return;
    }
    if (chunk_len == 0) {
        th_ssl_send_op_finalize(op, TH_ERR_OK);
        return;
    }
    th_ssl_io_op_init_write(&op->io, op->socket, op->session, op->buffer, chunk_len, th_ssl_send_op_io_complete, op);
    th_op_perform(&op->io.base);
}

TH_PRIVATE(void)
th_ssl_send_op_init(th_ssl_send_op* op, th_socket* socket, th_ssl_session* session,
                    th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len,
                    th_send_cb callback, void* user_data)
{
    op->socket = socket;
    op->session = session;
    op->iov = iov;
    op->iovcnt = iovcnt;
    op->file = file;
    op->offset = offset;
    op->len = len;
    op->file_pos = 0;
    op->pos = 0;
    op->callback = callback;
    op->user_data = user_data;
    th_ssl_send_op_start(op);
}

#endif
/* End of src/th_ssl_send.c */
/* Start of src/th_ssl_conn.c */

#if TH_WITH_SSL


#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_conn"

/** th_ssl_conn_op
 * @brief At most one recv and one send are ever in flight at a time on
 * an HTTP connection (request read, then response write), so a single
 * union covers every th_conn_methods.recv/send call without allocating.
 */
typedef union th_ssl_conn_op {
    th_ssl_recv_op recv;
    th_ssl_send_op send;
} th_ssl_conn_op;

typedef struct th_ssl_conn {
    th_conn_observable base;
    th_socket socket;
    th_address addr;
    th_ssl_session session;
    th_ssl_io_op handshake_op;
    th_ssl_conn_op recv_op;
    th_ssl_conn_op send_op;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
} th_ssl_conn;

TH_LOCAL(th_address*)
th_ssl_conn_get_address(void* self)
{
    th_ssl_conn* conn = self;
    return &conn->addr;
}

TH_LOCAL(th_socket*)
th_ssl_conn_get_socket(void* self)
{
    th_ssl_conn* conn = self;
    return &conn->socket;
}

TH_LOCAL(void)
th_ssl_conn_handshake_complete(void* user_data, size_t size, th_err err)
{
    (void)size;
    th_ssl_conn* conn = user_data;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p: SSL handshake failed: %s", (void*)conn, th_strerror(err));
        th_conn_destroy((th_conn*)conn);
        return;
    }
    TH_LOG_TRACE("%p: SSL handshake done", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_ssl_conn_start(void* self)
{
    th_ssl_conn* conn = self;
    TH_LOG_TRACE("%p: Starting SSL handshake", conn);
    th_ssl_io_op_init_handshake(&conn->handshake_op, &conn->socket, &conn->session,
                                th_ssl_conn_handshake_complete, conn);
    th_op_perform(&conn->handshake_op.base);
}

TH_LOCAL(void)
th_ssl_conn_recv(void* self, void* addr, size_t len, bool exact, th_recv_cb callback, void* user_data)
{
    th_ssl_conn* conn = self;
    th_ssl_recv_op_init(&conn->recv_op.recv, &conn->socket, &conn->session, addr, len, exact, callback, user_data);
}

TH_LOCAL(void)
th_ssl_conn_send(void* self, th_iov* iov, size_t iovcnt, th_file* file, size_t offset, size_t len, th_send_cb callback, void* user_data)
{
    th_ssl_conn* conn = self;
    th_ssl_send_op_init(&conn->send_op.send, &conn->socket, &conn->session, iov, iovcnt, file, offset, len, callback, user_data);
}

TH_LOCAL(void)
th_ssl_conn_cancel(void* self)
{
    th_ssl_conn* conn = self;
    th_socket_cancel(&conn->socket);
}

TH_LOCAL(void)
th_ssl_conn_free(void* self)
{
    th_ssl_conn* conn = self;
    TH_LOG_TRACE("%p: Destroying connection", conn);
    th_ssl_session_deinit(&conn->session);
    th_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

static const th_conn_methods th_ssl_conn_methods = {
    .get_address = th_ssl_conn_get_address,
    .get_socket = th_ssl_conn_get_socket,
    .start = th_ssl_conn_start,
    .recv = th_ssl_conn_recv,
    .send = th_ssl_conn_send,
    .cancel = th_ssl_conn_cancel,
    .destroy = th_conn_observable_destroy,
};

TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_socket* socket, th_ssl_context* ssl_context, th_ssl_ops* ssl_ops,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_ssl_conn* conn = th_allocator_alloc(allocator, sizeof(th_ssl_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_session_init(&conn->session, ssl_context, ssl_ops, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, conn);
        return err;
    }
    th_conn_observable_init(&conn->base, &th_ssl_conn_methods, th_ssl_conn_free, observer);
    conn->upgrader = upgrader;
    conn->allocator = allocator;
    conn->socket = *socket;
    th_address_init(&conn->addr);
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}

#endif
/* End of src/th_ssl_conn.c */
