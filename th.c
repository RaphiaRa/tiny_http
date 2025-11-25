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

#ifndef TH_CONFIG_OS_MOCK
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
#elif defined(TH_CONFIG_OS_MOCK)
    (void)errc;
    return "mock error";
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
#elif defined(TH_CONFIG_OS_MOCK)
#define TH_ENOENT 1
#define TH_EINTR 2
#define TH_EIO 3
#define TH_EBUSY 4
#define TH_EAGAIN 5
#define TH_EWOULDBLOCK 6
#define TH_ENOMEM 7
#define TH_ENOSYS 8
#define TH_ETIMEDOUT 9
#define TH_ECANCELED 10
#define TH_EBADF 11
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
/* Start of th_io_op.h */



#include <stddef.h>

TH_PRIVATE(th_err)
th_io_op_read(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_readv(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_write(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_writev(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_send(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_sendv(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_accept(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_sendfile(void* self, size_t* result) TH_MAYBE_UNUSED;

/* End of th_io_op.h */
/* Start of th_io_op_bsd.h */


#if defined(TH_CONFIG_WITH_BSD_SENDFILE)
TH_PRIVATE(th_err)
th_io_op_bsd_sendfile(void* self, size_t* result) TH_MAYBE_UNUSED;
#endif

/* End of th_io_op_bsd.h */
/* Start of th_io_op_linux.h */



#if defined(TH_CONFIG_WITH_LINUX_SENDFILE)
TH_PRIVATE(th_err)
th_io_op_linux_sendfile(void* self, size_t* result) TH_MAYBE_UNUSED;
#endif

/* End of th_io_op_linux.h */
/* Start of th_io_op_mock.h */



#if defined(TH_CONFIG_OS_MOCK)

TH_PRIVATE(th_err)
th_io_op_mock_read(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_readv(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_write(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_writev(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_send(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_sendv(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_accept(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_mock_sendfile(void* self, size_t* result);

#endif

/* End of th_io_op_mock.h */
/* Start of th_io_op_posix.h */



#if defined(TH_CONFIG_OS_POSIX)

TH_PRIVATE(th_err)
th_io_op_posix_read(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_readv(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_write(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_writev(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_send(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_sendv(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_accept(void* self, size_t* result);

TH_PRIVATE(th_err)
th_io_op_posix_sendfile_mmap(void* self, size_t* result) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_io_op_posix_sendfile_buffered(void* self, size_t* result) TH_MAYBE_UNUSED;

#endif
/* End of th_io_op_posix.h */
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
th_log_printf(int level, const char* fmt, ...) TH_MAYBE_UNUSED;

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
/* th_pool_allocator begin */
typedef struct th_pool_allocator_node th_pool_allocator_node;
struct th_pool_allocator_node {
    th_pool_allocator_node* next;
    th_pool_allocator_node* prev;
};
TH_DEFINE_LIST(th_pool_allocator_list, th_pool_allocator_node, prev, next)
typedef struct th_pool_allocator {
    th_allocator base;
    th_pool_allocator_list free_list;
    th_pool_allocator_list used_list;
    th_allocator* allocator;
    size_t block_size;
} th_pool_allocator;

TH_PRIVATE(void)
th_pool_allocator_init(th_pool_allocator* pool, th_allocator* allocator, size_t block_size);

TH_PRIVATE(void)
th_pool_allocator_deinit(th_pool_allocator* pool);

/** Generic object pool allocator.
 * The pool allocator is a allocator that allocates objects from a pool of fixed-size blocks.
 * It can be used with any object that has a next and prev pointer.
 */
#define TH_DEFINE_OBJ_POOL_ALLOCATOR(NAME, T, PREV, NEXT)                                         \
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
/* Start of th_string.h */

#include <stdint.h>
#include <string.h>



extern size_t th_string_npos;

typedef struct th_string {
    const char* ptr;
    size_t len;
} th_string;

/** th_string_make
 * @brief Helper function to create a th_string from a pointer and a length.
 */
TH_INLINE(th_string)
th_string_make(const char* ptr, size_t len)
{
    return (th_string){ptr, len};
}

/** th_string_make_empty
 * @brief Helper function to create an empty th_string.
 */
TH_INLINE(th_string)
th_string_make_empty(void)
{
    return (th_string){"", 0};
}

/** th_string_from_cstr
 * @brief Helper function to create a th_string from a null-terminated string.
 */
TH_INLINE(th_string)
th_string_from_cstr(const char* str)
{
    return th_string_make(str, strlen(str));
}

/** th_string_eq
 * @brief Helper function to compare two th_strings.
 * @return 1 if the strings are equal, 0 otherwise.
 */
TH_PRIVATE(bool)
th_string_eq(th_string a, th_string b);

/** th_string_empty
 * @brief Helper function to check if a th_string is empty.
 * @return true if the string is empty, false otherwise.
 */
TH_INLINE(bool)
th_string_empty(th_string str)
{
    return str.len == 0;
}

/** TH_STRING_INIT
 * @brief Helper macro to initialize a th_string from string literal.
 */
#define TH_STRING_INIT(str) {"" str, sizeof(str) - 1}

/** TH_STRING
 * @brief Helper macro to create a th_string compound literal from a string literal.
 */
#define TH_STRING(str) ((th_string){"" str, sizeof(str) - 1})

/** TH_STRING_EQ
 * @brief Helper macro to compare a th_string with a string literal.
 */
#define TH_STRING_EQ(str, cmp) (th_string_eq(str, TH_STRING(cmp)))

TH_PRIVATE(bool)
th_string_is_uint(th_string str);

TH_PRIVATE(th_err)
th_string_to_uint(th_string str, unsigned int* out);

TH_PRIVATE(size_t)
th_string_find_first(th_string str, size_t start, char c);

TH_PRIVATE(size_t)
th_string_find_first_not(th_string str, size_t start, char c);

TH_PRIVATE(size_t)
th_string_find_first_of(th_string str, size_t start, const char* chars);

TH_PRIVATE(size_t)
th_string_find_last(th_string str, size_t start, char c);

/** th_string_substr
 * @brief Returns a substring of a string.
 * If len == th_string_npos, the substring will go to the end of the string.
 * If start > len, an empty string is returned (ptr = str.ptr + str.len, len = 0).
 */
TH_PRIVATE(th_string)
th_string_substr(th_string str, size_t start, size_t len);

/** th_string_trim
 * @brief Removes leading and trailing whitespace from a string.
 * This doesn't modify the original string, just returns a new view of it.
 * @param str The string to trim.
 * @return A new string view with leading and trailing whitespace removed.
 */
TH_PRIVATE(th_string)
th_string_trim(th_string str);

TH_PRIVATE(size_t)
th_string_hash(th_string str);

/* End of th_string.h */
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
/* Start of th_heap_string.h */


typedef struct th_detail_large_string {
    size_t capacity;
    size_t len;
    char* ptr;
    th_allocator* allocator;
} th_detail_large_string;

#define TH_HEAP_STRING_SMALL_BUF_LEN (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 1)
#define TH_HEAP_STRING_SMALL_MAX_LEN (TH_HEAP_STRING_SMALL_BUF_LEN - 1)
typedef struct th_detail_small_string {
    unsigned char small : 1;
    unsigned char len : 7;
    char buf[TH_HEAP_STRING_SMALL_BUF_LEN];
    th_allocator* allocator;
} th_detail_small_string;

typedef struct th_heap_string {
    union {
        th_detail_small_string small;
        th_detail_large_string large;
    } impl;
} th_heap_string;

TH_PRIVATE(void)
th_heap_string_init(th_heap_string* self, th_allocator* allocator);

TH_PRIVATE(th_err)
th_heap_string_init_with(th_heap_string* self, th_string str, th_allocator* allocator);

TH_PRIVATE(th_err)
th_heap_string_set(th_heap_string* self, th_string str);

TH_PRIVATE(th_err)
th_heap_string_append(th_heap_string* self, th_string str);

TH_PRIVATE(th_err)
th_heap_string_append_cstr(th_heap_string* self, const char* str);

TH_PRIVATE(th_err)
th_heap_string_push_back(th_heap_string* self, char c);

TH_PRIVATE(th_err)
th_heap_string_resize(th_heap_string* self, size_t new_len, char fill);

TH_PRIVATE(th_string)
th_heap_string_view(const th_heap_string* self);

TH_PRIVATE(char*)
th_heap_string_at(th_heap_string* self, size_t index);

TH_PRIVATE(const char*)
th_heap_string_data(const th_heap_string* self);

TH_PRIVATE(size_t)
th_heap_string_len(const th_heap_string* self);

TH_PRIVATE(void)
th_heap_string_deinit(th_heap_string* self);

TH_PRIVATE(void)
th_heap_string_clear(th_heap_string* self);

TH_PRIVATE(void)
th_heap_string_to_lower(th_heap_string* self);

TH_PRIVATE(bool)
th_heap_string_eq(const th_heap_string* self, th_string other);

//TH_PRIVATE(uint32_t)
//th_heap_string_hash(const th_heap_string* self);

TH_DEFINE_VEC(th_heap_string_vec, th_heap_string, th_heap_string_deinit)

/* End of th_heap_string.h */
/* Start of th_dir.h */



typedef struct th_dir {
    th_allocator* allocator;
    th_heap_string path;
    int fd;
} th_dir;

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_allocator* allocator);

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_string path);

TH_PRIVATE(th_string)
th_dir_get_path(th_dir* dir);

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir);

/* End of th_dir.h */
/* Start of th_file.h */



typedef struct th_file_mmap {
    void* addr;
    size_t offset;
    size_t len;
} th_file_mmap;

typedef struct th_file {
    int fd;
    size_t size;
    th_file_mmap view;
} th_file;

TH_PRIVATE(void)
th_file_init(th_file* stream);

typedef struct th_open_opt {
    bool read;
    bool write;
    bool create;
    bool truncate;
} th_open_opt;

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, th_string path, th_open_opt opt);

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_file_write(th_file* stream, const void* addr, size_t len, size_t offset, size_t* written) TH_MAYBE_UNUSED;

typedef struct th_fileview {
    void* ptr;
    size_t len;
} th_fileview;

TH_PRIVATE(th_err)
th_file_get_view(th_file* stream, th_fileview* view, size_t offset, size_t len);

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

    /** destroy
     * @brief The destructor for the th_task.
     * Can be NULL if the th_task does not need to be destroyed.
     */
    void (*destroy)(void* self);

    /** This is used internally by the runner. */
    struct th_task* next;
} th_task;

/** th_task_init
 * @brief Initializes a task.
 */
TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void* self), void (*destroy)(void* self));

/** th_task complete
 * @brief Runs the task.
 */
TH_PRIVATE(void)
th_task_complete(th_task* task);

/** th_task_destroy
 * @brief Destroys the task, if the task has a destroy function.
 */
TH_PRIVATE(void)
th_task_destroy(th_task* task);

/* th_task_queue declarations begin */

#ifndef TH_TASK_QUEUE
#define TH_TASK_QUEUE
TH_DEFINE_QUEUE(th_task_queue, th_task)
#endif

/* th_task_queue declarations end */

/* End of th_task.h */
/* Start of th_io_task.h */



/** th_io_handler
 *@brief I/O operation completion handler, inherits from th_task.
 * and contains the result of the operation.
 */
typedef struct th_io_handler {
    th_task base;
    void (*fn)(void* self, size_t result, th_err err);
    size_t result;
    th_err err;
} th_io_handler;

TH_PRIVATE(void)
th_io_handler_fn(void* self);

TH_INLINE(void)
th_io_handler_init(th_io_handler* handler, void (*fn)(void* self, size_t result, th_err err), void (*destroy)(void* self))
{
    th_task_init(&handler->base, th_io_handler_fn, destroy);
    handler->fn = fn;
}

TH_INLINE(void)
th_io_handler_set_result(th_io_handler* handler, size_t result, th_err err)
{
    handler->result = result;
    handler->err = err;
}

TH_INLINE(void)
th_io_handler_complete(th_io_handler* handler, size_t result, th_err err)
{
    th_io_handler_set_result(handler, result, err);
    th_task_complete(&handler->base);
}

TH_INLINE(void)
th_io_handler_destroy(th_io_handler* handler)
{
    th_task_destroy(&handler->base);
}

// some aliases

typedef th_io_handler th_write_handler;
typedef th_io_handler th_read_handler;
#define th_write_handler_init th_io_handler_init
#define th_read_handler_init th_io_handler_init
#define th_write_handler_complete th_io_handler_complete
#define th_read_handler_complete th_io_handler_complete

typedef enum th_io_open_flag {
    TH_IO_OPEN_FLAG_RDONLY = 1 << 0,
    TH_IO_OPEN_FLAG_DIR = 1 << 1,
} th_io_open_flag;

/** th_io_op
 *@brief I/O operation type.
 */
typedef enum th_io_op_type {
    TH_IO_OP_TYPE_NONE = 0,
    TH_IO_OP_TYPE_READ = 1,
    TH_IO_OP_TYPE_WRITE = 2,
    TH_IO_OP_TYPE_MAX = TH_IO_OP_TYPE_WRITE
} th_io_op_type;
#define TH_IO_OP(opc, type) ((opc) | ((type) << 8))
#define TH_IO_OP_TYPE(op) ((op) >> 8)
typedef enum th_io_op {
    TH_IO_OP_ACCEPT = TH_IO_OP(0, TH_IO_OP_TYPE_READ),
    TH_IO_OP_READ = TH_IO_OP(1, TH_IO_OP_TYPE_READ),
    TH_IO_OP_WRITE = TH_IO_OP(2, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_WRITEV = TH_IO_OP(3, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_SEND = TH_IO_OP(4, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_SENDV = TH_IO_OP(5, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_READV = TH_IO_OP(6, TH_IO_OP_TYPE_READ),
    TH_IO_OP_OPENAT = TH_IO_OP(7, TH_IO_OP_TYPE_NONE),
    TH_IO_OP_OPEN = TH_IO_OP(8, TH_IO_OP_TYPE_NONE),
    TH_IO_OP_CLOSE = TH_IO_OP(9, TH_IO_OP_TYPE_NONE),
    TH_IO_OP_SENDFILE = TH_IO_OP(10, TH_IO_OP_TYPE_WRITE),
} th_io_op;

/** th_io_task
 *@brief I/O task, inherits from th_task.
 * Contains the I/O operation type and the I/O operation arguments.
 */
typedef struct th_io_task {
    th_task base;
    th_allocator* allocator;
    th_err (*fn)(void* self, size_t* result);
    th_io_handler* on_complete;
    void* addr;
    void* addr2;
    size_t len;
    size_t len2;
    size_t offset;
    unsigned int flags;
    int fd;
    enum th_io_op op;
} th_io_task;

TH_PRIVATE(th_io_task*)
th_io_task_create(th_allocator* allocator);

/*
TH_PRIVATE(void)
th_io_task_to_string(char* buf, size_t len, th_io_task* iot);
*/

TH_PRIVATE(void)
th_io_task_prepare_read(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete);

/*
TH_PRIVATE(void)
th_io_task_prepare_write(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_writev(th_io_task* iot, int fd, th_iov* iov, size_t len, th_io_handler* on_complete);
*/

TH_PRIVATE(void)
th_io_task_prepare_send(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_sendv(th_io_task* iot, int fd, th_iov* iov, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_readv(th_io_task* iot, int fd, th_iov* iov, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_sendfile(th_io_task* iot, th_file* file, int sfd, th_iov* header, size_t iovcnt,
                            size_t offset, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_accept(th_io_task* iot, int fd, void* addr, void* addrlen, th_io_handler* on_complete);

/** th_io_task_execute
 * @brief Executes the I/O task and leaves the completion handler untouched.
 * @param iot I/O task.
 * @param result Result of the I/O operation.
 * @return Error code.
 */
TH_PRIVATE(th_err)
th_io_task_execute(th_io_task* iot, size_t* result);

/** th_io_task_try_execute
 * @brief Tries to execute the I/O task and returns the completion handler
 * if the I/O operation was completed.
 * @param iot I/O task.
 * @return Completion handler.
 */
TH_PRIVATE(th_io_handler*)
th_io_task_try_execute(th_io_task* iot);

TH_PRIVATE(void)
th_io_task_destroy(th_io_task* iot);

/** th_io_task_abort
 * @brief Aborts the I/O task. Sets the error code and returns the completion handler.
 * @param iot I/O task.
 * @param err Error code.
 */
TH_PRIVATE(th_io_handler*)
th_io_task_abort(th_io_task* iot, th_err err);

/* End of th_io_task.h */
/* Start of th_io_service.h */



typedef struct th_io_handle {
    void (*cancel)(void* self);
    void (*submit)(void* self, th_io_task* task);
    void (*enable_timeout)(void* self, bool enabled);
    int (*get_fd)(void* self);
    void (*destroy)(void* self);
} th_io_handle;

TH_INLINE(void)
th_io_handle_cancel(th_io_handle* io_handle)
{
    io_handle->cancel(io_handle);
}

TH_INLINE(void)
th_io_handle_submit(th_io_handle* io_handle, th_io_task* iot)
{
    io_handle->submit(io_handle, iot);
}

TH_INLINE(int)
th_io_handle_get_fd(th_io_handle* io_handle)
{
    return io_handle->get_fd(io_handle);
}

TH_INLINE(void)
th_io_handle_enable_timeout(th_io_handle* io_handle, bool enabled)
{
    io_handle->enable_timeout(io_handle, enabled);
}

TH_INLINE(void)
th_io_handle_destroy(th_io_handle* io_handle)
{
    io_handle->destroy(io_handle);
}

typedef struct th_io_service {
    void (*run)(void* self, int timeout_ms);
    th_err (*create_handle)(void* self, th_io_handle** out, int fd);
    void (*destroy)(void* self);
} th_io_service;

TH_INLINE(void)
th_io_service_run(th_io_service* io_service, int timeout_ms)
{
    io_service->run(io_service, timeout_ms);
}

TH_INLINE(th_err)
th_io_service_create_handle(th_io_service* io_service, th_io_handle** out, int fd)
{
    return io_service->create_handle(io_service, out, fd);
}

TH_INLINE(void)
th_io_service_destroy(th_io_service* io_service)
{
    if (io_service->destroy)
        io_service->destroy(io_service);
}

/* End of th_io_service.h */
/* Start of th_runner.h */



typedef struct th_runner {
    th_io_service* io_service;
    th_task service_task;
    int waiting;
    th_task_queue queue;
    size_t num_tasks;
} th_runner;

TH_PRIVATE(void)
th_runner_init(th_runner* runner);

TH_PRIVATE(void)
th_runner_set_io_service(th_runner* runner, th_io_service* service);

TH_PRIVATE(void)
th_runner_push_task(th_runner* runner, th_task* task);

TH_PRIVATE(void)
th_runner_push_uncounted_task(th_runner* runner, th_task* task);

TH_PRIVATE(void)
th_runner_increase_task_count(th_runner* runner);

TH_PRIVATE(th_err)
th_runner_poll(th_runner* runner, int timeout_ms);

TH_PRIVATE(void)
th_runner_drain(th_runner* runner);

TH_PRIVATE(void)
th_runner_deinit(th_runner* runner);

/* End of th_runner.h */
/* Start of th_timer.h */



#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct th_timer {
    time_t expire;
} th_timer;

TH_PRIVATE(void)
th_timer_init(th_timer* timer);

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration);

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer);

/* End of th_timer.h */
/* Start of th_kqueue_service.h */



#ifdef TH_CONFIG_WITH_KQUEUE

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

/* Forward declarations begin */

typedef struct th_kqueue_service th_kqueue_service;
typedef struct th_kqueue_handle th_kqueue_handle;
typedef struct th_kqueue_handle_cleaner th_kqueue_handle_cleaner;

/* Forward declarations end */

struct th_kqueue_handle {
    th_io_handle base;
    th_timer timer;
    th_allocator* allocator;
    th_kqueue_handle* pool_next;
    th_kqueue_handle* pool_prev;
    th_kqueue_handle* timer_next;
    th_kqueue_handle* timer_prev;
    th_kqueue_service* service;
    th_io_task* iot[TH_IO_OP_TYPE_MAX];
    int fd;
    th_io_op_type active;
    bool timeout_enabled;
};

#ifndef TH_KQUEUE_HANDLE_POOL
#define TH_KQUEUE_HANDLE_POOL
TH_DEFINE_OBJ_POOL_ALLOCATOR(th_kqueue_handle_pool, th_kqueue_handle, pool_prev, pool_next)
#endif

#ifndef TH_KQUEUE_HANDLE_TIMER_LIST
#define TH_KQUEUE_HANDLE_TIMER_LIST
TH_DEFINE_LIST(th_kqueue_timer_list, th_kqueue_handle, timer_prev, timer_next)
#endif

struct th_kqueue_service {
    th_io_service base;
    th_allocator* allocator;
    th_runner* runner;
    th_kqueue_handle_pool handle_allocator;
    th_kqueue_timer_list timer_list;
    int kq;
};

TH_PRIVATE(th_err)
th_kqueue_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator);

#endif /* TH_HAVE_KQUEUE */
/* End of th_kqueue_service.h */
/* Start of th_io_composite.h */


/** th_io_composite
 *@brief I/O composite task, inherits from th_io_handler.
 * and contains a pointer to another I/O handler that will be called
 * when the composite task is completed.
 */
typedef struct th_io_composite {
    th_io_handler base;
    th_io_handler* on_complete;
    void (*destroy)(void* self);
    unsigned int refcount;
} th_io_composite;

TH_PRIVATE(void)
th_io_composite_unref(void* self);

TH_INLINE(void)
th_io_composite_init(th_io_composite* composite, void (*fn)(void* self, size_t result, th_err err), void (*destroy)(void* self), th_io_handler* on_complete)
{
    th_io_handler_init(&composite->base, fn, th_io_composite_unref);
    composite->destroy = destroy;
    composite->on_complete = on_complete;
    composite->refcount = 1;
}

static inline void
th_io_composite_complete(th_io_composite* composite, size_t result, th_err err)
{
    th_io_handler_complete(composite->on_complete, result, err);
}

TH_INLINE(th_io_composite*)
th_io_composite_ref(th_io_composite* composite)
{
    ++composite->refcount;
    return composite;
}

typedef enum th_io_composite_forward_type {
    TH_IO_COMPOSITE_FORWARD_MOVE,
    TH_IO_COMPOSITE_FORWARD_COPY
} th_io_composite_forward_type;

TH_INLINE(th_io_composite*)
th_io_composite_forward(th_io_composite* composite, th_io_composite_forward_type type) TH_MAYBE_UNUSED;

TH_INLINE(th_io_composite*)
th_io_composite_forward(th_io_composite* composite, th_io_composite_forward_type type)
{
    switch (type) {
    case TH_IO_COMPOSITE_FORWARD_MOVE:
        return composite;
    case TH_IO_COMPOSITE_FORWARD_COPY:
        return th_io_composite_ref(composite);
        break;
    default:
        return NULL;
        break;
    }
}

/* End of th_io_composite.h */
/* Start of th_context.h */


typedef struct th_context {
    th_runner runner;
    th_allocator* allocator;
    th_io_service* io_service;
} th_context;

TH_PRIVATE(th_err)
th_context_init(th_context* context, th_allocator* allocator);

TH_PRIVATE(th_err)
th_context_init_with_service(th_context* context, th_io_service* service) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_context_push_task(th_context* context, th_task* task) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_context_create_handle(th_context* context, th_io_handle** out, int fd);

TH_PRIVATE(th_err)
th_context_poll(th_context* context, int timeout_ms);

TH_PRIVATE(void)
th_context_drain(th_context* context);

TH_PRIVATE(void)
th_context_deinit(th_context* context);

TH_PRIVATE(void)
th_context_dispatch_handler(th_context* context, th_io_handler* handler, size_t result, th_err err);

TH_PRIVATE(void)
th_context_dispatch_composite_completion(th_context* context, th_io_composite* composite, size_t result, th_err err) TH_MAYBE_UNUSED;

/* End of th_context.h */
/* Start of th_socket.h */


#include <sys/socket.h>


typedef struct th_address {
    struct sockaddr_storage addr;
    socklen_t addrlen;
} th_address;

TH_PRIVATE(void)
th_address_init(th_address* addr);

/* th_socket_handler begin */

typedef th_io_handler th_socket_handler;
#define th_socket_handler_init th_io_handler_init
#define th_socket_handler_complete th_io_handler_complete

/* th_socket_task_handler end */
/* th_socket begin */

typedef struct th_socket_methods {
    void (*set_fd)(void* self, int fd);
    void (*cancel)(void* self);
    th_allocator* (*get_allocator)(void* self);
    th_context* (*get_context)(void* self);
    void (*async_write)(void* self, void* addr, size_t len, th_socket_handler* handler);
    void (*async_writev)(void* self, th_iov* iov, size_t len, th_socket_handler* handler);
    void (*async_read)(void* self, void* addr, size_t len, th_socket_handler* handler);
    void (*async_readv)(void* self, th_iov* iov, size_t len, th_socket_handler* handler);
    void (*async_sendfile)(void* self, th_iov* header, size_t iovcnt,
                           th_file* stream, size_t offset, size_t len, th_socket_handler* handler);
} th_socket_methods;

typedef struct th_socket {
    const th_socket_methods* methods;
} th_socket;

TH_INLINE(void)
th_socket_set_fd(th_socket* socket, int fd)
{
    socket->methods->set_fd(socket, fd);
}

TH_INLINE(void)
th_socket_cancel(th_socket* socket)
{
    socket->methods->cancel(socket);
}

TH_INLINE(th_allocator*)
th_socket_get_allocator(th_socket* socket)
{
    return socket->methods->get_allocator(socket);
}

TH_INLINE(th_context*)
th_socket_get_context(th_socket* socket)
{
    return socket->methods->get_context(socket);
}

TH_INLINE(void)
th_socket_async_write(th_socket* sock, void* addr, size_t len, th_socket_handler* handler)
{
    sock->methods->async_write(sock, addr, len, handler);
}

TH_INLINE(void)
th_socket_async_writev(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler)
{
    sock->methods->async_writev(sock, iov, len, handler);
}

TH_INLINE(void)
th_socket_async_read(th_socket* sock, void* addr, size_t len, th_socket_handler* handler)
{
    sock->methods->async_read(sock, addr, len, handler);
}

TH_INLINE(void)
th_socket_async_readv(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler)
{
    sock->methods->async_readv(sock, iov, len, handler);
}

TH_INLINE(void)
th_socket_async_sendfile(th_socket* sock, th_iov* header, size_t iovcnt,
                         th_file* stream, size_t offset, size_t len, th_socket_handler* handler)
{
    sock->methods->async_sendfile(sock, header, iovcnt, stream, offset, len, handler);
}

/* th_socket end */
/** generic socket functions begin */

TH_PRIVATE(void)
th_socket_async_write_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* handler) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_socket_async_writev_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler);

TH_PRIVATE(void)
th_socket_async_read_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* handler);

TH_PRIVATE(void)
th_socket_async_readv_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* handler) TH_MAYBE_UNUSED;

TH_PRIVATE(void)
th_socket_async_sendfile_exact(th_socket* sock, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_socket_handler* handler);

/* th_socket functionss end */

/* End of th_socket.h */
/* Start of th_acceptor.h */



typedef struct th_acceptor {
    th_context* context;
    th_allocator* allocator;
    th_io_handle* handle;
} th_acceptor;

typedef struct th_acceptor_opt {
    bool reuse_addr;
    bool reuse_port;
} th_acceptor_opt;

TH_PRIVATE(th_err)
th_acceptor_init(th_acceptor* acceptor, th_context* context,
                 th_allocator* allocator,
                 const char* addr, const char* port);

/** th_acceptor_async_accept
 * @brief Asynchronously accept a new connection. And call the handler when the operation is complete.
 * Both addr and sock must point to valid memory locations until the handler is called.
 * @param acceptor The acceptor that will accept the new connection.
 * @param addr Pointer to the address that will be filled with the address of the new connection.
 * @param sock Pointer to the socket that will be filled with the new connection.
 */
TH_PRIVATE(void)
th_acceptor_async_accept(th_acceptor* acceptor, th_address* addr, th_io_handler* handler);

TH_PRIVATE(void)
th_acceptor_cancel(th_acceptor* acceptor);

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor);

/* End of th_acceptor.h */
/* Start of th_method.h */


struct th_method_mapping {
    const char* name;
    th_method method;
};

struct th_method_mapping* th_method_mapping_find(const char* str, size_t len);

/* End of th_method.h */
/* Start of th_dir_mgr.h */



TH_DEFINE_HASHMAP(th_dir_map, th_string, th_dir, th_string_hash, th_string_eq, (th_string){0})

typedef struct th_dir_mgr {
    th_allocator* allocator;
    th_dir_map map;
    th_heap_string_vec heap_strings;
} th_dir_mgr;

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator);

TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_string label, th_string path);

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_string label);

TH_PRIVATE(void)
th_dir_mgr_deinit(th_dir_mgr* mgr);

/* End of th_dir_mgr.h */
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
/* Start of th_fcache.h */



typedef struct th_fcache th_fcache;
typedef struct th_fcache_entry th_fcache_entry;
struct th_fcache_entry {
    th_refcounted base;
    th_file stream;
    th_heap_string path;
    th_dir* dir;
    th_allocator* allocator;
    th_fcache* cache;
    th_fcache_entry* next;
    th_fcache_entry* prev;
    uint32_t stat_hash;
};

typedef struct th_fcache_id {
    th_string path;
    th_dir* dir;
} th_fcache_id;

TH_INLINE(bool)
th_fcache_id_eq(th_fcache_id a, th_fcache_id b)
{
    return a.dir == b.dir && th_string_eq(a.path, b.path);
}

TH_INLINE(size_t)
th_fcache_id_hash(th_fcache_id id)
{
    return th_string_hash(id.path) + (size_t)id.dir->fd;
}

TH_DEFINE_HASHMAP(th_fcache_map, th_fcache_id, th_fcache_entry*, th_fcache_id_hash, th_fcache_id_eq, (th_fcache_id){0})
TH_DEFINE_LIST(th_fcache_list, th_fcache_entry, prev, next)

struct th_fcache {
    th_allocator* allocator;
    th_dir_mgr dir_mgr;
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
th_fcache_init(th_fcache* cache, th_allocator* allocator);

TH_PRIVATE(th_err)
th_fcache_get(th_fcache* cache, th_string root, th_string path, th_fcache_entry** out);

TH_PRIVATE(th_err)
th_fcache_add_dir(th_fcache* cache, th_string label, th_string path);

TH_PRIVATE(th_dir*)
th_fcache_find_dir(th_fcache* cache, th_string label);

TH_PRIVATE(void)
th_fcache_deinit(th_fcache* cache);

/* End of th_fcache.h */
/* Start of th_upload.h */



struct th_upload {
    th_heap_string name;
    th_heap_string filename;
    th_heap_string content_type;
    th_string data;
    th_fcache* fcache;
};

TH_PRIVATE(void)
th_upload_init(th_upload* upload, th_string buffer, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_upload_deinit(th_upload* upload);

TH_PRIVATE(th_err)
th_upload_set_name(th_upload* upload, th_string name);

TH_PRIVATE(th_err)
th_upload_set_filename(th_upload* upload, th_string filename);

TH_PRIVATE(th_err)
th_upload_set_content_type(th_upload* upload, th_string content_type);

/* End of th_upload.h */
/* Start of th_request.h */



struct th_iter_methods {
    bool (*next)(th_iter* it);
    const char* (*key)(const th_iter* it);
    const void* (*val)(const th_iter* it);
};

typedef struct th_hstr_pair {
    th_heap_string key;
    th_heap_string value;
} th_hstr_pair;

TH_INLINE(void)
th_hstr_pair_deinit(th_hstr_pair* pair)
{
    th_heap_string_deinit(&pair->key);
    th_heap_string_deinit(&pair->value);
}

TH_DEFINE_VEC(th_hstr_vec, th_hstr_pair, th_hstr_pair_deinit)

TH_DEFINE_VEC(th_upload_vec, th_upload, th_upload_deinit)

struct th_request {
    th_allocator* allocator;
    th_fcache* fcache;
    th_heap_string uri_path;
    th_heap_string uri_query;
    th_upload_vec uploads;
    th_hstr_vec cookies;
    th_hstr_vec headers;
    th_hstr_vec queryvars;
    th_hstr_vec formvars;
    th_hstr_vec pathvars;
    th_string body;
    th_method method;
    int version;
    bool close;
};

TH_PRIVATE(void)
th_request_init(th_request* request, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_request_deinit(th_request* request);

TH_PRIVATE(void)
th_request_reset(th_request* request);

TH_PRIVATE(void)
th_request_set_version(th_request* request, int version);

TH_PRIVATE(void)
th_request_set_method(th_request* request, th_method method);

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_string path);

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_string query);

TH_PRIVATE(th_err)
th_request_add_queryvar(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_string key, th_string value);

TH_PRIVATE(th_err)
th_request_add_upload(th_request* request, th_string data, th_string name, th_string filename, th_string content_type);

TH_PRIVATE(void)
th_request_clear_queryvars(th_request* request);

TH_PRIVATE(void)
th_request_set_body(th_request* request, th_string body);

TH_PRIVATE(th_string)
th_request_get_header(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_string)
th_request_get_pathvar(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_string)
th_request_get_queryvar(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_string)
th_request_get_formvar(th_request* request, th_string key) TH_MAYBE_UNUSED;

TH_PRIVATE(th_upload*)
th_request_get_upload(th_request* request, th_string key) TH_MAYBE_UNUSED;

/* End of th_request.h */
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

/* End of th_response.h */
/* Start of th_router.h */



typedef struct th_route_handler {
    th_handler handler;
    void* user_data;
} th_route_handler;

typedef struct th_capture {
    th_string key;
    th_string value;
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
    th_heap_string name;
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
th_router_add_route(th_router* router, th_method method, th_string route, th_handler handler, void* user_data);

/* End of th_router.h */
/* Start of th_ssl_context.h */


#if TH_WITH_SSL

#include <openssl/ssl.h>

typedef struct th_ssl_context {
    SSL_CTX* ctx;
    BIO_METHOD* smem_method;
} th_ssl_context;

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, const char* key, const char* cert);

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context);

#endif
/* End of th_ssl_context.h */
/* Start of th_tcp_socket.h */


/* th_tcp_socket begin */

typedef struct th_tcp_socket {
    th_socket base;
    th_context* context;
    th_allocator* allocator;
    th_io_handle* handle;
} th_tcp_socket;

TH_PRIVATE(void)
th_tcp_socket_init(th_tcp_socket* socket, th_context* context, th_allocator* allocator);

/** th_socket_close
 * @brief Closes the underlying file descriptor of the socket.
 * while the socket object is still valid and can be reused.
 */
TH_PRIVATE(void)
th_tcp_socket_close(th_tcp_socket* socket);

TH_PRIVATE(void)
th_tcp_socket_deinit(th_tcp_socket* socket);

#define th_tcp_socket_set_fd(socket, fd) ((socket)->base.methods->set_fd((socket), (fd)))

#define th_tcp_socket_cancel(socket) ((socket)->base.methods->cancel((socket)))

#define th_tcp_socket_get_allocator(socket) ((socket)->base.methods->get_allocator((socket)))

#define th_tcp_socket_get_context(socket) ((socket)->base.methods->get_context((socket)))

#define th_tcp_socket_async_write(socket, addr, len, handler) ((socket)->base.methods->async_write((socket), (addr), (len), (handler)))

#define th_tcp_socket_async_writev(socket, iov, iovcnt, handler) ((socket)->base.methods->async_writev((socket), (iov), (iovcnt), (handler)))

#define th_tcp_socket_async_read(socket, addr, len, handler) ((socket)->base.methods->async_read((socket), (addr), (len), (handler)))

#define th_tcp_socket_async_readv(socket, iov, iovcnt, handler) ((socket)->base.methods->async_readv((socket), (iov), (iovcnt), (handler)))

#define th_tcp_socket_async_sendfile(socket, header, iovcnt, stream, offset, len, handler) ((socket)->base.methods->async_sendfile((socket), (header), (iovcnt), (stream), (offset), (len), (handler)))

/* th_tcp_socket end */

/* End of th_tcp_socket.h */
/* Start of th_ssl_socket.h */


#if TH_WITH_SSL


#include <openssl/ssl.h>

/* th_ssl_socket begin */

typedef struct th_ssl_socket {
    th_socket base;
    th_tcp_socket tcp_socket;
    SSL* ssl;
    BIO* wbio; // ssl output buffer
    BIO* rbio; // ssl input buffer
} th_ssl_socket;

typedef enum th_ssl_socket_mode {
    TH_SSL_SOCKET_MODE_SERVER,
    TH_SSL_SOCKET_MODE_CLIENT
} th_ssl_socket_mode;

TH_PRIVATE(th_err)
th_ssl_socket_init(th_ssl_socket* socket, th_context* context, th_ssl_context* ssl_context, th_allocator* allocator);

/** ssl socket specific functions */

TH_PRIVATE(void)
th_ssl_socket_set_mode(th_ssl_socket* socket, th_ssl_socket_mode mode);

TH_PRIVATE(void)
th_ssl_socket_async_handshake(th_ssl_socket* socket, th_socket_handler* handler);

TH_PRIVATE(void)
th_ssl_socket_async_shutdown(th_ssl_socket* socket, th_socket_handler* handler);

/** th_socket_close
 * @brief Closes the underlying file descriptor of the socket.
 * while the socket object is still valid and can be reused.
 */
TH_PRIVATE(void)
th_ssl_socket_close(th_ssl_socket* socket);

TH_PRIVATE(void)
th_ssl_socket_deinit(th_ssl_socket* socket);

#endif
/* End of th_ssl_socket.h */
/* Start of th_conn.h */



/* th_prot interface begin */

/* th_prot interface end */
/* th_conn interface begin */
typedef struct th_conn th_conn;
struct th_conn {
    th_socket* (*get_socket)(void* self);
    th_address* (*get_address)(void* self);
    void (*start)(void* self);
    void (*destroy)(void* self);
};

/** th_conn_init
 * @brief Initialize the client interface, this function should be called
 * by the parent client implementation on initialization.
 */
TH_INLINE(void)
th_conn_init(th_conn* client,
             th_socket* (*get_socket)(void* self),
             th_address* (*get_address)(void* self),
             void (*start)(void* self),
             void (*destroy)(void* self))
{
    client->get_socket = get_socket;
    client->get_address = get_address;
    client->start = start;
    client->destroy = destroy;
}

TH_INLINE(th_socket*)
th_conn_get_socket(th_conn* client)
{
    return client->get_socket(client);
}

TH_INLINE(th_address*)
th_conn_get_address(th_conn* client)
{
    return client->get_address(client);
}

TH_INLINE(void)
th_conn_start(th_conn* client)
{
    client->start(client);
}

TH_INLINE(void)
th_conn_destroy(th_conn* client)
{
    client->destroy(client);
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

/* th_conn_observable interface end */
/* th_tcp_conn declaration begin */

typedef struct th_tcp_conn th_tcp_conn;

struct th_tcp_conn {
    th_conn_observable base;
    th_tcp_socket socket;
    th_address addr;
    th_context* context;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_context* context,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);

/* th_tcp_conn declaration end */
/* th_ssl_conn declaration begin */
#if TH_WITH_SSL
typedef struct th_ssl_conn th_ssl_conn;
typedef struct th_ssl_conn_io_handler {
    th_io_handler base;
    th_ssl_conn* conn;
} th_ssl_conn_io_handler;

struct th_ssl_conn {
    th_conn_observable base;
    th_ssl_conn_io_handler handshake_handler;
    th_ssl_conn_io_handler shutdown_handler;
    th_ssl_socket socket;
    th_address addr;
    th_context* context;
    th_conn_upgrader* upgrader;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_context* context, th_ssl_context* ssl_context,
                   th_conn_upgrader* upgrader, th_conn_observer* observer,
                   th_allocator* allocator);
#endif
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
th_request_parser_parse(th_request_parser* parser, th_request* request, th_string data, size_t* parsed);

TH_PRIVATE(bool)
th_request_parser_header_done(th_request_parser* parser);

TH_PRIVATE(bool)
th_request_parser_done(th_request_parser* parser);

/* End of th_request_parser.h */
/* Start of th_http.h */



typedef struct th_http th_http;

typedef enum th_http_state {
    TH_HTTP_STATE_READ_REQUEST,
    TH_HTTP_STATE_WRITE_RESPONSE,
} th_http_state;

typedef struct th_http_io_handler {
    th_io_handler base;
    th_http* http;
} th_http_io_handler;

struct th_http {
    const th_conn_tracker* tracker;
    th_http_io_handler io_handler;
    th_request_parser parser;
    th_request request;
    th_response response;
    th_buf_vec buf;
    th_conn* conn;
    th_router* router;
    th_fcache* fcache;
    th_allocator* allocator;
    size_t read_bytes;
    size_t parsed_bytes;

    // the current state of the http connection
    th_http_state state;

    // true if the connection should be closed
    bool close;
};

typedef struct th_http_upgrader {
    th_conn_upgrader base;
    const th_conn_tracker* tracker;
    th_router* router;
    th_fcache* fcache;
    th_allocator* allocator;
} th_http_upgrader;

TH_PRIVATE(void)
th_http_upgrader_init(th_http_upgrader* upgrader, const th_conn_tracker* tracker, th_router* router,
                      th_fcache* fcache, th_allocator* allocator);

/* End of th_http.h */
/* Start of th_listener.h */


typedef struct th_listener th_listener;

typedef struct th_listener_accept_handler {
    th_io_handler base;
    th_listener* listener;
} th_listener_accept_handler;

typedef struct th_listener_conn_destroy_handler {
    th_task base;
    th_listener* listener;
} th_listener_conn_destroy_handler;

struct th_listener {
    th_acceptor acceptor;
    th_listener* next;
    th_context* context;

    /** The conn that will be used to handle the incoming connections. */
    th_conn* conn;

    /** Used to keep track of all the clients that are currently active. */
    th_conn_tracker conn_tracker;

    /** Used to react to the destruction of a client. */
    th_listener_conn_destroy_handler client_destroy_handler;

    th_http_upgrader upgrader;

#if TH_WITH_SSL
    /** Ssl context that will be used to create the ssl socket. */
    th_ssl_context ssl_context;
#endif /* TH_WITH_SSL */

    /** Flag that indicates if ssl is enabled. */
    bool ssl_enabled;

    /** The accept handler that will be used to handle the completion
     * of the accept operation.
     */
    th_listener_accept_handler accept_handler;

    /** As long as the listener keeps accepting new connections,
     * this flag will be set to 1.
     */
    bool running;
    th_allocator* allocator;
};

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_context* context,
                   const char* host, const char* port,
                   th_router* router, th_fcache* fcache,
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
    th_string mime;
};

struct th_mime_mapping* th_mime_mapping_find(const char* ext, size_t len);

/* End of th_mime.h */
/* Start of th_mock_service.h */



#if defined(TH_CONFIG_OS_MOCK)


typedef struct th_mock_service th_mock_service;
typedef struct th_mock_handle th_mock_handle;
struct th_mock_handle {
    th_io_handle base;
    th_mock_service* service;
    int fd;
};

struct th_mock_service {
    th_io_service base;
    th_runner* runner;
};

TH_PRIVATE(th_err)
th_mock_service_create(th_io_service** out, th_runner* runner);

#endif
/* End of th_mock_service.h */
/* Start of th_mock_syscall.h */

#include <stddef.h>


typedef struct th_mock_syscall {
    int (*accept)(void);
    int (*open)(void);
    int (*lseek)(void);
    int (*close)(void);
    int (*read)(void* buf, size_t len);
    int (*write)(size_t len);
    int (*settime)(void);
} th_mock_syscall;

th_mock_syscall* th_mock_syscall_get(void);
void th_mock_syscall_reset(void);

int th_mock_accept(void);

int th_mock_open(void);

int th_mock_lseek(void);

int th_mock_close(void);

int th_mock_read(void* buf, size_t len);

int th_mock_write(size_t len);

int th_mock_settime(void);

/* End of th_mock_syscall.h */
/* Start of th_path.h */


/**
 * @brief th_path provides a bunch of helper functions to work with paths.
 */

/**
 * @brief th_path_resolve resolves a path to a absolute path.
 * @param dir The directory to resolve the path against.
 * @param path The path to resolve.
 * @param out The resolved path.
 * @return TH_ERR_OK on success, otherwise an error code.
 */
TH_PRIVATE(th_err)
th_path_resolve_against(th_string path, th_dir* dir, th_heap_string* out);

TH_PRIVATE(th_err)
th_path_resolve(th_string path, th_heap_string* out);

TH_PRIVATE(bool)
th_path_is_within(th_string path, th_dir* dir);

TH_PRIVATE(bool)
th_path_is_hidden(th_string path);

/* End of th_path.h */
/* Start of th_poll_service.h */



#ifdef TH_CONFIG_WITH_POLL

#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Forward declarations begin */

typedef struct th_poll_service th_poll_service;
typedef struct th_poll_handle th_poll_handle;
typedef struct th_poll_handle_map th_poll_handle_map;

/* Forward declarations end */
/* th_fd_to_idx_map implementation begin */

TH_INLINE(uint32_t)
th_fd_hash(int fd)
{
    return (uint32_t)fd;
}

TH_INLINE(bool)
th_int_eq(int a, int b)
{
    return a == b;
}

TH_DEFINE_HASHMAP(th_fd_to_idx_map, int, size_t, th_fd_hash, th_int_eq, -1)

/* th_fd_to_idx_map implementation end */
/* th_poll_handle begin */

struct th_poll_handle {
    th_io_handle base;
    th_timer timer;
    th_poll_handle* next;
    th_poll_handle* prev;
    th_allocator* allocator;
    th_poll_service* service;
    th_io_task* iot[TH_IO_OP_TYPE_MAX];
    int fd;
    bool timeout_enabled;
};

#ifndef TH_POLL_HANDLE_POOL
#define TH_POLL_HANDLE_POOL
TH_DEFINE_OBJ_POOL_ALLOCATOR(th_poll_handle_pool, th_poll_handle, prev, next)
#endif

#ifndef TH_POLL_HANDLE_LIST
#define TH_POLL_HANDLE_LIST
TH_DEFINE_QUEUE(th_poll_handle_list, th_poll_handle)
#endif

#ifndef TH_POLLFD_VEC
#define TH_POLLFD_VEC
TH_DEFINE_VEC(th_pollfd_vec, struct pollfd, (void))
#endif

/* th_poll_handle end */
/* th_poll_handle_map begin */

struct th_poll_handle_map {
    th_fd_to_idx_map fd_to_idx_map;
    th_allocator* allocator;
    th_poll_handle** handles;
    size_t size;
    size_t capacity;
};

/* th_poll_handle_map end */

struct th_poll_service {
    th_io_service base;
    th_allocator* allocator;
    th_runner* runner;
    th_poll_handle_pool handle_allocator;
    th_poll_handle_map handles;
    th_pollfd_vec fds;
};

TH_PRIVATE(th_err)
th_poll_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator);

#endif /* TH_HAVE_POLL */
/* End of th_poll_service.h */
/* Start of th_ssl_error.h */


#if TH_WITH_SSL


TH_PRIVATE(void)
th_ssl_log_error_stack(void);

TH_PRIVATE(const char*)
th_ssl_strerror(int code);

TH_PRIVATE(th_err)
th_ssl_handle_error_stack(void);

#endif // TH_WITH_SSL
/* End of th_ssl_error.h */
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
/* Start of th_url_decode.h */



#include <stddef.h>

typedef enum th_url_decode_type {
    TH_URL_DECODE_TYPE_PATH = 0,
    TH_URL_DECODE_TYPE_QUERY
} th_url_decode_type;

/*
TH_PRIVATE(th_err)
th_url_decode_inplace(char* str, size_t* in_out_len, th_url_decode_type type);
*/

TH_PRIVATE(th_err)
th_url_decode_string(th_string input, th_heap_string* output, th_url_decode_type type);

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

#include <stdint.h>
#include <string.h>


#define TH_MAIN_ALLOCATOR_PTR_OFFSET TH_ALIGNUP(sizeof(uint32_t), TH_ALIGNOF(th_max_align))
#define TH_MAIN_ALLOCATOR_BUCKET_NUM 5

typedef struct th_main_allocator {
    th_allocator base;
    th_allocator* allocator;
    th_pool_allocator pool[TH_MAIN_ALLOCATOR_BUCKET_NUM];
} th_main_allocator;

TH_LOCAL(size_t)
th_main_allocator_bucket_size(int index)
{
    TH_ASSERT(index >= 0 && index < TH_MAIN_ALLOCATOR_BUCKET_NUM);
    static const size_t bucket_sizes[] = {128, 256, 512, 1024, 2048};
    return bucket_sizes[index];
}

TH_LOCAL(int)
th_main_allocator_bucket_index(size_t size)
{
    static const int bucket_map[] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
    size_t n = (size - 1) / 128;
    if (n < TH_ARRAY_SIZE(bucket_map))
        return bucket_map[n];
    return TH_MAIN_ALLOCATOR_BUCKET_NUM;
}

TH_LOCAL(void*)
th_main_allocator_alloc(void* self, size_t size)
{
    th_main_allocator* allocator = self;
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    void* ptr = NULL;
    int index = th_main_allocator_bucket_index(size);
    if (index < TH_MAIN_ALLOCATOR_BUCKET_NUM) {
        ptr = th_allocator_alloc(&allocator->pool[index].base, th_main_allocator_bucket_size(index) + ptr_offset);
    } else {
        ptr = th_allocator_alloc(allocator->allocator, size + ptr_offset);
    }
    if (!ptr)
        return NULL;
    ((uint32_t*)ptr)[0] = (uint32_t)size;
    return (char*)ptr + ptr_offset;
}

TH_LOCAL(void)
th_main_allocator_free(void* self, void* ptr)
{
    th_main_allocator* allocator = self;
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    void* old_ptr = (char*)ptr - ptr_offset;
    size_t size = ((uint32_t*)old_ptr)[0];
    int index = th_main_allocator_bucket_index(size);
    if (index < TH_MAIN_ALLOCATOR_BUCKET_NUM) {
        th_allocator_free(&allocator->pool[index].base, old_ptr);
    } else {
        th_allocator_free(allocator->allocator, old_ptr);
    }
}

TH_LOCAL(void*)
th_main_allocator_realloc(void* self, void* ptr, size_t size)
{
    th_main_allocator* allocator = self;
    if (!ptr)
        return th_main_allocator_alloc(allocator, size);
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    void* old_ptr = (char*)ptr - ptr_offset;
    size_t old_size = ((uint32_t*)old_ptr)[0];
    if (old_size >= size)
        return ptr;
    void* new_ptr = th_main_allocator_alloc(allocator, size);
    if (!new_ptr)
        return NULL;
    memcpy(new_ptr, ptr, old_size);
    th_main_allocator_free(allocator, ptr);
    return new_ptr;
}

TH_LOCAL(void)
th_main_allocator_init(th_main_allocator* allocator, th_allocator* parent)
{
    allocator->base.alloc = th_main_allocator_alloc;
    allocator->base.realloc = th_main_allocator_realloc;
    allocator->base.free = th_main_allocator_free;
    allocator->allocator = parent;
    const size_t ptr_offset = TH_MAIN_ALLOCATOR_PTR_OFFSET;
    for (size_t i = 0; i < TH_MAIN_ALLOCATOR_BUCKET_NUM; ++i) {
        th_pool_allocator_init(&allocator->pool[i], parent, (1 << (i + 7)) + ptr_offset);
    }
}

TH_LOCAL(void)
th_main_allocator_deinit(th_main_allocator* allocator)
{
    for (size_t i = 0; i < TH_MAIN_ALLOCATOR_BUCKET_NUM; ++i) {
        th_pool_allocator_deinit(&allocator->pool[i]);
    }
}

struct th_server {
    th_context context;
    th_router router;
    th_fcache fcache;
    th_listener* listeners;
    th_allocator* allocator;
    th_main_allocator pool;
};

TH_LOCAL(th_err)
th_server_init(th_server* server, th_allocator* allocator)
{
    th_router_init(&server->router, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_context_init(&server->context, allocator)) != TH_ERR_OK)
        goto cleanup_router;
    th_fcache_init(&server->fcache, allocator);
    th_main_allocator_init(&server->pool, allocator);
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
    th_context_drain(&server->context);
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
    th_context_deinit(&server->context);
    th_router_deinit(&server->router);
    th_fcache_deinit(&server->fcache);
    th_main_allocator_deinit(&server->pool);
}

TH_LOCAL(th_err)
th_server_bind(th_server* server, const char* host, const char* port, th_bind_opt* opt)
{
    th_listener* listener = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_create(&listener, &server->context,
                                  host, port,
                                  &server->router, &server->fcache,
                                  opt, &server->pool.base))
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
    return th_router_add_route(&server->router, method, th_string_from_cstr(path), handler, user_data);
}

TH_LOCAL(th_err)
th_server_add_dir(th_server* server, const char* name, const char* path)
{
    return th_fcache_add_dir(&server->fcache, th_string_from_cstr(name), th_string_from_cstr(path));
}

TH_LOCAL(th_err)
th_server_poll(th_server* server, int timeout_ms)
{
    return th_context_poll(&server->context, timeout_ms);
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
    if ((err = th_ssl_context_init(&listener->ssl_context, key_file, cert_file)) != TH_ERR_OK) {
        return err;
    }
    listener->ssl_enabled = 1;
    return TH_ERR_OK;
#else
    (void)listener;
    (void)key_file;
    (void)cert_file;
    TH_LOG_ERROR("SSL is not not enabled in this build.");
    return TH_ERR_NOSUPPORT;
#endif
}

TH_LOCAL(th_err)
th_listener_init(th_listener* listener, th_context* context,
                 const char* host, const char* port,
                 th_router* router, th_fcache* fcache,
                 th_bind_opt* opt, th_allocator* allocator)
{
    listener->context = context;
    listener->running = 0;
    listener->ssl_enabled = 0;
    listener->allocator = allocator ? allocator : th_default_allocator_get();
    th_err err = TH_ERR_OK;
    if ((err = th_acceptor_init(&listener->acceptor, context, allocator, host, port)) != TH_ERR_OK)
        return err;
    if (opt && opt->key_file && opt->cert_file) {
        if ((err = th_listener_enable_ssl(listener, opt->key_file, opt->cert_file)) != TH_ERR_OK)
            goto cleanup_acceptor;
    }
    th_conn_tracker_init(&listener->conn_tracker);
    th_http_upgrader_init(&listener->upgrader, &listener->conn_tracker, router, fcache, allocator);
    TH_LOG_INFO("Created listener on %s:%s", host, port);
    return TH_ERR_OK;
cleanup_acceptor:
    th_acceptor_deinit(&listener->acceptor);
    return err;
}

TH_PRIVATE(th_err)
th_listener_create(th_listener** out, th_context* context,
                   const char* host, const char* port,
                   th_router* router, th_fcache* fcache,
                   th_bind_opt* opt, th_allocator* allocator)
{
    th_listener* listener = th_allocator_alloc(allocator, sizeof(th_listener));
    if (!listener)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_listener_init(listener, context, host, port, router, fcache, opt, allocator)) != TH_ERR_OK)
        goto cleanup;
    *out = listener;
    return TH_ERR_OK;
cleanup:
    th_allocator_free(allocator, listener);
    return err;
}

TH_LOCAL(th_err)
th_listener_async_accept(th_listener* listener)
{
    th_err err = TH_ERR_OK;
    if (!listener->ssl_enabled) {
        if ((err = th_tcp_conn_create(&listener->conn, listener->context,
                                      &listener->upgrader.base,
                                      (th_conn_observer*)&listener->conn_tracker,
                                      listener->allocator))
            != TH_ERR_OK) {
            return err;
        }
    } else {
#if TH_WITH_SSL
        if ((err = th_ssl_conn_create(&listener->conn, listener->context,
                                      &listener->ssl_context,
                                      &listener->upgrader.base,
                                      (th_conn_observer*)&listener->conn_tracker,
                                      listener->allocator))
            != TH_ERR_OK) {
            return err;
        }
#else
        TH_ASSERT(0 && "SSL is not enabled in this build.");
        return TH_ERR_NOSUPPORT;
#endif
    }
    th_acceptor_async_accept(&listener->acceptor,
                             th_conn_get_address(listener->conn),
                             &listener->accept_handler.base);
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
th_listener_accept_handler_fn(void* self, size_t result, th_err err)
{
    th_listener_accept_handler* handler = self;
    th_listener* listener = handler->listener;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("Accept failed: %s", th_strerror(err));
        th_conn_destroy(TH_MOVE_PTR(listener->conn));
    } else if (err == TH_ERR_OK) {
        th_socket_set_fd(th_conn_get_socket(listener->conn), (int)result);
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
    // Accept handler
    listener->accept_handler.listener = listener;
    th_io_handler_init(&listener->accept_handler.base, th_listener_accept_handler_fn, NULL);
    // Client destroy handler
    listener->client_destroy_handler.listener = listener;
    th_task_init(&listener->client_destroy_handler.base, th_listener_client_destroy_handler_fn, NULL);
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
    if (listener->ssl_enabled) {
        th_ssl_context_deinit(&listener->ssl_context);
    }
#endif /* TH_WITH_SSL */
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
th_route_init(th_route_segment* route, th_capture_type type, th_string segment, th_allocator* allocator)
{
    th_heap_string_init(&route->name, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_set(&route->name, segment)) != TH_ERR_OK) {
        th_heap_string_deinit(&route->name);
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
th_route_create(th_route_segment** out, th_capture_type type, th_string token, th_allocator* allocator)
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
    th_heap_string_deinit(&route->name);
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
th_route_consume_trail(th_route_segment* route, th_request* request, th_string* trail, bool dry, bool* result)
{
    th_string route_name = th_heap_string_view(&route->name);
    th_heap_string decoded = {0};
    th_heap_string_init(&decoded, route->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_url_decode_string(th_string_substr(*trail, 0, th_string_find_first_of(*trail, 0, "/?")), &decoded, TH_URL_DECODE_TYPE_PATH))
        != TH_ERR_OK) {
        goto cleanup;
    }
    th_string segment = th_heap_string_view(&decoded);
    // if (th_string_empty(segment) && route->type != TH_CAPTURE_TYPE_NONE)
    //     return false;
    switch (route->type) {
    case TH_CAPTURE_TYPE_NONE:
        if (th_string_eq(route_name, segment)) {
            *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
            *result = true;
        }
        break;
    case TH_CAPTURE_TYPE_INT:
        if (th_string_is_uint(segment)) {
            if (!dry)
                (void)th_request_add_pathvar(request, route_name, segment);
            *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
            *result = true;
        }
        break;
    case TH_CAPTURE_TYPE_STRING:
        if (!dry)
            (void)th_request_add_pathvar(request, route_name, segment);
        *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
        *result = true;
        break;
    case TH_CAPTURE_TYPE_PATH:
        if (!dry)
            (void)th_request_add_pathvar(request, route_name, *trail);
        *trail = th_string_make(NULL, 0);
        *result = true;
        break;
    default:
        break;
    }
cleanup:
    th_heap_string_deinit(&decoded);
    return err;
}

TH_LOCAL(th_err)
th_router_do_handle(th_router* router, th_method method, th_request* request, th_response* response, bool dry)
{
    TH_LOG_DEBUG("Handling request %p: %s", request, th_heap_string_data(&request->uri_path));
    if (*th_heap_string_at(&request->uri_path, 0) != '/')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string trail = th_string_substr(th_heap_string_view(&request->uri_path), 1, th_string_npos);
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
            if (th_string_empty(trail))
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
th_route_parse_trail(th_string* trail, th_string* name, th_capture_type* type)
{
    th_string segment = th_string_substr(*trail, 0, th_string_find_first_of(*trail, 0, "/"));
    size_t open_curly = th_string_find_first(segment, 0, '{');
    size_t close_curly = th_string_find_first(segment, 0, '}');
    if (segment.len > 2 && open_curly == 0 && close_curly == segment.len - 1) {
        th_string capture = th_string_substr(segment, 1, segment.len - 2);
        size_t sep = th_string_find_first(capture, 0, ':');
        if (sep == th_string_npos) {
            *name = capture;
            *type = TH_CAPTURE_TYPE_STRING;
        } else {
            th_string type_str = th_string_substr(capture, 0, sep);
            if (th_string_eq(type_str, TH_STRING("int"))) {
                *name = th_string_substr(capture, sep + 1, th_string_npos);
                *type = TH_CAPTURE_TYPE_INT;
            } else if (th_string_eq(type_str, TH_STRING("path"))) {
                *name = th_string_substr(capture, sep + 1, th_string_npos);
                *type = TH_CAPTURE_TYPE_PATH;
            } else {
                return TH_ERR_INVALID_ARG;
            }
        }
    } else if (open_curly == th_string_npos && close_curly == th_string_npos) {
        *name = segment;
        *type = TH_CAPTURE_TYPE_NONE;
    } else {
        return TH_ERR_INVALID_ARG;
    }
    // Consume segment
    *trail = th_string_substr(*trail, segment.len + 1, th_string_npos);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_router_add_route(th_router* router, th_method method, th_string path, th_handler handler, void* user_data)
{
    if (th_string_empty(path) || path.ptr[0] != '/')
        return TH_ERR_INVALID_ARG;
    th_string trail = th_string_substr(path, 1, th_string_npos);
    th_route_segment** list = &router->routes;
    th_route_segment* route = *list;

    // find a matching route
    bool last = false;
    while (!last) {
        th_string name = {0};
        th_capture_type type = TH_CAPTURE_TYPE_NONE;
        th_err err = TH_ERR_OK;
        if ((err = th_route_parse_trail(&trail, &name, &type)) != TH_ERR_OK)
            return err;
        last = th_string_empty(trail);
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
                 && th_string_eq(th_heap_string_view(&route->name), name))
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
/* ANSI-C code produced by gperf version 3.1 */
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
  static struct th_mime_mapping wordlist[] =
    {
      {""}, {""}, {""},
      {"ogv",  TH_STRING_INIT("video/ogg")},
      {"ico",  TH_STRING_INIT("image/x-icon")},
      {""}, {""}, {""},
      {"wav",  TH_STRING_INIT("audio/wav")},
      {"json", TH_STRING_INIT("application/json")},
      {"woff2",TH_STRING_INIT("font/woff2")},
      {""}, {""},
      {"ogg",  TH_STRING_INIT("audio/ogg")},
      {"opus", TH_STRING_INIT("audio/opus")},
      {""}, {""},
      {"js",   TH_STRING_INIT("text/javascript")},
      {"jpg",  TH_STRING_INIT("image/jpeg")},
      {"jpeg", TH_STRING_INIT("image/jpeg")},
      {""}, {""}, {""},
      {"svg",  TH_STRING_INIT("image/svg+xml")},
      {"weba", TH_STRING_INIT("audio/webm")},
      {""}, {""}, {""},
      {"otf",  TH_STRING_INIT("font/otf")},
      {"webp", TH_STRING_INIT("image/webp")},
      {""}, {""}, {""},
      {"png",  TH_STRING_INIT("image/png")},
      {"woff", TH_STRING_INIT("font/woff")},
      {""}, {""}, {""},
      {"gif",  TH_STRING_INIT("image/gif")},
      {"html", TH_STRING_INIT("text/html")},
      {""}, {""},
      {"md",   TH_STRING_INIT("text/markdown")},
      {"csv",  TH_STRING_INIT("text/csv")},
      {"avif", TH_STRING_INIT("image/avif")},
      {""}, {""}, {""},
      {"pdf",  TH_STRING_INIT("application/pdf")},
      {"webm", TH_STRING_INIT("video/webm")},
      {""}, {""}, {""},
      {"css",  TH_STRING_INIT("text/css")},
      {"mpeg", TH_STRING_INIT("video/mpeg")},
      {""}, {""}, {""},
      {"aac",  TH_STRING_INIT("audio/aac")},
      {""}, {""}, {""}, {""},
      {"ttf",  TH_STRING_INIT("font/ttf")},
      {""}, {""}, {""}, {""},
      {"xml",  TH_STRING_INIT("application/xml")},
      {""},
      {"xhtml",TH_STRING_INIT("application/xhtml+xml")},
      {""}, {""},
      {"txt",  TH_STRING_INIT("text/plain")},
      {""}, {""}, {""}, {""},
      {"zip",  TH_STRING_INIT("application/zip")},
      {""}, {""}, {""}, {""},
      {"mp4",  TH_STRING_INIT("video/mp4")},
      {""}, {""}, {""}, {""},
      {"mp3",  TH_STRING_INIT("audio/mpeg")}
    };

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
  return 0;
}

#pragma GCC diagnostic pop
/* End of src/th_mime.c */
/* Start of src/th_method.c */
/* ANSI-C code produced by gperf version 3.1 */
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
  return 0;
}

#pragma GCC diagnostic pop
/* End of src/th_method.c */
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
#elif defined(TH_CONFIG_OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(TH_CONFIG_OS_MOCK)
#endif

TH_PRIVATE(th_err)
th_acceptor_init(th_acceptor* acceptor, th_context* context,
                 th_allocator* allocator,
                 const char* addr, const char* port)
{
    acceptor->handle = NULL;
    acceptor->context = context;
    acceptor->allocator = allocator;
#if defined(TH_CONFIG_OS_POSIX)
    th_err err = TH_ERR_OK;
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo* res = NULL;
    if (getaddrinfo(addr, port, &hints, &res) != 0) {
        return TH_ERR_SYSTEM(errno);
    }
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
    // Set the socket to non-blocking mode
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if (bind(fd, res->ai_addr, res->ai_addrlen) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if (listen(fd, 1024) < 0) {
        err = TH_ERR_SYSTEM(errno);
        goto cleanup_fd;
    }
    if ((err = th_context_create_handle(context, &acceptor->handle, fd)) != TH_ERR_OK)
        goto cleanup_fd;
    freeaddrinfo(res);
    return TH_ERR_OK;
cleanup_fd:
    close(fd);
cleanup_addrinfo:
    freeaddrinfo(res);
    return err;
#elif defined(TH_CONFIG_OS_WIN)
    th_err err = TH_ERR_OK;
    const ADDRINFOA hints = {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    ADDRINFOA* res = NULL;
    if (getaddrinfo(addr, port, &hints, &res) != 0) {
        return TH_ERR_SYSTEM(WSAGetLastError());
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == INVALID_SOCKET) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_addrinfo;
    }
#if TH_CONFIG_REUSE_ADDR
    {
        int optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
            err = TH_ERR_SYSTEM(WSAGetLastError());
            goto cleanup_fd;
        }
    }
#endif
#if TH_CONFIG_REUSE_PORT
    {
        TH_LOG_FATAL("SO_REUSEPORT is not supported on this platform");
        err = TH_ERR_NOSUPPORT;
        goto cleanup_fd;
    }
#endif
    // Set the socket to non-blocking mode
    u_long mode = 1;
    if (ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_fd;
    }
    if (bind(fd, res->ai_addr, (int)res->ai_addrlen) == SOCKET_ERROR) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_fd;
    }
    if (listen(fd, 1024) == SOCKET_ERROR) {
        err = TH_ERR_SYSTEM(WSAGetLastError());
        goto cleanup_fd;
    }
    if ((err = th_context_create_handle(context, &acceptor->handle, fd)) != TH_ERR_OK)
        goto cleanup_fd;
    freeaddrinfo(res);
    return TH_ERR_OK;
cleanup_fd:
    closesocket(fd);
cleanup_addrinfo:
    freeaddrinfo(res);
    return err;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)addr;
    (void)port;
    int fd = th_mock_open();
    if (fd < 0)
        return TH_ERR_SYSTEM(-fd);
    th_err err = TH_ERR_OK;
    if ((err = th_context_create_handle(context, &acceptor->handle, fd)) != TH_ERR_OK)
        th_mock_close();
    return err;
#endif
}

TH_PRIVATE(void)
th_acceptor_async_accept(th_acceptor* acceptor, th_address* addr, th_io_handler* on_complete)
{
    th_address_init(addr);
    th_io_task* iot = th_io_task_create(acceptor->allocator);
    if (!iot) {
        th_context_dispatch_handler(acceptor->context, on_complete, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_accept(iot, th_io_handle_get_fd(acceptor->handle), &addr->addr, &addr->addrlen, on_complete);
    th_io_handle_submit(acceptor->handle, iot);
}

TH_PRIVATE(void)
th_acceptor_cancel(th_acceptor* acceptor)
{
    th_io_handle_cancel(acceptor->handle);
}

TH_PRIVATE(void)
th_acceptor_deinit(th_acceptor* acceptor)
{
    th_io_handle_destroy(acceptor->handle);
}

/* th_acceptor functions end */
/* End of src/th_acceptor.c */
/* Start of src/th_runner.c */

/* th_runner begin */

TH_PRIVATE(void)
th_runner_init(th_runner* runner)
{
    runner->queue = th_task_queue_make();
    runner->num_tasks = 0;
    runner->waiting = 0;
    th_task_queue_push(&runner->queue, &runner->service_task);
}

TH_PRIVATE(void)
th_runner_set_io_service(th_runner* runner, th_io_service* service)
{
    runner->io_service = service;
}

TH_PRIVATE(void)
th_runner_push_task(th_runner* runner, th_task* task)
{
    ++runner->num_tasks;
    th_task_queue_push(&runner->queue, task);
}

TH_PRIVATE(void)
th_runner_push_uncounted_task(th_runner* runner, th_task* task)
{
    th_task_queue_push(&runner->queue, task);
}

TH_PRIVATE(void)
th_runner_increase_task_count(th_runner* runner)
{
    ++runner->num_tasks;
}

TH_PRIVATE(th_err)
th_runner_poll(th_runner* runner, int timeout_ms)
{
    if (runner->num_tasks == 0) {
        return TH_ERR_EOF;
    }
    while (1) {
        th_task* task = th_task_queue_pop(&runner->queue);
        TH_ASSERT(task && "Task queue must never be empty");
        int empty = th_task_queue_empty(&runner->queue);
        if (task == &runner->service_task) {
            th_io_service_run(runner->io_service, empty ? timeout_ms : 0);
            th_task_queue_push(&runner->queue, &runner->service_task);
            if (empty)
                return TH_ERR_OK;
        } else {
            task->fn(task);
            if (task->destroy)
                task->destroy(task);
            --runner->num_tasks;
            return TH_ERR_OK;
        }
    }
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_runner_drain(th_runner* runner)
{
    th_task* task = NULL;
    while ((task = th_task_queue_pop(&runner->queue))) {
        if (task != &runner->service_task) {
            task->fn(task);
            if (task->destroy)
                task->destroy(task);
            --runner->num_tasks;
        }
    }
}

TH_PRIVATE(void)
th_runner_deinit(th_runner* runner)
{
    th_task* task = NULL;
    while ((task = th_task_queue_pop(&runner->queue))) {
        if (task->destroy)
            task->destroy(task);
    }
}

/* runner end */
/* End of src/th_runner.c */
/* Start of src/th_io_task.c */

#include <assert.h>
#include <errno.h>
#include <stdio.h>

TH_PRIVATE(void)
th_io_handler_fn(void* self)
{
    th_io_handler* handler = self;
    handler->fn(self, handler->result, handler->err);
}

TH_LOCAL(void)
th_io_task_destroy_impl(void* self)
{
    th_io_task* iot = self;
    if (iot->on_complete) {
        th_io_handler_destroy(iot->on_complete);
        iot->on_complete = NULL;
    }
    th_allocator_free(iot->allocator, iot);
}

TH_LOCAL(void)
th_io_task_fn(void* self)
{
    th_io_task* iot = self;
    size_t result = 0;
    th_err err = th_io_task_execute(iot, &result);
    if (iot->on_complete) {
        th_io_handler_complete(iot->on_complete, result, err);
    }
}

TH_PRIVATE(th_io_task*)
th_io_task_create(th_allocator* allocator)
{
    th_io_task* iot = th_allocator_alloc(allocator, sizeof(th_io_task));
    if (!iot)
        return NULL;
    th_task_init(&iot->base, th_io_task_fn, th_io_task_destroy_impl);
    iot->allocator = allocator;
    iot->on_complete = NULL;
    return iot;
}

/*
TH_PRIVATE(void)
th_io_task_to_string(char* buf, size_t len, th_io_task* iot)
{
    const char* op_str = NULL;
    switch (iot->op) {
    case TH_IO_OP_OPEN:
        op_str = "OPEN";
        break;
    case TH_IO_OP_OPENAT:
        op_str = "OPENAT";
        break;
    case TH_IO_OP_CLOSE:
        op_str = "CLOSE";
        break;
    case TH_IO_OP_READ:
        op_str = "READ";
        break;
    case TH_IO_OP_WRITE:
        op_str = "WRITE";
        break;
    case TH_IO_OP_WRITEV:
        op_str = "WRITEV";
        break;
    case TH_IO_OP_READV:
        op_str = "READV";
        break;
    case TH_IO_OP_SENDFILE:
        op_str = "SENDFILE";
        break;
    case TH_IO_OP_ACCEPT:
        op_str = "ACCEPT";
        break;
    default:
        op_str = "UNKNOWN";
        break;
    }
    snprintf(buf, len, "th_io_task(%s, fd=%d, fd2=%d, addr=%p, len=%u)", op_str, iot->fd, iot->fd2, iot->addr, (unsigned int)iot->len);
}
*/

TH_LOCAL(void)
th_io_task_prepare_read_write(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete, enum th_io_op op)
{
    iot->op = op;
    iot->fd = fd;
    iot->addr = addr;
    iot->len = len;
    iot->on_complete = on_complete;
}

TH_PRIVATE(void)
th_io_task_prepare_read(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_read;
    th_io_task_prepare_read_write(iot, fd, addr, len, on_complete, TH_IO_OP_READ);
}

/*
TH_PRIVATE(void)
th_io_task_prepare_write(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_write;
    th_io_task_prepare_read_write(iot, fd, addr, len, on_complete, TH_IO_OP_WRITE);
}

TH_PRIVATE(void)
th_io_task_prepare_writev(th_io_task* iot, int fd, th_iov* iov, size_t iovcnt, th_io_handler* on_complete)
{
    iot->fn = th_io_op_writev;
    th_io_task_prepare_read_write(iot, fd, iov, iovcnt, on_complete, TH_IO_OP_WRITEV);
}

*/

TH_PRIVATE(void)
th_io_task_prepare_send(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_send;
    th_io_task_prepare_read_write(iot, fd, addr, len, on_complete, TH_IO_OP_SEND);
}

TH_PRIVATE(void)
th_io_task_prepare_sendv(th_io_task* iot, int fd, th_iov* iov, size_t iovcnt, th_io_handler* on_complete)
{
    iot->fn = th_io_op_sendv;
    th_io_task_prepare_read_write(iot, fd, iov, iovcnt, on_complete, TH_IO_OP_SENDV);
}

TH_PRIVATE(void)
th_io_task_prepare_readv(th_io_task* iot, int fd, th_iov* iov, size_t iovcnt, th_io_handler* on_complete)
{
    iot->fn = th_io_op_readv;
    th_io_task_prepare_read_write(iot, fd, iov, iovcnt, on_complete, TH_IO_OP_READV);
}

TH_PRIVATE(void)
th_io_task_prepare_sendfile(th_io_task* iot, th_file* file, int sfd, th_iov* header, size_t iovcnt,
                            size_t offset, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_sendfile;
    iot->op = TH_IO_OP_SENDFILE;
    iot->fd = sfd;
    iot->addr2 = file;
    iot->addr = header;
    iot->len = iovcnt;
    iot->offset = offset;
    iot->len2 = len;
    iot->flags = 0;
    iot->on_complete = on_complete;
}

TH_PRIVATE(void)
th_io_task_prepare_accept(th_io_task* iot, int fd, void* addr, void* addrlen, th_io_handler* on_complete)
{
    iot->fn = th_io_op_accept;
    iot->op = TH_IO_OP_ACCEPT;
    iot->fd = fd;
    iot->addr = addr;
    iot->addr2 = addrlen;
    iot->on_complete = on_complete;
}

TH_PRIVATE(th_err)
th_io_task_execute(th_io_task* iot, size_t* result)
{
    return iot->fn(iot, result);
}

TH_PRIVATE(th_io_handler*)
th_io_task_try_execute(th_io_task* iot)
{
    size_t result = 0;
    th_err err = th_io_task_execute(iot, &result);
    if (err == TH_ERR_SYSTEM(TH_EAGAIN)
        || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK)) {
        return NULL;
    }
    th_io_handler* on_complete = TH_MOVE_PTR(iot->on_complete);
    th_task_destroy(&iot->base);
    th_io_handler_set_result(on_complete, result, err);
    return on_complete;
}

TH_PRIVATE(void)
th_io_task_destroy(th_io_task* iot)
{
    th_task_destroy(&iot->base);
}

TH_PRIVATE(th_io_handler*)
th_io_task_abort(th_io_task* iot, th_err err)
{
    th_io_handler* on_complete = TH_MOVE_PTR(iot->on_complete);
    th_io_handler_set_result(on_complete, 0, err);
    th_io_task_destroy(iot);
    return on_complete;
}
/* End of src/th_io_task.c */
/* Start of src/th_io_composite.c */

TH_PRIVATE(void)
th_io_composite_unref(void* self)
{
    th_io_composite* composite = self;
    TH_ASSERT(composite->refcount > 0 && "Invalid refcount");
    if (--composite->refcount == 0) {
        if (composite->on_complete)
            th_io_handler_destroy(TH_MOVE_PTR(composite->on_complete));
        composite->destroy(composite);
    }
}
/* End of src/th_io_composite.c */
/* Start of src/th_io_op_posix.c */


#if defined(TH_CONFIG_OS_POSIX)


#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#if defined(TH_CONFIG_OS_BSD)
#define CAST_MSG_IOVLEN(len) ((int)(len))
#else
#define CAST_MSG_IOVLEN(len) ((size_t)(len))
#endif

TH_PRIVATE(th_err)
th_io_op_posix_read(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    ssize_t ret = read(iot->fd, iot->addr, iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    else if (ret == 0)
        err = TH_ERR_EOF;
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_readv(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    ssize_t ret = readv(iot->fd, (struct iovec*)iov, (int)iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    else if (ret == 0)
        err = TH_ERR_EOF;
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_write(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    ssize_t ret = write(iot->fd, iot->addr, iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_writev(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    ssize_t ret = writev(iot->fd, (struct iovec*)iov, (int)iot->len);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_send(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    ssize_t ret = send(iot->fd, iot->addr, iot->len, flags);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_sendv(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = iot->addr;
    msg.msg_iovlen = CAST_MSG_IOVLEN(iot->len);
    ssize_t ret = sendmsg(iot->fd, &msg, flags);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_accept(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    int ret = accept(iot->fd, iot->addr, iot->addr2);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    (*result) = (size_t)ret;
    return err;
}

TH_PRIVATE(th_err)
th_io_op_posix_sendfile_mmap(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_file* file = iot->addr2;
    th_fileview view;
    if ((err = th_file_get_view(file, &view, iot->offset, iot->len2)) != TH_ERR_OK)
        return err;
    struct iovec vec[64];
    size_t veclen = 0;
    if (iot->len > 0) {
        th_iov* iov = iot->addr;
        for (size_t i = 0; i < iot->len; i++) {
            vec[i].iov_base = iov[i].base;
            vec[i].iov_len = iov[i].len;
            veclen++;
        }
    }
    vec[veclen].iov_base = view.ptr;
    vec[veclen].iov_len = view.len;
    veclen++;
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    struct msghdr msg = {0};
    msg.msg_iov = vec;
    msg.msg_iovlen = CAST_MSG_IOVLEN(veclen);
    ssize_t ret = sendmsg(iot->fd, &msg, flags);
    if (ret < 0)
        err = TH_ERR_SYSTEM(errno);
    *result = (size_t)ret;
    return err;
}

#define TH_IO_OP_POSIX_SENDFILE_BUFFERED_MAX 8 * 1024
TH_PRIVATE(th_err)
th_io_op_posix_sendfile_buffered(void* self, size_t* result)
{
    uint8_t buffer[TH_IO_OP_POSIX_SENDFILE_BUFFERED_MAX];
    th_io_task* iot = self;
    struct iovec vec[64];
    size_t veclen = 0;
    if (iot->len > 0) {
        th_iov* iov = iot->addr;
        for (size_t i = 0; i < iot->len; i++) {
            vec[i].iov_base = iov[i].base;
            vec[i].iov_len = iov[i].len;
            veclen++;
        }
    }
    size_t toread = TH_MIN(sizeof(buffer), iot->len2);
    ssize_t readlen = pread(((th_file*)iot->addr2)->fd, buffer, toread, (off_t)iot->offset);
    if (readlen < 0)
        return TH_ERR_SYSTEM(errno);
    int flags = 0;
#if defined(MSG_NOSIGNAL)
    flags |= MSG_NOSIGNAL;
#endif
    vec[veclen].iov_base = buffer;
    vec[veclen].iov_len = (size_t)readlen;
    veclen++;
    struct msghdr msg = {0};
    msg.msg_iov = vec;
    msg.msg_iovlen = CAST_MSG_IOVLEN(veclen);
    ssize_t writelen = sendmsg(iot->fd, &msg, flags);
    if (writelen < 0)
        return TH_ERR_SYSTEM(errno);
    *result = (size_t)writelen;
    return TH_ERR_OK;
}

#endif
/* End of src/th_io_op_posix.c */
/* Start of src/th_io_op_bsd.c */


#if defined(TH_CONFIG_WITH_BSD_SENDFILE)

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

TH_PRIVATE(th_err)
th_io_op_bsd_sendfile(void* self, size_t* result)
{
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    off_t len = (off_t)iot->len2;
    int ret = 0;
    if (iot->len == 0) {
        ret = sendfile(((th_file*)iot->addr2)->fd, iot->fd, (off_t)iot->offset, &len, NULL, 0);
    } else {
        struct sf_hdtr hdtr = {.headers = (struct iovec*)iov, .hdr_cnt = (int)iot->len, .trailers = NULL, .trl_cnt = 0};
        ret = sendfile(((th_file*)iot->addr2)->fd, iot->fd, (off_t)iot->offset, &len, &hdtr, 0);
    }
    th_err err = TH_ERR_OK;
    if (ret < 0 && len == 0) {
        int errc = errno;
        if (errc != TH_EAGAIN
            && errc != TH_EBUSY) {
            err = TH_ERR_SYSTEM(errc);
        }
    }
    *result = (size_t)len;
    return err;
}

#endif
/* End of src/th_io_op_bsd.c */
/* Start of src/th_io_op_linux.c */

#if defined(TH_CONFIG_WITH_LINUX_SENDFILE)
TH_PRIVATE(th_err)
th_io_op_linux_sendfile(void* self, size_t* result)
{
    (void)self;
    (void)result;
    return TH_ERR_NOSUPPORT;
}
#endif
/* End of src/th_io_op_linux.c */
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
/* th_pool_allocator implementation begin */

#define TH_POOL_ALLOCATOR_PTR_OFFSET TH_ALIGNUP(sizeof(th_pool_allocator_node), TH_ALIGNOF(th_max_align))
TH_LOCAL(void*)
th_pool_allocator_alloc(void* self, size_t size)
{
    th_pool_allocator* pool = self;
    (void)size;
    TH_ASSERT(size <= pool->block_size && "Invalid size");
    const size_t ptr_offset = TH_POOL_ALLOCATOR_PTR_OFFSET;
    th_pool_allocator_node* node = th_pool_allocator_list_pop_front(&pool->free_list);
    if (!node) {
        node = th_allocator_alloc(pool->allocator, ptr_offset + pool->block_size);
        if (!node)
            return NULL;
    }
    void* ptr = (char*)node + ptr_offset;
    th_pool_allocator_list_push_back(&pool->used_list, node);
    return ptr;
}

TH_LOCAL(void)
th_pool_allocator_free(void* self, void* ptr)
{
    th_pool_allocator* pool = self;
    const size_t ptr_offset = TH_POOL_ALLOCATOR_PTR_OFFSET;
    th_pool_allocator_node* node = (th_pool_allocator_node*)((char*)ptr - ptr_offset);
    th_pool_allocator_list_erase(&pool->used_list, node);
    th_pool_allocator_list_push_back(&pool->free_list, node);
}

TH_LOCAL(void*)
th_pool_allocator_realloc(void* self, void* ptr, size_t size)
{
    th_pool_allocator* pool = self;
    (void)pool;
    (void)size;
    TH_ASSERT(size <= pool->block_size && "Invalid size");
    return ptr;
}

TH_PRIVATE(void)
th_pool_allocator_init(th_pool_allocator* pool, th_allocator* allocator, size_t block_size)
{
    pool->base.alloc = th_pool_allocator_alloc;
    pool->base.realloc = th_pool_allocator_realloc;
    pool->base.free = th_pool_allocator_free;
    pool->allocator = allocator ? allocator : th_default_allocator_get();
    pool->block_size = block_size;
    pool->free_list = (th_pool_allocator_list){0};
    pool->used_list = (th_pool_allocator_list){0};
}

TH_PRIVATE(void)
th_pool_allocator_deinit(th_pool_allocator* pool)
{
    th_pool_allocator_node* node = NULL;
    while ((node = th_pool_allocator_list_pop_front(&pool->free_list))) {
        th_allocator_free(pool->allocator, node);
    }
    node = th_pool_allocator_list_pop_front(&pool->used_list);
    TH_ASSERT(node == NULL && "Memory leak detected");
}
/* End of src/th_allocator.c */
/* Start of src/th_task.c */

#include <assert.h>
#include <stdlib.h>

/* th_task functions begin */

TH_PRIVATE(void)
th_task_init(th_task* task, void (*fn)(void*), void (*destroy)(void*))
{
    TH_ASSERT(task);
    task->fn = fn;
    task->destroy = destroy;
    task->next = NULL;
}

TH_PRIVATE(void)
th_task_complete(th_task* task)
{
    if (task->fn)
        task->fn(task);
}

TH_PRIVATE(void)
th_task_destroy(th_task* task)
{
    if (task->destroy)
        task->destroy(task);
}

/* th_task functions end */
/* End of src/th_task.c */
/* Start of src/th_kqueue_service.c */

#ifdef TH_CONFIG_WITH_KQUEUE

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "kqueue_service"

/* th_kqueue_handle forward declarations begin */

TH_LOCAL(void)
th_kqueue_handle_init(th_kqueue_handle* handle, th_kqueue_service* service, int fd, th_allocator* allocator);

TH_LOCAL(void)
th_kqueue_handle_do_cancel(th_kqueue_handle* handle, th_err reason);

/* th_kqueue_handle forward declarations end */
/* th_kqueue_task_dispatcher implementation begin */

TH_LOCAL(const char*)
th_kqueue_fitler_to_string(int filter) TH_MAYBE_UNUSED;

TH_LOCAL(const char*)
th_kqueue_flags_to_string(int flags) TH_MAYBE_UNUSED;

TH_LOCAL(const char*)
th_kqueue_fitler_to_string(int filter)
{
    switch (filter) {
    case EVFILT_READ:
        return "EVFILT_READ";
    case EVFILT_WRITE:
        return "EVFILT_WRITE";
    case EVFILT_TIMER:
        return "EVFILT_TIMER";
    default:
        return "UNKNOWN";
    }
}

TH_LOCAL(const char*)
th_kqueue_flags_to_string(int flags)
{
    static char buf[256];
    buf[0] = '[';
    buf[1] = '\0';
    if (flags & EV_ADD)
        strncat(buf, "EV_ADD ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_DELETE)

        strncat(buf, "EV_DELETE ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_ENABLE)
        strncat(buf, "EV_ENABLE ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_DISABLE)
        strncat(buf, "EV_DISABLE ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_ONESHOT)
        strncat(buf, "EV_ONESHOT ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_CLEAR)
        strncat(buf, "EV_CLEAR ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_EOF)
        strncat(buf, "EV_EOF ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_ERROR)
        strncat(buf, "EV_ERROR ", sizeof(buf) - strlen(buf) - 1);
    if (flags & EV_OOBAND)
        strncat(buf, "EV_OOBAND ", sizeof(buf) - strlen(buf) - 1);
    strncat(buf, "]", sizeof(buf) - strlen(buf) - 1);
    return buf;
}

TH_LOCAL(void)
th_kqueue_service_run(void* self, int timeout_ms)
{
    th_kqueue_service* service = self;

    static const int max_events = 128;
    struct kevent evlist[max_events] = {0};

    struct timespec timeout = {
        .tv_sec = timeout_ms / 1000,
        .tv_nsec = (timeout_ms % 1000) * 1000000,
    };

    int nev = kevent(service->kq, NULL, 0, evlist, max_events, timeout_ms == -1 ? NULL : &timeout);
    if (nev == -1) {
        TH_LOG_ERROR("kevent failed: %s", strerror(errno));
        return;
    }
    for (int i = 0; i < nev; ++i) {
        TH_LOG_TRACE("kevent: fd=%d, filter=%s, flags=%s, data=%d",
                     (int)evlist[i].ident, th_kqueue_fitler_to_string(evlist[i].filter),
                     th_kqueue_flags_to_string(evlist[i].flags), (int)evlist[i].data);

        th_kqueue_handle* handle = evlist[i].udata;
        th_io_op_type op_type = TH_IO_OP_TYPE_NONE;
        switch (evlist[i].filter) {
        case EVFILT_READ:
            op_type = TH_IO_OP_TYPE_READ;
            break;
        case EVFILT_WRITE:
            op_type = TH_IO_OP_TYPE_WRITE;
            break;
        default:
            TH_ASSERT(0 && "Invalid filter");
            break;
        }
        int idx = (int)(op_type - 1);
        if (handle->iot[idx]) {
            if (evlist[i].flags & EV_ERROR) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(TH_MOVE_PTR(handle->iot[idx]), TH_ERR_SYSTEM(errno)));
            } else if (evlist[i].flags & EV_EOF && evlist[i].data == 0) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(TH_MOVE_PTR(handle->iot[idx]), TH_ERR_EOF));
            } else {
                th_runner_push_uncounted_task(service->runner, (th_task*)TH_MOVE_PTR(handle->iot[idx]));
            }
            if (handle->timeout_enabled)
                th_kqueue_timer_list_erase(&service->timer_list, handle);
        }
    }
    th_kqueue_handle* handle = NULL;
    while ((handle = th_kqueue_timer_list_front(&service->timer_list)) != NULL) {
        if (!th_timer_expired(&handle->timer))
            break;
        (void)th_kqueue_timer_list_pop_front(&service->timer_list);
        th_kqueue_handle_do_cancel(handle, TH_ERR_SYSTEM(TH_ETIMEDOUT));
    }
}

TH_LOCAL(void)
th_kqueue_service_deinit(th_kqueue_service* service)
{
    th_kqueue_handle_pool_deinit(&service->handle_allocator);
    close(service->kq);
}

TH_LOCAL(void)
th_kqueue_service_destroy(void* self)
{
    th_kqueue_service* service = self;
    th_kqueue_service_deinit(service);
    th_allocator_free(service->allocator, service);
}

TH_LOCAL(th_err)
th_kqueue_service_create_handle(void* self, th_io_handle** out, int fd)
{
    th_kqueue_service* service = self;
    th_kqueue_handle* handle = th_allocator_alloc(&service->handle_allocator.base, sizeof(th_kqueue_handle));
    if (!handle)
        return TH_ERR_SYSTEM(errno);
    th_kqueue_handle_init(handle, service, fd, &service->handle_allocator.base);
    *out = (th_io_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_kqueue_service_init(th_kqueue_service* service, th_runner* runner, th_allocator* allocator)
{
    service->base.create_handle = th_kqueue_service_create_handle;
    service->base.run = th_kqueue_service_run;
    service->base.destroy = th_kqueue_service_destroy;
    service->allocator = allocator;
    service->runner = runner;
    service->timer_list = (th_kqueue_timer_list){0};
    if ((service->kq = kqueue()) == -1) {
        return TH_ERR_SYSTEM(errno);
    }
    th_kqueue_handle_pool_init(&service->handle_allocator, service->allocator, 16, TH_CONFIG_MAX_HANDLES);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_kqueue_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_kqueue_service* service = th_allocator_alloc(allocator, sizeof(th_kqueue_service));
    if (!service)
        return TH_ERR_SYSTEM(errno);
    th_err err = th_kqueue_service_init(service, runner, allocator);
    if (err != TH_ERR_OK) {
        th_allocator_free(allocator, service);
        return err;
    }
    *out = (th_io_service*)service;
    return TH_ERR_OK;
}

/* th_kqueue_task_dispatcher implementation end */
/* th_kqueue_handle implementation begin */

TH_LOCAL(void)
th_kqueue_handle_do_cancel(th_kqueue_handle* handle, th_err reason)
{
    th_io_task* iot[TH_IO_OP_TYPE_MAX] = {0};
    size_t count = 0;
    for (int i = 0; i < TH_IO_OP_TYPE_MAX; ++i) {
        if (handle->iot[i]) {
            iot[count++] = TH_MOVE_PTR(handle->iot[i]);
        }
    }
    for (size_t i = 0; i < count; ++i) {
        th_runner_push_uncounted_task(handle->service->runner, (th_task*)th_io_task_abort(iot[i], reason));
    }
}
TH_LOCAL(void)
th_kqueue_handle_cancel(void* self)
{
    th_kqueue_handle* handle = self;
    th_kqueue_handle_do_cancel(handle, TH_ERR_SYSTEM(TH_ECANCELED));
}

TH_LOCAL(void)
th_kqueue_handle_submit(void* self, th_io_task* task)
{
    th_kqueue_handle* handle = self;
    th_io_op_type op_type = TH_IO_OP_TYPE(task->op);
    th_io_handler* on_complete = th_io_task_try_execute(task);
    if (on_complete) {
        th_runner_push_task(handle->service->runner, (th_task*)on_complete);
        return;
    }

    if ((handle->active & op_type) == 0) {

        struct kevent ev = {0};
        switch (op_type) {
        case TH_IO_OP_TYPE_READ:
            EV_SET(&ev, handle->fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, handle);
            break;
        case TH_IO_OP_TYPE_WRITE:
            EV_SET(&ev, handle->fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, handle);
            break;
        default:
            TH_ASSERT(0 && "Invalid op type");
            break;
        }
        if (kevent(handle->service->kq, &ev, 1, NULL, 0, NULL) == -1) {
            th_runner_push_task(handle->service->runner, (th_task*)th_io_task_abort(task, TH_ERR_SYSTEM(errno)));
            return;
        }
        handle->active |= op_type;
    }
    if (handle->timeout_enabled) {
        th_err err = th_timer_set(&handle->timer, th_seconds(TH_CONFIG_IO_TIMEOUT));
        if (err != TH_ERR_OK) {
            TH_LOG_ERROR("Failed to set timer: %s, disabling timeout", th_strerror(err));
            handle->timeout_enabled = false;
        } else {
            th_kqueue_timer_list_push_back(&handle->service->timer_list, handle);
        }
    }

    th_runner_increase_task_count(handle->service->runner);
    handle->iot[op_type - 1] = task;
}

TH_LOCAL(int)
th_kqueue_handle_get_fd(void* self)
{
    th_kqueue_handle* handle = self;
    return handle->fd;
}

TH_LOCAL(void)
th_kqueue_handle_enable_timeout(void* self, bool enabled)
{
    th_kqueue_handle* handle = self;
    handle->timeout_enabled = enabled;
}

TH_LOCAL(void)
th_kqueue_handle_destroy(void* self)
{
    th_kqueue_handle* handle = self;
    th_kqueue_handle_cancel(handle);
    close(handle->fd);
    th_allocator_free(handle->allocator, handle);
}

TH_LOCAL(void)
th_kqueue_handle_init(th_kqueue_handle* handle, th_kqueue_service* service, int fd, th_allocator* allocator)
{
    handle->base.cancel = th_kqueue_handle_cancel;
    handle->base.submit = th_kqueue_handle_submit;
    handle->base.enable_timeout = th_kqueue_handle_enable_timeout;
    handle->base.get_fd = th_kqueue_handle_get_fd;
    handle->base.destroy = th_kqueue_handle_destroy;
    handle->allocator = allocator;
    handle->iot[TH_IO_OP_TYPE_READ - 1] = NULL;
    handle->iot[TH_IO_OP_TYPE_WRITE - 1] = NULL;
    handle->service = service;
    handle->fd = fd;
    handle->active = TH_IO_OP_TYPE_NONE;
    th_timer_init(&handle->timer);
}

/* th_kqueue_handle implementation end */

#endif /* TH_HAVE_KQUEUE */
/* End of src/th_kqueue_service.c */
/* Start of src/th_poll_service.c */

#ifdef TH_CONFIG_WITH_POLL

#include <errno.h>
#include <string.h>
#include <unistd.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "poll_service"

/* th_poll_handle_map implementation begin */

TH_LOCAL(void)
th_poll_handle_map_init(th_poll_handle_map* map, th_allocator* allocator)
{
    th_fd_to_idx_map_init(&map->fd_to_idx_map, allocator);
    map->allocator = (allocator) ? allocator : th_default_allocator_get();
    map->handles = NULL;
    map->size = 0;
    map->capacity = 0;
}

TH_LOCAL(void)
th_poll_handle_map_deinit(th_poll_handle_map* map)
{
    th_fd_to_idx_map_deinit(&map->fd_to_idx_map);
    th_allocator_free(map->allocator, map->handles);
}

/** th_poll_handle_map_set
 * @brief Sets the poll handle for the given file descriptor.
 */
TH_LOCAL(void)
th_poll_handle_map_set(th_poll_handle_map* map, int fd, th_poll_handle* handle)
{
    size_t idx = 0;
    th_fd_to_idx_map_iter iter = th_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
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
        th_fd_to_idx_map_set(&map->fd_to_idx_map, fd, idx);
    } else {
        idx = iter->value;
    }
    map->handles[idx] = handle;
}

/* th_poll_handle_map_try_get
 * @brief Get the poll handle for the given file descriptor.
 * @param map The handle map.
 * @param fd The file descriptor.
 * @return The poll handle, NULL if the handle wasn't found.
 */
TH_LOCAL(th_poll_handle*)
th_poll_handle_map_try_get(th_poll_handle_map* map, int fd)
{
    th_poll_handle* handle = NULL;
    th_fd_to_idx_map_iter iter = th_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    if (iter) {
        handle = map->handles[iter->value];
    }
    return handle;
}

TH_LOCAL(void)
th_poll_handle_map_remove(th_poll_handle_map* map, int fd)
{
    th_fd_to_idx_map_iter iter = th_fd_to_idx_map_find(&map->fd_to_idx_map, fd);
    TH_ASSERT(iter && "Must not remove a non-existent handle");
    if (iter) {
        size_t idx = iter->value;
        th_fd_to_idx_map_erase(&map->fd_to_idx_map, iter);
        if (idx != map->size - 1) {
            th_fd_to_idx_map_iter last = th_fd_to_idx_map_find(&map->fd_to_idx_map, map->handles[map->size - 1]->fd);
            last->value = idx;
            map->handles[idx] = map->handles[map->size - 1];
        }
        --map->size;
    }
}

/* th_poll_handle_map implementation end */
/* th_poll_handle implementation begin */

TH_LOCAL(void)
th_poll_handle_submit(void* self, th_io_task* task)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    th_poll_service* service = handle->service;
    th_io_handler* on_complete = th_io_task_try_execute(task);
    if (on_complete) {
        th_runner_push_task(service->runner, (th_task*)on_complete);
        return;
    }
    th_io_op_type op_type = TH_IO_OP_TYPE(task->op);
    handle->iot[op_type - 1] = task;
    struct pollfd pfd = {.fd = handle->fd, .events = 0};
    switch (op_type) {
    case TH_IO_OP_TYPE_READ:
        pfd.events = POLLIN;
        break;
    case TH_IO_OP_TYPE_WRITE:
        pfd.events = POLLOUT;
        break;
    default:
        TH_ASSERT(0 && "Invalid operation");
        break;
    }
    if (handle->timeout_enabled) {
        th_err err = th_timer_set(&handle->timer, th_seconds(TH_CONFIG_IO_TIMEOUT));
        if (err != TH_ERR_OK) {
            TH_LOG_ERROR("Failed to set timer: %s, disabling timeout", th_strerror(err));
            handle->timeout_enabled = false;
        }
    }
    th_err err = TH_ERR_OK;
    if ((err = th_pollfd_vec_push_back(&service->fds, pfd)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to push back pollfd");
        th_runner_push_task(service->runner, (th_task*)th_io_task_abort(task, err));
        return;
    }
    th_runner_increase_task_count(service->runner);
}

TH_LOCAL(void)
th_poll_handle_cancel(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
    for (int i = 0; i < TH_IO_OP_TYPE_MAX; ++i) {
        th_io_task* iot = handle->iot[i];
        if (iot) {
            handle->iot[i] = NULL;
            th_runner_push_uncounted_task(handle->service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_ECANCELED)));
        }
    }
}

TH_LOCAL(int)
th_poll_handle_get_fd(void* self)
{
    th_poll_handle* handle = (th_poll_handle*)self;
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
    th_poll_handle_map_remove(&handle->service->handles, handle->fd);
    close(handle->fd);
    th_allocator_free(handle->allocator, handle);
}

TH_LOCAL(void)
th_poll_handle_init(th_poll_handle* handle, th_poll_service* service, int fd, th_allocator* allocator)
{
    handle->base.submit = th_poll_handle_submit;
    handle->base.cancel = th_poll_handle_cancel;
    handle->base.destroy = th_poll_handle_destroy;
    handle->base.get_fd = th_poll_handle_get_fd;
    handle->base.enable_timeout = th_poll_handle_enable_timeout;
    th_timer_init(&handle->timer);
    handle->iot[TH_IO_OP_TYPE_READ - 1] = NULL;
    handle->iot[TH_IO_OP_TYPE_WRITE - 1] = NULL;
    handle->allocator = allocator;
    handle->service = service;
    handle->fd = fd;
    handle->timeout_enabled = false;
}

/* th_poll_handle implementation end */
/* th_poll_service implementation begin */

TH_LOCAL(th_err)
th_poll_service_create_handle(void* self, th_io_handle** out, int fd)
{
    th_poll_service* service = (th_poll_service*)self;
    th_poll_handle* handle = th_poll_handle_pool_alloc(&service->handle_allocator, sizeof(th_poll_handle));
    if (!handle) {
        return TH_ERR_BAD_ALLOC;
    }
    th_poll_handle_init(handle, service, fd, &service->handle_allocator.base);
    th_poll_handle_map_set(&service->handles, handle->fd, handle);
    *out = (th_io_handle*)handle;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_poll_service_run(void* self, int timeout_ms)
{
    th_poll_service* service = (th_poll_service*)self;
    nfds_t nfds = (nfds_t)th_pollfd_vec_size(&service->fds);
    int ret = poll(th_pollfd_vec_begin(&service->fds), nfds, timeout_ms);
    if (ret <= 0) {
        if (ret == -1)
            TH_LOG_WARN("poll failed: %s", strerror(errno));
        return;
    }

    size_t reenqueue = 0;
    for (size_t i = 0; i < nfds; ++i) {
        th_poll_handle* handle = th_poll_handle_map_try_get(&service->handles, th_pollfd_vec_at(&service->fds, i)->fd);
        if (!handle) // handle was removed
            continue;
        short revents = th_pollfd_vec_at(&service->fds, i)->revents;
        short events = th_pollfd_vec_at(&service->fds, i)->events & (POLLIN | POLLOUT);
        int op_index = 0;
        switch (events) {
        case POLLIN:
            op_index = TH_IO_OP_TYPE_READ - 1;
            break;
        case POLLOUT:
            op_index = TH_IO_OP_TYPE_WRITE - 1;
            break;
        default:
            TH_LOG_ERROR("Unknown poll event: %d", events);
            continue;
            break;
        }
        th_io_task* iot = handle->iot[op_index];
        if (revents && iot) {
            if (revents & events) {
                th_runner_push_uncounted_task(service->runner, (th_task*)iot);
            } else if (revents & POLLHUP) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_EOF));
            } else if (revents & (POLLERR | POLLPRI)) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_EIO)));
            } else if (revents & POLLNVAL) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_EBADF)));
            } else {
                TH_LOG_ERROR("[th_poll_service] Unknown poll event: %d", revents);
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_UNKNOWN));
            }
            handle->iot[op_index] = NULL;
        } else if (iot) { // reenqueue
            if (handle->timeout_enabled && th_timer_expired(&handle->timer)) {
                th_runner_push_uncounted_task(service->runner, (th_task*)th_io_task_abort(iot, TH_ERR_SYSTEM(TH_ETIMEDOUT)));
                handle->iot[op_index] = NULL;
            } else {
                if (reenqueue < i)
                    *th_pollfd_vec_at(&service->fds, reenqueue) = *th_pollfd_vec_at(&service->fds, i);
                ++reenqueue;
            }
        }
        // handles without iot were cancelled, so we don't need to reenqueue them
    }
    th_pollfd_vec_resize(&service->fds, reenqueue);
    return;
}

TH_LOCAL(void)
th_poll_service_deinit(th_poll_service* service)
{
    th_poll_handle_map_deinit(&service->handles);
    th_poll_handle_pool_deinit(&service->handle_allocator);
    th_pollfd_vec_deinit(&service->fds);
}

TH_LOCAL(void)
th_poll_service_destroy(void* self)
{
    th_poll_service* service = (th_poll_service*)self;
    th_poll_service_deinit(service);
    th_allocator_free(service->allocator, service);
}

TH_LOCAL(th_err)
th_poll_service_init(th_poll_service* service, th_runner* runner, th_allocator* allocator)
{
    service->base.run = th_poll_service_run;
    service->base.destroy = th_poll_service_destroy;
    service->base.create_handle = th_poll_service_create_handle;
    service->allocator = allocator;
    service->runner = runner;
    th_pollfd_vec_init(&service->fds, allocator);
    th_poll_handle_map_init(&service->handles, allocator);
    th_poll_handle_pool_init(&service->handle_allocator, allocator, 16, 8 * 1024);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_poll_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_poll_service* service = (th_poll_service*)th_allocator_alloc(allocator, sizeof(th_poll_service));
    if (!service) {
        return TH_ERR_BAD_ALLOC;
    }
    memset(service, 0, sizeof(th_poll_service));
    th_err err = TH_ERR_OK;
    if ((err = th_poll_service_init(service, runner, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, service);
        return err;
    }
    *out = &service->base;
    return TH_ERR_OK;
}

#endif /* TH_CONFIG_WITH_POLL */
/* End of src/th_poll_service.c */
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
    }
    return "Unknown error category";
}
/* End of src/th_error.c */
/* Start of src/th_socket.c */

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

/* th_address functions begin */

TH_PRIVATE(void)
th_address_init(th_address* addr)
{
    addr->addrlen = sizeof(addr->addr);
}

/* th_address functions end */
/* generic socket functions begin */

typedef struct th_socket_exact_task_handler {
    th_io_composite base;
    th_allocator* allocator;
    th_socket* socket;
    void* addr;
    size_t remaining;
    size_t len;
} th_socket_exact_task_handler;

TH_LOCAL(void)
th_socket_exact_task_handler_destroy(void* self)
{
    th_socket_exact_task_handler* handler = self;
    th_allocator_free(handler->allocator, handler);
}

TH_LOCAL(void)
th_socket_exact_task_handler_complete(th_socket_exact_task_handler* handler, size_t len, th_err err)
{
    th_io_composite_complete(&handler->base, len, err);
}

/* th_socket_write_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_write_exact_handler;
#define th_socket_write_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_write_exact_handler_destroy th_socket_exact_task_handler_destroy

TH_LOCAL(void)
th_socket_write_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_write_exact_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_write_exact_handler_complete(handler, handler->len - handler->remaining, err);
        return;
    }
    handler->remaining -= len;
    if (handler->remaining == 0) {
        th_socket_write_exact_handler_complete(handler, handler->len, err);
        return;
    }
    th_socket_async_write(handler->socket, (uint8_t*)handler->addr + handler->len - handler->remaining, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_write_exact_handler_create(th_socket_write_exact_handler** out, th_allocator* allocator,
                                     th_socket* socket, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_socket_write_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_write_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_write_exact_handler_fn, th_socket_write_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = addr;
    handler->remaining = len;
    handler->len = len;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_write_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_write_exact_handler* handler = NULL;
    if ((err = th_socket_write_exact_handler_create(&handler, th_socket_get_allocator(sock),
                                                    sock, addr, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_write(sock, addr, len, (th_io_handler*)handler);
}

/* th_socket_write_exact implementation end */
/* th_socket_writev_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_writev_exact_handler;
#define th_socket_writev_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_writev_exact_handler_destroy th_socket_exact_task_handler_destroy

/** th_socket_writev_exact_handler_fn
 * @brief For each write, shifts the iov array and increases the len by the number of bytes written.
 * The remaining parameter is decremented by the number of buffers consumed.
 */
TH_LOCAL(void)
th_socket_writev_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_exact_task_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_exact_task_handler_complete(handler, handler->len, err);
        return;
    }
    handler->len += len;
    th_iov* iov = handler->addr;
    th_iov_consume(&iov, &handler->remaining, len);
    if (handler->remaining == 0) {
        th_socket_exact_task_handler_complete(handler, handler->len, err);
        return;
    }
    handler->addr = iov;
    th_socket_async_writev(handler->socket, handler->addr, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_writev_exact_handler_create(th_socket_writev_exact_handler** out, th_allocator* allocator,
                                      th_socket* socket, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_socket_writev_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_writev_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_writev_exact_handler_fn, th_socket_writev_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = iov;
    handler->remaining = len;
    handler->len = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_writev_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_writev_exact_handler* handler = NULL;
    if ((err = th_socket_writev_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, iov, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_writev(sock, iov, len, (th_io_handler*)handler);
}

/* th_socket_writev_exact implementation end */
/* th_socket_read_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_read_exact_handler;
#define th_socket_read_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_read_exact_handler_destroy th_socket_exact_task_handler_destroy

TH_LOCAL(void)
th_socket_read_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_exact_task_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_read_exact_handler_complete(handler, handler->len - handler->remaining, err);
        return;
    }
    handler->remaining -= len;
    if (handler->remaining == 0) {
        th_socket_read_exact_handler_complete(handler, handler->len, err);
        return;
    }
    th_socket_async_read(handler->socket, (uint8_t*)handler->addr + handler->len - handler->remaining, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_read_exact_handler_create(th_socket_read_exact_handler** out, th_allocator* allocator,
                                    th_socket* socket, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_socket_read_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_read_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_read_exact_handler_fn, th_socket_read_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = addr;
    handler->remaining = len;
    handler->len = len;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_read_exact(th_socket* sock, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_read_exact_handler* handler = NULL;
    if ((err = th_socket_read_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, addr, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_read(sock, addr, len, (th_io_handler*)handler);
}

/* th_socket_read_exact implementation end */
/* th_socket_readv_exact implementation begin */

typedef th_socket_exact_task_handler th_socket_readv_exact_handler;
#define th_socket_readv_exact_handler_complete th_socket_exact_task_handler_complete
#define th_socket_readv_exact_handler_destroy th_socket_exact_task_handler_destroy

TH_LOCAL(void)
th_socket_readv_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_exact_task_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_readv_exact_handler_complete(handler, handler->len, err);
        return;
    }
    handler->len += len;
    th_iov* iov = handler->addr;
    th_iov_consume(&iov, &handler->remaining, len);
    if (handler->remaining == 0) {
        th_socket_readv_exact_handler_complete(handler, handler->len, err);
        return;
    }
    handler->addr = iov;
    th_socket_async_readv(handler->socket, handler->addr, handler->remaining, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(th_err)
th_socket_readv_exact_handler_create(th_socket_readv_exact_handler** out, th_allocator* allocator,
                                     th_socket* socket, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_socket_readv_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_readv_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_readv_exact_handler_fn, th_socket_readv_exact_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->addr = iov;
    handler->remaining = len;
    handler->len = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_readv_exact(th_socket* sock, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_socket_readv_exact_handler* handler = NULL;
    if ((err = th_socket_readv_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, iov, len, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    th_socket_async_readv(sock, iov, len, (th_io_handler*)handler);
}

/* th_socket_readv_exact implementation end */
/* th_socket_sendfile_exact implementation begin */

typedef struct th_socket_sendfile_exact_handler {
    th_io_composite base;
    th_socket* socket;
    th_file* fstream;
    th_iov* iov;
    size_t iovcnt;
    size_t offset;
    size_t slen;
    size_t vlen;
    size_t relative_offset;
} th_socket_sendfile_exact_handler;

TH_LOCAL(void)
th_socket_sendfile_exact_handler_complete(th_socket_sendfile_exact_handler* handler, size_t len, th_err err)
{
    th_io_composite_complete(&handler->base, len, err);
}

TH_LOCAL(void)
th_socket_sendfile_exact_handler_fn(void* self, size_t len, th_err err)
{
    th_socket_sendfile_exact_handler* handler = self;
    if (err != TH_ERR_OK) {
        th_socket_sendfile_exact_handler_complete(handler, 0, err);
        return;
    }
    if (handler->iovcnt > 0) {
        handler->relative_offset += th_iov_consume(&handler->iov, (size_t*)&handler->iovcnt, len);
    } else {
        handler->relative_offset += len;
    }
    if (handler->relative_offset == handler->slen) {
        th_socket_sendfile_exact_handler_complete(handler, handler->relative_offset + handler->vlen, err);
        return;
    }
    size_t remaining = handler->slen - handler->relative_offset;
    size_t chunk = remaining > TH_CONFIG_SENDFILE_CHUNK_LEN ? TH_CONFIG_SENDFILE_CHUNK_LEN : remaining;
    th_socket_async_sendfile(handler->socket, handler->iov, handler->iovcnt, handler->fstream,
                             handler->offset + handler->relative_offset, chunk, (th_io_handler*)th_io_composite_ref(&handler->base));
}

TH_LOCAL(void)
th_socket_sendfile_exact_handler_destroy(void* self)
{
    th_socket_sendfile_exact_handler* handler = self;
    th_allocator_free(th_socket_get_allocator(handler->socket), handler);
}

TH_LOCAL(th_err)
th_socket_sendfile_exact_handler_create(th_socket_sendfile_exact_handler** out, th_allocator* allocator,
                                        th_socket* socket, th_iov* iov, size_t iovcnt, th_file* stream,
                                        size_t offset, size_t slen, size_t vlen, th_socket_handler* on_complete)
{
    th_socket_sendfile_exact_handler* handler = th_allocator_alloc(allocator, sizeof(th_socket_sendfile_exact_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_io_composite_init(&handler->base, th_socket_sendfile_exact_handler_fn, th_socket_sendfile_exact_handler_destroy, on_complete);
    handler->socket = socket;
    handler->iov = iov;
    handler->iovcnt = iovcnt;
    handler->fstream = stream;
    handler->offset = offset;
    handler->slen = slen;
    handler->vlen = vlen;
    handler->relative_offset = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_socket_async_sendfile_exact(th_socket* sock, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t slen, th_socket_handler* on_complete)
{
    size_t vlen = th_iov_bytes(iov, iovcnt);
    th_err err = TH_ERR_OK;
    th_socket_sendfile_exact_handler* handler = NULL;
    if ((err = th_socket_sendfile_exact_handler_create(&handler, th_socket_get_allocator(sock), sock, iov, iovcnt, stream, offset, slen, vlen, on_complete))
        != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(sock), on_complete, 0, err);
        return;
    }
    size_t chunk = slen > TH_CONFIG_SENDFILE_CHUNK_LEN ? TH_CONFIG_SENDFILE_CHUNK_LEN : slen;
    th_socket_async_sendfile(sock, iov, iovcnt, stream, offset, chunk, (th_io_handler*)handler);
}

/* th_socket_sendfile_exact implementation end */
/* generic socket functions end */
/* End of src/th_socket.c */
/* Start of src/th_tcp_socket.c */

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

/* th_tcp_socket functions begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(void)
th_tcp_socket_set_fd_options(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1
        || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        TH_LOG_WARN("Failed to set non-blocking: %s", th_strerror(TH_ERR_SYSTEM(errno)));
    }
    int optval = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
        TH_LOG_WARN("Failed to disable nagle: %s", th_strerror(TH_ERR_SYSTEM(errno)));
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
        TH_LOG_WARN("Failed to enable keepalive: %s", th_strerror(TH_ERR_SYSTEM(errno)));
}
#elif defined(TH_CONFIG_OS_WIN)
TH_LOCAL(void)
th_tcp_socket_set_fd_options(int fd)
{
    u_long mode = 1;
    if (ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR)
        TH_LOG_WARN("Failed to set non-blocking: %s", th_strerror(TH_ERR_SYSTEM(WSAGetLastError())));
    int optval = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        TH_LOG_WARN("Failed to disable nagle: %s", th_strerror(TH_ERR_SYSTEM(WSAGetLastError())));
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
        TH_LOG_WARN("Failed to enable keepalive: %s", th_strerror(TH_ERR_SYSTEM(WSAGetLastError())));
}
#else
TH_LOCAL(void)
th_tcp_socket_set_fd_options(int fd)
{
    (void)fd;
}
#endif

TH_LOCAL(void)
th_tcp_socket_set_fd_impl(void* self, int fd)
{
    th_tcp_socket* sock = self;
    if (sock->handle) {
        th_io_handle_destroy(sock->handle);
        sock->handle = NULL;
    }
    th_tcp_socket_set_fd_options(fd);
    th_context_create_handle(sock->context, &sock->handle, fd);
    th_io_handle_enable_timeout(sock->handle, true);
}

TH_LOCAL(void)
th_tcp_socket_cancel_impl(void* self)
{
    th_tcp_socket* sock = self;
    if (sock->handle)
        th_io_handle_cancel(sock->handle);
}

TH_LOCAL(th_allocator*)
th_tcp_socket_get_allocator_impl(void* self)
{
    th_tcp_socket* sock = self;
    return sock->allocator;
}

TH_LOCAL(th_context*)
th_tcp_socket_get_context_impl(void* self)
{
    th_tcp_socket* sock = self;
    return sock->context;
}

TH_LOCAL(void)
th_tcp_socket_async_write_impl(void* self, void* addr, size_t len, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_send(iot, th_io_handle_get_fd(sock->handle), addr, len, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_writev_impl(void* self, th_iov* iov, size_t iovcnt, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_sendv(iot, th_io_handle_get_fd(sock->handle), iov, iovcnt, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_read_impl(void* self, void* addr, size_t len, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_read(iot, th_io_handle_get_fd(sock->handle), addr, len, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_readv_impl(void* self, th_iov* iov, size_t iovcnt, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_readv(iot, th_io_handle_get_fd(sock->handle), iov, iovcnt, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_LOCAL(void)
th_tcp_socket_async_sendfile_impl(void* self, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_io_handler* handler)
{
    th_tcp_socket* sock = self;
    th_io_task* iot = th_io_task_create(sock->allocator);
    if (!iot) {
        th_context_dispatch_handler(sock->context, handler, 0, TH_ERR_BAD_ALLOC);
        return;
    }
    th_io_task_prepare_sendfile(iot, stream, th_io_handle_get_fd(sock->handle), iov, iovcnt, offset, len, handler);
    th_io_handle_submit(sock->handle, iot);
}

TH_PRIVATE(void)
th_tcp_socket_init(th_tcp_socket* sock, th_context* context, th_allocator* allocator)
{
    static const th_socket_methods methods = {
        .set_fd = th_tcp_socket_set_fd_impl,
        .cancel = th_tcp_socket_cancel_impl,
        .get_allocator = th_tcp_socket_get_allocator_impl,
        .get_context = th_tcp_socket_get_context_impl,
        .async_write = th_tcp_socket_async_write_impl,
        .async_writev = th_tcp_socket_async_writev_impl,
        .async_read = th_tcp_socket_async_read_impl,
        .async_readv = th_tcp_socket_async_readv_impl,
        .async_sendfile = th_tcp_socket_async_sendfile_impl,
    };
    sock->base.methods = &methods;
    sock->handle = NULL;
    sock->context = context;
    sock->allocator = allocator ? allocator : th_default_allocator_get();
}

TH_PRIVATE(void)
th_tcp_socket_close(th_tcp_socket* sock)
{
    if (sock->handle) {
        th_io_handle_destroy(sock->handle);
        sock->handle = NULL;
    }
}

TH_PRIVATE(void)
th_tcp_socket_deinit(th_tcp_socket* sock)
{
    if (sock->handle)
        th_tcp_socket_close(sock);
}

/* th_socket functions end */
/* End of src/th_tcp_socket.c */
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
th_request_parser_do_cookie(th_request* request, th_string cookie)
{
    size_t eq = th_string_find_first(cookie, 0, '=');
    if (eq == th_string_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    th_string key = th_string_trim(th_string_substr(cookie, 0, eq));
    th_string value = th_string_trim(th_string_substr(cookie, eq + 1, cookie.len));
    th_err err = TH_ERR_OK;
    if ((err = th_request_add_cookie(request, key, value)) != TH_ERR_OK) {
        return err;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_cookie_list(th_request* request, th_string cookie_list)
{
    size_t start = 0;
    size_t pos = 0;
    while (pos != th_string_npos) {
        pos = th_string_find_first(cookie_list, start, ';');
        th_string cookie = th_string_trim(th_string_substr(cookie_list, start, pos - start));
        th_err err = th_request_parser_do_cookie(request, cookie);
        if (err != TH_ERR_OK) {
            return err;
        }
        start = pos + 1;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_next_queryvar(th_string string, size_t* pos, th_string* key, th_string* value)
{
    size_t eq = th_string_find_first(string, *pos, '=');
    if (eq == th_string_npos) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    *key = th_string_trim(th_string_substr(string, *pos, eq - *pos));
    *pos = th_string_find_first(string, eq + 1, '&');
    if (*pos != th_string_npos) {
        *value = th_string_trim(th_string_substr(string, eq + 1, *pos - eq - 1));
        (*pos)++;
        return TH_ERR_OK;
    } else {
        *value = th_string_trim(th_string_substr(string, eq + 1, *pos));
        return TH_ERR_OK;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_bodyvars(th_request* request, th_string body)
{
    th_err err = TH_ERR_OK;
    size_t pos = 0;
    while (pos != th_string_npos) {
        th_string key;
        th_string value;
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
th_request_parser_next_token(th_string buffer, th_string* token, char until, size_t* parsed)
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
    *token = th_string_substr(buffer, 0, i);
    *parsed = i + 1;
    return TH_ERR_OK;
}

TH_LOCAL(bool)
th_request_parser_is_printable_string(th_string input)
{
    for (size_t i = 0; i < input.len; i++) {
        if (input.ptr[i] < 32 || input.ptr[i] > 126) {
            return false;
        }
    }
    return true;
}

TH_LOCAL(th_err)
th_request_parser_do_method(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed_out)
{
    th_string method;
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
th_request_parser_do_uri_query(th_request* request, th_string path)
{
    size_t pos = 0;
    while (pos != th_string_npos) {
        th_string key;
        th_string value;
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
th_request_parser_next_path_segment(th_string buffer, th_string* segment, size_t* parsed)
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
    *segment = th_string_substr(buffer, 0, i);
    *parsed = i + 1;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_path(th_request_parser* parser, th_request* request, th_string path, size_t* parsed)
{
    th_string segment;
    size_t uri_parsed = 0;
    th_err err = th_request_parser_next_path_segment(path, &segment, &uri_parsed);
    if (err != TH_ERR_OK || uri_parsed == 0)
        return err;
    if ((err = th_request_set_uri_path(request, segment)) != TH_ERR_OK)
        return err;
    if (segment.ptr[segment.len] == '?') { // got a query
        size_t query_parsed = 0;
        err = th_request_parser_next_path_segment(th_string_substr(path, uri_parsed, th_string_npos), &segment, &query_parsed);
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
        if ((err = th_request_set_uri_query(request, TH_STRING(""))) != TH_ERR_OK)
            return err;
    }
    *parsed = uri_parsed;
    parser->state = TH_REQUEST_PARSER_STATE_VERSION;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_version(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    size_t n = th_string_find_first(buffer, 0, '\r');
    if (n == th_string_npos || n + 1 == buffer.len)
        return TH_ERR_OK;
    if (buffer.ptr[n + 1] != '\n')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string version = th_string_substr(buffer, 0, n);
    if (version.len < 8)
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
th_request_parse_handle_header(th_request_parser* parser, th_request* request, th_string name, th_string value)
{
    char arena[1024] = {0};
    th_arena_allocator arena_allocator;
    th_arena_allocator_init(&arena_allocator, arena, sizeof(arena), NULL);
    th_heap_string normalized_name;
    th_heap_string_init(&normalized_name, &arena_allocator.base);
    if (th_heap_string_set(&normalized_name, name) != TH_ERR_OK) {
        // This can only happen if the name is too long
        return TH_ERR_HTTP(TH_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    th_heap_string_to_lower(&normalized_name);
    th_header_id id = th_header_id_from_string(th_heap_string_data(&normalized_name), th_heap_string_len(&normalized_name));
    switch (id) {
    case TH_HEADER_ID_COOKIE:
        return th_request_parser_do_cookie_list(request, value);
    case TH_HEADER_ID_CONTENT_LENGTH:
        return th_string_to_uint(value, (unsigned*)&parser->content_len);
    case TH_HEADER_ID_CONNECTION:
        if (th_string_eq(value, TH_STRING("close"))) {
            request->close = true;
        } else if (th_string_eq(value, TH_STRING("keep-alive"))) {
            request->close = false;
        }
        return TH_ERR_OK;
    case TH_HEADER_ID_CONTENT_TYPE:
        if (th_string_eq(value, TH_STRING("application/x-www-form-urlencoded"))) {
            parser->body_encoding = TH_REQUEST_BODY_ENCODING_FORM_URL_ENCODED;
        } else if (th_string_eq(th_string_substr(value, 0, 19), TH_STRING("multipart/form-data"))) {
            parser->body_encoding = TH_REQUEST_BODY_ENCODING_MULTIPART_FORM_DATA;
        }
        break;
    default:
        break;
    }
    return th_request_add_header(request, th_heap_string_view(&normalized_name), value);
}

TH_LOCAL(th_err)
th_request_parser_parse_header_line(th_string line, th_string* out_name, th_string* out_value)
{
    th_err err = TH_ERR_OK;
    size_t parsed = 0;
    if ((err = th_request_parser_next_token(line, out_name, ':', &parsed)) != TH_ERR_OK)
        return err;
    if (parsed == 0)
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string header_value = th_string_substr(line, parsed, th_string_npos);
    if (!th_request_parser_is_printable_string(header_value))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *out_value = th_string_trim(header_value);
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_header(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    size_t n = th_string_find_first(buffer, 0, '\r');
    if (n == th_string_npos || n + 1 == buffer.len)
        return TH_ERR_OK;
    if (buffer.ptr[n + 1] != '\n')
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (n == 0) {
        *parsed = 2;
        if (parser->content_len == 0) {
            th_request_set_body(request, th_string_make(&buffer.ptr[2], 0));
            parser->state = TH_REQUEST_PARSER_STATE_DONE;
        } else {
            if (request->method == TH_METHOD_GET || request->method == TH_METHOD_HEAD)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
            parser->state = TH_REQUEST_PARSER_STATE_BODY;
        }
        return TH_ERR_OK;
    }
    size_t key_parsed = 0;
    th_string key;
    th_err err = TH_ERR_OK;
    if ((err = th_request_parser_next_token(buffer, &key, ':', &key_parsed)) != TH_ERR_OK
        || key_parsed == 0)
        return err;
    th_string value = th_string_substr(buffer, key_parsed, n - key_parsed);
    if (!th_request_parser_is_printable_string(value))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if ((err = th_request_parse_handle_header(parser, request, th_string_trim(key), th_string_trim(value)))
        != TH_ERR_OK)
        return err;
    *parsed = n + 2;
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_request_parser_multipart_find_eol(th_string buffer, size_t start)
{
    size_t pos = start;
    while (pos + 1 < buffer.len) {
        if (buffer.ptr[pos] == '\r' && buffer.ptr[pos + 1] == '\n')
            return pos;
        pos++;
    }
    return th_string_npos;
}

TH_LOCAL(bool)
th_request_parser_multipart_is_boundary_line(th_string line, th_string boundary, bool* last)
{
    *last = false;
    if (line.len < boundary.len + 2)
        return false;
    if (line.ptr[0] != '-' || line.ptr[1] != '-')
        return false;
    if (th_string_eq(th_string_substr(line, 2, boundary.len), boundary)) {
        if (line.len == boundary.len + 2)
            return true;
        if (line.ptr[boundary.len + 2] == '-' && line.ptr[boundary.len + 3] == '-') {
            *last = true;
            return true;
        }
    }
    return false;
}

TH_LOCAL(th_err)
th_request_parser_multipart_next_header_param(th_string buffer, th_string* out_name, th_string* out_value, size_t* out_parsed)
{
    // skip leading spaces
    buffer = th_string_substr(buffer, th_string_find_first_not(buffer, 0, ' '), th_string_npos);
    size_t eq = th_string_find_first_of(buffer, 0, "=; ");
    if (eq == th_string_npos || buffer.ptr[eq] == ';') {
        *out_name = th_string_substr(buffer, 0, eq);
        *out_value = th_string_make_empty();
        *out_parsed = eq == th_string_npos ? buffer.len : eq + 1;
        return TH_ERR_OK;
    }
    if (buffer.ptr[eq] == ' ') // spaces are not allowed
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    *out_name = th_string_substr(buffer, 0, eq);
    size_t parsed = eq + 1;
    buffer = th_string_substr(buffer, eq + 1, th_string_npos);
    if (th_string_empty(buffer)) // equals sign must be followed by a value
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    if (buffer.ptr[0] == '"') {
        size_t end = th_string_find_first(buffer, 1, '"');
        if (end == th_string_npos) // no closing quote
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *out_value = th_string_substr(buffer, 1, end - 1);
        parsed += (end == th_string_npos ? buffer.len : end + 1);
    } else {
        size_t end = th_string_find_first_of(buffer, 0, "; ");
        if (end != th_string_npos && buffer.ptr[end] == ' ')
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        *out_value = th_string_substr(buffer, 0, end);
        parsed += (end == th_string_npos ? buffer.len : end + 1);
    }
    *out_parsed = parsed;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_multipart_content_disposition(th_string header_value, th_string* out_name, th_string* out_filename)
{
    // skip heading
    header_value = th_string_substr(header_value, th_string_find_first(header_value, 0, ';') + 1, th_string_npos);
    // parse the parameters
    while (!th_string_empty(header_value)) {
        th_err err = TH_ERR_OK;
        th_string name, value = th_string_make_empty();
        size_t parsed = 0;
        if ((err = th_request_parser_multipart_next_header_param(header_value, &name, &value, &parsed)) != TH_ERR_OK)
            return err;
        header_value = th_string_substr(header_value, parsed, th_string_npos);
        if (th_string_eq(name, TH_STRING("name"))) {
            *out_name = value;
        } else if (th_string_eq(name, TH_STRING("filename"))) {
            *out_filename = value;
        }
    }
    return TH_ERR_OK;
}

TH_LOCAL(size_t)
th_request_parser_multipart_find_boundary(th_string buffer, th_string boundary, bool* last, size_t* length)
{
    TH_ASSERT(length && "lenght pointer must not be NULL");
    size_t pos = 0;
    while (1) {
        size_t eol = th_request_parser_multipart_find_eol(buffer, pos);
        if (eol == th_string_npos)
            return th_string_npos;
        th_string line = th_string_substr(buffer, pos, eol - pos);
        if (th_request_parser_multipart_is_boundary_line(line, boundary, last)) {
            *length = line.len;
            break;
        }
        pos = eol + 2;
    }
    return pos;
}

TH_LOCAL(th_err)
th_request_parser_multipart_do_next(th_request* request, th_string buffer, th_string boundary, size_t* out_parsed)
{
    th_string content_disposition, content_type;
    content_disposition = content_type = th_string_make_empty();
    size_t content_len = th_string_npos;
    size_t original_len = buffer.len;
    // parse the headers
    while (1) {
        if (th_string_empty(buffer))
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        size_t line_length = th_request_parser_multipart_find_eol(buffer, 0);
        if (line_length == th_string_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        th_string line = th_string_substr(buffer, 0, line_length);
        if (th_string_empty(line)) {
            buffer = th_string_substr(buffer, line_length + 2, th_string_npos);
            break; // end of headers
        }
        th_string header_name;
        th_string header_value;
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_parse_header_line(line, &header_name, &header_value)) != TH_ERR_OK)
            return err;
        if (th_string_eq(header_name, TH_STRING("Content-Disposition"))) {
            content_disposition = header_value;
        } else if (th_string_eq(header_name, TH_STRING("Content-Length"))) {
            if (th_string_to_uint(header_value, (unsigned*)&content_len) != TH_ERR_OK)
                return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        } else if (th_string_eq(header_name, TH_STRING("Content-Type"))) {
            content_type = header_value;
        }
        buffer = th_string_substr(buffer, line_length + 2, th_string_npos);
    }
    if (th_string_empty(content_disposition))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    th_string name, filename;
    name = filename = th_string_make_empty();
    th_err err = TH_ERR_OK;
    if ((err = th_request_parser_multipart_content_disposition(content_disposition, &name, &filename)) != TH_ERR_OK)
        return err;
    if (th_string_empty(name))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    bool last = false;
    th_string content = th_string_make_empty();
    if (content_len != th_string_npos) {
        content = th_string_substr(buffer, 0, content_len);
        buffer = th_string_substr(buffer, content_len, th_string_npos);
        // check the boundary
        if (buffer.ptr[0] != '\r' || buffer.ptr[1] != '\n')
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        th_string line = th_string_substr(buffer, 2, th_request_parser_multipart_find_eol(buffer, 0));
        if (!th_request_parser_multipart_is_boundary_line(line, boundary, &last))
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        buffer = th_string_substr(buffer, content_len + boundary.len + 2, th_string_npos);
    } else {
        // we don't have the content length, so we need to find the boundary
        size_t boundary_length = 0;
        size_t pos = th_request_parser_multipart_find_boundary(buffer, boundary, &last, &boundary_length);
        if (pos == th_string_npos)
            return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
        content = th_string_substr(buffer, 0, pos - 2); // -2 to remove the \r\n
        buffer = th_string_substr(buffer, pos + boundary_length + 2, th_string_npos);
    }
    if (last && !th_string_empty(buffer)) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    if (th_string_empty(filename)) {
        if (th_request_add_formvar(request, name, content) != TH_ERR_OK)
            return TH_ERR_BAD_ALLOC;
    } else {
        if (th_request_add_upload(request, content, name, filename, content_type) != TH_ERR_OK)
            return TH_ERR_BAD_ALLOC;
    }
    *out_parsed = original_len - buffer.len;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parsed_multipart_parse_content_type(th_string content_type, th_string* boundary)
{
    // skip heading
    content_type = th_string_substr(content_type, th_string_find_first(content_type, 0, ';') + 1, th_string_npos);
    while (!th_string_empty(content_type)) {
        th_string name, value = th_string_make_empty();
        size_t parsed = 0;
        th_err err = TH_ERR_OK;
        if ((err = th_request_parser_multipart_next_header_param(content_type, &name, &value, &parsed)) != TH_ERR_OK)
            return err;
        content_type = th_string_substr(content_type, parsed, th_string_npos);
        if (th_string_eq(name, TH_STRING("boundary"))) {
            *boundary = value;
            return TH_ERR_OK;
        }
    }
    return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
}

TH_LOCAL(th_err)
th_request_parser_do_multipart_form_data(th_request* request, th_string body)
{
    // first, read the boundary
    th_string content_type = th_request_get_header(request, TH_STRING("content-type"));
    if (th_string_empty(content_type)) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    th_err err = TH_ERR_OK;
    th_string boundary = th_string_make_empty();
    if ((err = th_request_parsed_multipart_parse_content_type(content_type, &boundary)) != TH_ERR_OK)
        return err;
    if (th_string_empty(boundary))
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    // parse the body
    // first, find the first boundary
    bool last = false;
    size_t pos = th_request_parser_multipart_find_eol(body, 0);
    if (!th_request_parser_multipart_is_boundary_line(th_string_substr(body, 0, pos), boundary, &last)
        || last) {
        return TH_ERR_HTTP(TH_CODE_BAD_REQUEST);
    }
    body = th_string_substr(body, pos + 2, th_string_npos);
    do {
        size_t parsed = 0;
        if ((err = th_request_parser_multipart_do_next(request, body, boundary, &parsed)) != TH_ERR_OK) {
            return err;
        }
        body = th_string_substr(body, parsed, th_string_npos);
    } while (!th_string_empty(body));
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_request_parser_do_body(th_request_parser* parser, th_request* request, th_string buffer, size_t* parsed)
{
    if (buffer.len < parser->content_len) {
        *parsed = 0;
        return TH_ERR_OK;
    }
    // Got the whole body
    th_string body = th_string_substr(buffer, 0, parser->content_len);
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
th_request_parser_parse_next(th_request_parser* parser, th_request* request, th_string data, size_t* parsed)
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
th_request_parser_parse(th_request_parser* parser, th_request* request, th_string data, size_t* parsed)
{
    th_err err = TH_ERR_OK;
    while (data.len > 0) {
        size_t p = 0;
        if ((err = th_request_parser_parse_next(parser, request, th_string_substr(data, p, data.len), &p)) != TH_ERR_OK) {
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
    return th_heap_string_data(&((const th_hstr_pair*)it->ptr)->key);
}

TH_INLINE(const void*)
th_hstr_iter_val(const th_iter* it)
{
    return th_heap_string_data(&((const th_hstr_pair*)it->ptr)->value);
}

static th_iter_methods th_hstr_iter_methods = {
    .next = th_hstr_iter_next,
    .key = th_hstr_iter_key,
    .val = th_hstr_iter_val,
};

// hstr iterator end
// upload iterator begin

TH_INLINE(bool)
th_upload_iter_next(th_iter* it)
{
    it->ptr = ((const th_upload*)it->ptr) + 1;
    return it->ptr < it->end;
}

TH_INLINE(const char*)
th_upload_iter_key(const th_iter* it)
{
    return th_heap_string_data(&((const th_upload*)it->ptr)->name);
}

TH_INLINE(const void*)
th_upload_iter_val(const th_iter* it)
{
    return it->ptr;
}

static th_iter_methods th_upload_iter_methods = {
    .next = th_upload_iter_next,
    .key = th_upload_iter_key,
    .val = th_upload_iter_val,
};

// upload iterator end

TH_LOCAL(th_err)
th_request_map_store(th_request* request, th_hstr_vec* vec, th_string key, th_string value)
{
    th_err err = TH_ERR_OK;
    th_heap_string k;
    th_heap_string v;
    if ((err = th_heap_string_init_with(&k, key, request->allocator)) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_init_with(&v, value, request->allocator)) != TH_ERR_OK)
        goto cleanup_key;
    if ((err = th_hstr_vec_push_back(vec, (th_hstr_pair){k, v})) != TH_ERR_OK)
        goto cleanup_value;
    return TH_ERR_OK;
cleanup_value:
    th_heap_string_deinit(&v);
cleanup_key:
    th_heap_string_deinit(&k);
    return err;
}

TH_LOCAL(th_err)
th_request_map_store_url_decoded(th_request* request, th_hstr_vec* vec, th_string key, th_string value, th_url_decode_type type)
{
    th_err err = TH_ERR_OK;
    th_heap_string k;
    th_heap_string v;
    th_heap_string_init(&k, request->allocator);
    th_heap_string_init(&v, request->allocator);
    if ((err = th_url_decode_string(key, &k, type)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_url_decode_string(value, &v, type)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_hstr_vec_push_back(vec, (th_hstr_pair){k, v})) != TH_ERR_OK)
        goto cleanup;
    return TH_ERR_OK;
cleanup:
    th_heap_string_deinit(&v);
    th_heap_string_deinit(&k);
    return err;
}

TH_PRIVATE(th_err)
th_request_add_cookie(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->cookies, key, value);
}

TH_PRIVATE(th_err)
th_request_add_header(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->headers, key, value);
}

TH_PRIVATE(th_err)
th_request_add_upload(th_request* request, th_string data, th_string name, th_string filename, th_string content_type)
{
    th_upload upload;
    th_upload_init(&upload, data, request->fcache, request->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_upload_set_name(&upload, name)) != TH_ERR_OK)
        goto cleanup_upload;
    if ((err = th_upload_set_filename(&upload, filename)) != TH_ERR_OK)
        goto cleanup_upload;
    if ((err = th_upload_set_content_type(&upload, content_type)) != TH_ERR_OK)
        goto cleanup_upload;
    if ((err = th_upload_vec_push_back(&request->uploads, upload)) != TH_ERR_OK)
        goto cleanup_upload;
    return TH_ERR_OK;
cleanup_upload:
    th_upload_deinit(&upload);
    return err;
}

TH_PRIVATE(th_err)
th_request_add_queryvar(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->queryvars, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_formvar(th_request* request, th_string key, th_string value)
{
    return th_request_map_store_url_decoded(request, &request->formvars, key, value, TH_URL_DECODE_TYPE_QUERY);
}

TH_PRIVATE(th_err)
th_request_add_pathvar(th_request* request, th_string key, th_string value)
{
    return th_request_map_store(request, &request->pathvars, key, value);
}

TH_PRIVATE(th_err)
th_request_set_uri_path(th_request* request, th_string path)
{
    return th_heap_string_set(&request->uri_path, path);
}

TH_PRIVATE(th_err)
th_request_set_uri_query(th_request* request, th_string query)
{
    return th_heap_string_set(&request->uri_query, query);
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
th_request_set_body(th_request* request, th_string body)
{
    request->body = body;
}

TH_PRIVATE(void)
th_request_init(th_request* request, th_fcache* fcache, th_allocator* allocator)
{
    request->allocator = allocator ? allocator : th_default_allocator_get();
    request->fcache = fcache;
    th_heap_string_init(&request->uri_path, request->allocator);
    th_heap_string_init(&request->uri_query, request->allocator);
    th_upload_vec_init(&request->uploads, request->allocator);
    th_hstr_vec_init(&request->cookies, request->allocator);
    th_hstr_vec_init(&request->headers, request->allocator);
    th_hstr_vec_init(&request->queryvars, request->allocator);
    th_hstr_vec_init(&request->formvars, request->allocator);
    th_hstr_vec_init(&request->pathvars, request->allocator);
    request->body = (th_string){0};
    request->version = 0;
    request->close = false;
}

TH_PRIVATE(void)
th_request_deinit(th_request* request)
{
    th_heap_string_deinit(&request->uri_path);
    th_heap_string_deinit(&request->uri_query);
    th_upload_vec_deinit(&request->uploads);
    th_hstr_vec_deinit(&request->cookies);
    th_hstr_vec_deinit(&request->headers);
    th_hstr_vec_deinit(&request->queryvars);
    th_hstr_vec_deinit(&request->formvars);
    th_hstr_vec_deinit(&request->pathvars);
}

TH_PRIVATE(void)
th_request_reset(th_request* request)
{
    th_heap_string_clear(&request->uri_path);
    th_heap_string_clear(&request->uri_query);
    th_upload_vec_clear(&request->uploads);
    th_hstr_vec_clear(&request->cookies);
    th_hstr_vec_clear(&request->headers);
    th_hstr_vec_clear(&request->queryvars);
    th_hstr_vec_clear(&request->formvars);
    th_hstr_vec_clear(&request->pathvars);
    request->body = (th_string){0};
    request->version = 0;
    request->close = false;
}

TH_LOCAL(th_string)
th_request_vec_get(th_hstr_vec* vec, th_string key)
{
    size_t num = th_hstr_vec_size(vec);
    for (size_t i = 0; i < num; i++) {
        if (th_heap_string_eq(&vec->data[i].key, key))
            return th_heap_string_view(&vec->data[i].value);
    }
    return TH_STRING("");
}

TH_PRIVATE(th_string)
th_request_get_header(th_request* request, th_string key)
{
    return th_request_vec_get(&request->headers, key);
}

TH_PRIVATE(th_string)
th_request_get_pathvar(th_request* request, th_string key)
{
    return th_request_vec_get(&request->pathvars, key);
}

TH_PRIVATE(th_string)
th_request_get_queryvar(th_request* request, th_string key)
{
    return th_request_vec_get(&request->queryvars, key);
}

TH_PRIVATE(th_string)
th_request_get_formvar(th_request* request, th_string key)
{
    return th_request_vec_get(&request->formvars, key);
}

TH_PRIVATE(th_upload*)
th_request_get_upload(th_request* request, th_string key)
{
    size_t num = th_upload_vec_size(&request->uploads);
    for (size_t i = 0; i < num; i++) {
        if (th_heap_string_eq(&request->uploads.data[i].name, key))
            return th_upload_vec_at(&request->uploads, i);
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
    return th_heap_string_data(&req->uri_path);
}

TH_PUBLIC(const char*)
th_get_query(const th_request* req)
{
    return th_heap_string_data(&req->uri_query);
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
        if (strncmp(key, th_heap_string_data(&req->headers.data[i].key), th_heap_string_len(&req->headers.data[i].key)) == 0) {
            return th_heap_string_data(&req->headers.data[i].value);
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
        if (strncmp(key, th_heap_string_data(&req->cookies.data[i].key), th_heap_string_len(&req->cookies.data[i].key)) == 0) {
            return th_heap_string_data(&req->cookies.data[i].value);
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
        if (strncmp(key, th_heap_string_data(&req->queryvars.data[i].key), th_heap_string_len(&req->queryvars.data[i].key)) == 0) {
            return th_heap_string_data(&req->queryvars.data[i].value);
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
        if (strncmp(key, th_heap_string_data(&req->formvars.data[i].key), th_heap_string_len(&req->formvars.data[i].key)) == 0) {
            return th_heap_string_data(&req->formvars.data[i].value);
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
        if (strncmp(key, th_heap_string_data(&req->pathvars.data[i].key), th_heap_string_len(&req->pathvars.data[i].key)) == 0) {
            return th_heap_string_data(&req->pathvars.data[i].value);
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

TH_PUBLIC(const th_upload*)
th_find_upload(const th_request* req, const char* name)
{
    size_t num = th_upload_vec_size(&req->uploads);
    for (size_t i = 0; i < num; i++) {
        if (strncmp(name, th_heap_string_data(&req->uploads.data[i].name), th_heap_string_len(&req->uploads.data[i].name)) == 0) {
            return th_upload_vec_cat(&req->uploads, i);
        }
    }
    return NULL;
}

TH_PUBLIC(th_iter)
th_upload_iter(const th_request* req)
{
    return (th_iter){
        .methods = &th_upload_iter_methods,
        .ptr = req->uploads.data,
        .end = req->uploads.data + req->uploads.size,
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
th_response_init(th_response* response, th_fcache* fcache, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_heap_string_init(&response->headers, allocator);
    th_heap_string_init(&response->body, allocator);
    response->iov[0] = (th_iov){0};
    response->iov[1] = (th_iov){0};
    response->iov[2] = (th_iov){0};
    response->allocator = allocator;
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
    th_heap_string_deinit(&response->headers);
    th_heap_string_deinit(&response->body);
    if (response->fcache_entry) {
        th_fcache_entry_unref(response->fcache_entry);
        response->fcache_entry = NULL;
    }
}

TH_PRIVATE(void)
th_response_reset(th_response* response)
{
    th_heap_string_clear(&response->headers);
    th_heap_string_clear(&response->body);
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
th_response_add_header(th_response* response, th_string key, th_string value)
{
    th_header_id header_id = th_header_id_from_string(key.ptr, key.len);
    if (header_id != TH_HEADER_ID_UNKNOWN && response->header_is_set[header_id]) {
        return TH_ERR_INVALID_ARG;
    }
    th_err err = TH_ERR_OK;
    size_t old_len = th_heap_string_len(&response->headers);
    if ((err = th_heap_string_append(&response->headers, key)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_heap_string_append(&response->headers, TH_STRING(": "))) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_heap_string_append(&response->headers, value)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_heap_string_append(&response->headers, TH_STRING("\r\n"))) != TH_ERR_OK)
        goto cleanup;
    if (header_id != TH_HEADER_ID_UNKNOWN) {
        response->header_is_set[header_id] = 1;
    }
    return TH_ERR_OK;
cleanup:
    th_heap_string_resize(&response->headers, old_len, '\0');
    return err;
}

TH_LOCAL(th_string)
th_response_get_mime_type(th_string filename)
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
        return mm ? mm->mime : TH_STRING("application/octet-stream");
    } else {
        return TH_STRING("application/octet-stream");
    }
}

TH_LOCAL(th_err)
th_response_set_body_from_file(th_response* response, th_string root, th_string path)
{
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_get(response->fcache, root, path, &response->fcache_entry)) != TH_ERR_OK) {
        return err;
    }
    // Set the content type, if not already set
    if (response->header_is_set[TH_HEADER_ID_CONTENT_TYPE] == 0) {
        th_string mime_type = th_response_get_mime_type(path);
        if ((err = th_response_add_header(response, TH_STRING("Content-Type"), mime_type)) != TH_ERR_OK)
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
th_response_set_body(th_response* response, th_string body)
{
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_set(&response->body, body)) != TH_ERR_OK)
        return err;
    response->is_file = 0;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
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
        if ((err = th_heap_string_set(&response->body, th_string_make(buffer, (size_t)len))) != TH_ERR_OK) {
            return err;
        }
    } else {
        th_heap_string_resize(&response->body, (size_t)len, ' ');
        vsnprintf(th_heap_string_at(&response->body, 0), (size_t)len, fmt, args);
    }
    response->is_file = 0;
    return TH_ERR_OK;
}

TH_LOCAL(th_err)
th_response_finalize_headers(th_response* response)
{
    th_err err = TH_ERR_OK;
    if ((err = th_heap_string_append(&response->headers, TH_STRING("\r\n"))) != TH_ERR_OK)
        return err;
    size_t headers_len = th_heap_string_len(&response->headers);

    // Set the start line
    char int_buffer[128]; // Buffer for the integer to string conversion
    if ((err = th_heap_string_append(&response->headers, TH_STRING("HTTP/1.1 "))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append_cstr(&response->headers, th_fmt_uint_to_str(int_buffer, sizeof(int_buffer), response->code))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append(&response->headers, TH_STRING(" "))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append_cstr(&response->headers, th_http_strerror((int)response->code))) != TH_ERR_OK)
        return err;
    if ((err = th_heap_string_append(&response->headers, TH_STRING("\r\n"))) != TH_ERR_OK)
        return err;
    response->iov[0].base = th_heap_string_at(&response->headers, headers_len);
    response->iov[0].len = th_heap_string_len(&response->headers) - headers_len;
    response->iov[1].base = th_heap_string_at(&response->headers, 0);
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
        if ((err = th_response_add_header(response, TH_STRING("Content-Length"), th_string_make(content_len, len))) != TH_ERR_OK)
            return err;
    } else {
        size_t len = 0;
        const char* body_len = th_fmt_uint_to_str_ex(buffer, sizeof(buffer), (unsigned int)th_heap_string_len(&response->body), &len);
        if ((err = th_response_add_header(response, TH_STRING("Content-Length"), th_string_make(body_len, len))) != TH_ERR_OK)
            return err;
    }
    if (!response->header_is_set[TH_HEADER_ID_SERVER]) {
        if ((err = th_response_add_header(response, TH_STRING("Server"), TH_STRING("TinyHTTP"))) != TH_ERR_OK)
            return err;
    }
    if (!response->header_is_set[TH_HEADER_ID_DATE]) {
        th_date now = th_date_now();
        char date[64];
        size_t len = th_fmt_strtime(date, sizeof(date), now);
        if ((err = th_response_add_header(response, TH_STRING("Date"), th_string_make(date, len))) != TH_ERR_OK)
            return err;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_response_async_write(th_response* response, th_socket* socket, th_io_handler* handler)
{
    th_err err = TH_ERR_OK;
    if (response->is_file) {
        response->file_len = response->fcache_entry->stream.size;
    }
    if ((err = th_response_set_default_headers(response)) != TH_ERR_OK)
        goto cleanup;
    if ((err = th_response_finalize_headers(response)) != TH_ERR_OK)
        goto cleanup;
    size_t iovcnt = 2; // start line + headers
    if (response->only_headers) {
        th_socket_async_writev_exact(socket, response->iov, iovcnt, handler);
        return;
    }
    if (response->is_file == 0) { // user provided body
        if (th_heap_string_len(&response->body) > 0) {
            response->iov[iovcnt].base = (void*)th_heap_string_data(&response->body);
            response->iov[iovcnt].len = th_heap_string_len(&response->body);
            iovcnt++;
        }
        th_socket_async_writev_exact(socket, response->iov, iovcnt, handler);
    } else {
        th_socket_async_sendfile_exact(socket, response->iov, iovcnt, &response->fcache_entry->stream, 0, (size_t)response->file_len, handler);
    }
    return;
cleanup:
    th_context_dispatch_handler(th_socket_get_context(socket), handler, 0, err);
}

/* Public response API begin */

TH_PUBLIC(th_err)
th_set_body(th_response* response, const char* body)
{
    return th_response_set_body(response, th_string_from_cstr(body));
}

TH_PUBLIC(th_err)
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
    return th_response_set_body_from_file(response, th_string_from_cstr(root), th_string_from_cstr(filepath));
}

TH_PUBLIC(th_err)
th_add_header(th_response* response, const char* key, const char* value)
{
    return th_response_add_header(response, th_string_from_cstr(key), th_string_from_cstr(value));
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
    return th_response_add_header(response, TH_STRING("Set-Cookie"), th_string_make(buffer, len));
}
/* End of src/th_response.c */
/* Start of src/th_context.c */

#undef TH_LOG_TAG
#define TH_LOG_TAG "context"

TH_LOCAL(th_err)
th_io_service_create(th_io_service** out, th_runner* runner, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    (void)out;
#if defined(TH_CONFIG_OS_MOCK)
    (void)allocator;
    TH_LOG_TRACE("Using mock");
    return th_mock_service_create(out, runner);
#endif
#if defined(TH_CONFIG_WITH_KQUEUE)
    TH_LOG_TRACE("Using kqueue");
    return th_kqueue_service_create(out, runner, allocator);
#endif
#if defined(TH_CONFIG_WITH_POLL)
    TH_LOG_TRACE("Using poll");
    return th_poll_service_create(out, runner, allocator);
#endif
    TH_LOG_ERROR("No IO service implementation available");
    return TH_ERR_NOSUPPORT;
}

TH_PRIVATE(th_err)
th_context_init(th_context* context, th_allocator* allocator)
{
    th_err err = TH_ERR_OK;
    context->allocator = allocator ? allocator : th_default_allocator_get();
    th_runner_init(&context->runner);
    if ((th_io_service_create(&context->io_service, &context->runner, context->allocator)) != TH_ERR_OK) {
        return err;
    }
    th_runner_set_io_service(&context->runner, context->io_service);
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_context_init_with_service(th_context* context, th_io_service* service)
{
    context->io_service = service;
    th_runner_init(&context->runner);
    th_runner_set_io_service(&context->runner, context->io_service);
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_context_push_task(th_context* context, th_task* task)
{
    th_runner_push_task(&context->runner, task);
}

TH_PRIVATE(th_err)
th_context_create_handle(th_context* context, th_io_handle** out, int fd)
{
    return th_io_service_create_handle(context->io_service, out, fd);
}

TH_PRIVATE(th_err)
th_context_poll(th_context* context, int timeout_ms)
{
    return th_runner_poll(&context->runner, timeout_ms);
}

TH_PRIVATE(void)
th_context_drain(th_context* context)
{
    th_runner_drain(&context->runner);
}

TH_PRIVATE(void)
th_context_deinit(th_context* context)
{
    th_runner_deinit(&context->runner);
    th_io_service_destroy(context->io_service);
}

TH_PRIVATE(void)
th_context_dispatch_handler(th_context* context, th_io_handler* handler, size_t result, th_err err)
{
    th_io_handler_set_result(handler, result, err);
    th_context_push_task(context, &handler->base);
}

TH_PRIVATE(void)
th_context_dispatch_composite_completion(th_context* context, th_io_composite* composite, size_t result, th_err err)
{
    th_context_dispatch_handler(context, TH_MOVE_PTR(composite->on_complete), result, err);
}
/* End of src/th_context.c */
/* Start of src/th_conn.c */


#include <assert.h>
#include <stdio.h>
#include <string.h>


/* th_conn_observable begin */

TH_LOCAL(void)
th_conn_observable_destroy(void* self)
{
    th_conn_observable* observable = self;
    th_conn_observer_on_deinit(observable->observer, observable);
    observable->destroy(observable);
}

TH_LOCAL(void)
th_conn_observable_init(th_conn_observable* observable,
                        th_socket* (*get_socket)(void* self),
                        th_address* (*get_address)(void* self),
                        void (*start)(void* self),
                        void (*destroy)(void* self),
                        th_conn_observer* observer)
{
    th_conn_init(&observable->base, get_socket, get_address, start, th_conn_observable_destroy);
    th_conn_observer_on_init(observer, observable);
    observable->destroy = destroy;
    observable->observer = observer;
}

/* th_conn_observable end */
/* th_tcp_conn begin */

#undef TH_LOG_TAG
#define TH_LOG_TAG "tcp_conn"

TH_LOCAL(th_socket*)
th_tcp_conn_get_socket(void* self);

TH_LOCAL(th_address*)
th_tcp_conn_get_address(void* self);

TH_LOCAL(void)
th_tcp_conn_start(void* self);

TH_LOCAL(void)
th_tcp_conn_destroy(void* conn);

TH_LOCAL(void)
th_tcp_conn_init(th_tcp_conn* conn, th_context* context,
                 th_conn_upgrader* upgrader,
                 th_conn_observer* observer,
                 th_allocator* allocator)
{
    th_conn_observable_init(&conn->base, th_tcp_conn_get_socket, th_tcp_conn_get_address,
                            th_tcp_conn_start, th_tcp_conn_destroy, observer);
    conn->context = context;
    conn->allocator = allocator ? allocator : th_default_allocator_get();
    conn->upgrader = upgrader;
    th_tcp_socket_init(&conn->socket, context, conn->allocator);
    th_address_init(&conn->addr);
}

TH_PRIVATE(th_err)
th_tcp_conn_create(th_conn** out, th_context* context,
                   th_conn_upgrader* upgrader,
                   th_conn_observer* observer,
                   th_allocator* allocator)
{
    th_tcp_conn* conn = th_allocator_alloc(allocator, sizeof(th_tcp_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_tcp_conn_init(conn, context, upgrader, observer, allocator);
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}

TH_LOCAL(th_socket*)
th_tcp_conn_get_socket(void* self)
{
    th_tcp_conn* conn = (th_tcp_conn*)self;
    return &conn->socket.base;
}

TH_LOCAL(th_address*)
th_tcp_conn_get_address(void* self)
{
    th_tcp_conn* conn = (th_tcp_conn*)self;
    return &conn->addr;
}

TH_LOCAL(void)
th_tcp_conn_start(void* self)
{
    th_tcp_conn* conn = (th_tcp_conn*)self;
    TH_LOG_TRACE("%p: Starting", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_tcp_conn_destroy(void* self)
{
    th_tcp_conn* conn = self;
    TH_LOG_TRACE("%p: Destroying connection", conn);
    th_tcp_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

/* th_tcp_conn end */
/* th_ssl_conn begin */

#if TH_WITH_SSL

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_conn"

TH_LOCAL(th_socket*)
th_ssl_conn_get_socket(void* self);

TH_LOCAL(th_address*)
th_ssl_conn_get_address(void* self);

TH_LOCAL(void)
th_ssl_conn_start(void* self);

TH_LOCAL(void)
th_ssl_conn_destroy(void* self);

TH_LOCAL(void)
th_ssl_conn_handshake_handler_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_ssl_conn_io_handler* handler = self;
    th_ssl_conn* conn = handler->conn;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p Handshake error: %s", conn, th_strerror(err));
        th_conn_destroy((th_conn*)conn);
        return;
    }
    TH_LOG_TRACE("%p Handshake complete", conn);
    th_conn_upgrader_upgrade(conn->upgrader, (th_conn*)conn);
}

TH_LOCAL(void)
th_ssl_conn_shutdown_handler_fn(void* self, size_t len, th_err err)
{
    (void)len;
    th_ssl_conn_io_handler* handler = self;
    th_ssl_conn* conn = handler->conn;
    // Whatever the result, we should finish the connection
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p Shutdown error: %s", conn, th_strerror(err));
    } else {
        TH_LOG_DEBUG("%p Shutdown complete", conn);
    }
    th_ssl_conn_destroy(conn);
}

TH_LOCAL(void)
th_ssl_conn_io_handler_init(th_ssl_conn_io_handler* handler, th_ssl_conn* conn,
                            void (*fn)(void* self, size_t len, th_err err), void (*destroy)(void* self))
{
    th_io_handler_init(&handler->base, fn, destroy);
    handler->conn = conn;
}

TH_LOCAL(th_err)
th_ssl_conn_init(th_ssl_conn* conn, th_context* context, th_ssl_context* ssl_context,
                 th_conn_upgrader* upgrader, th_conn_observer* observer,
                 th_allocator* allocator)
{
    th_conn_observable_init(&conn->base, th_ssl_conn_get_socket, th_ssl_conn_get_address,
                            th_ssl_conn_start, th_ssl_conn_destroy, observer);
    th_ssl_conn_io_handler_init(&conn->handshake_handler, conn,
                                th_ssl_conn_handshake_handler_fn, NULL);
    th_ssl_conn_io_handler_init(&conn->shutdown_handler, conn,
                                th_ssl_conn_shutdown_handler_fn, NULL);
    conn->context = context;
    conn->allocator = allocator;
    conn->upgrader = upgrader;
    th_address_init(&conn->addr);
    return th_ssl_socket_init(&conn->socket, context, ssl_context, conn->allocator);
}

TH_PRIVATE(th_err)
th_ssl_conn_create(th_conn** out, th_context* context, th_ssl_context* ssl_context,
                   th_conn_upgrader* upgrader,
                   th_conn_observer* observer,
                   th_allocator* allocator)
{
    th_ssl_conn* conn = th_allocator_alloc(allocator, sizeof(th_ssl_conn));
    if (!conn)
        return TH_ERR_BAD_ALLOC;
    th_err err = TH_ERR_OK;
    if ((err = th_ssl_conn_init(conn, context, ssl_context, upgrader, observer, allocator)) != TH_ERR_OK) {
        th_allocator_free(allocator, conn);
        return err;
    }
    *out = (th_conn*)conn;
    return TH_ERR_OK;
}

TH_LOCAL(th_socket*)
th_ssl_conn_get_socket(void* self)
{
    th_ssl_conn* conn = (th_ssl_conn*)self;
    return (th_socket*)&conn->socket;
}

TH_LOCAL(th_address*)
th_ssl_conn_get_address(void* self)
{
    th_ssl_conn* conn = (th_ssl_conn*)self;
    return &conn->addr;
}

TH_LOCAL(void)
th_ssl_conn_start(void* self)
{
    th_ssl_conn* conn = (th_ssl_conn*)self;
    TH_LOG_TRACE("%p: Starting", conn);
    th_ssl_socket_set_mode(&conn->socket, TH_SSL_SOCKET_MODE_SERVER);
    th_ssl_socket_async_handshake(&conn->socket, &conn->handshake_handler.base);
}

TH_LOCAL(void)
th_ssl_conn_destroy(void* self)
{
    th_ssl_conn* conn = self;
    TH_LOG_TRACE("%p Destroying connection", conn);
    th_ssl_socket_deinit(&conn->socket);
    th_allocator_free(conn->allocator, conn);
}

#endif /* TH_WITH_SSL */

/* th_ssl_conn end */
/* End of src/th_conn.c */
/* Start of src/th_header_id.c */
/* ANSI-C code produced by gperf version 3.1 */
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
  return len;
}

struct th_header_id_mapping *
th_header_id_mapping_find (register const char *str, register size_t len)
{
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
  return 0;
}

#pragma GCC diagnostic pop
/* End of src/th_header_id.c */
/* Start of src/th_file.c */

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_MOCK)
#endif

#undef TH_LOG_TAG
#define TH_LOG_TAG "file"

/* th_file_view implmentation begin */

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(th_err)
th_file_mmap_mmap_posix(th_file_mmap* view, th_file* file, size_t offset, size_t len)
{
    size_t page_size = (size_t)sysconf(_SC_PAGESIZE);
    size_t moffset = TH_ALIGNDOWN(offset, page_size);
    void* addr = mmap(NULL, len, PROT_READ, MAP_PRIVATE, file->fd, (off_t)moffset);
    if (addr == MAP_FAILED) {
        return TH_ERR_SYSTEM(errno);
    }
    view->addr = addr;
    view->offset = moffset;
    view->len = len;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_file_mmap_munmap_posix(th_file_mmap* view)
{
    munmap(view->addr, view->len);
    view->addr = 0;
    view->len = 0;
    view->offset = 0;
}
#endif

TH_LOCAL(th_err)
th_file_mmap_mmap(th_file_mmap* view, th_file* file, size_t offset, size_t len)
{
#if defined(TH_CONFIG_OS_POSIX)
    return th_file_mmap_mmap_posix(view, file, offset, len);
#else
    (void)view;
    (void)file;
    (void)offset;
    (void)len;
    return TH_ERR_NOSUPPORT;
#endif
}

TH_LOCAL(void)
th_file_mmap_munmap(th_file_mmap* view)
{
#if defined(TH_CONFIG_OS_POSIX)
    th_file_mmap_munmap_posix(view);
#else
    (void)view;
#endif
}

TH_LOCAL(void)
th_file_mmap_init(th_file_mmap* view)
{
    view->addr = 0;
    view->offset = 0;
    view->len = 0;
}

TH_LOCAL(th_err)
th_file_mmap_map(th_file_mmap* view, th_file* file, size_t offset, size_t len)
{
    if (view->addr)
        th_file_mmap_munmap(view);
    len = TH_MIN(len, file->size - offset);
    return th_file_mmap_mmap(view, file, offset, len);
}

TH_LOCAL(void)
th_file_mmap_deinit(th_file_mmap* view)
{
    if (view->addr)
        th_file_mmap_munmap(view);
}

/* th_file_mmap_map implementation end */
/* th_file implementation begin */

TH_LOCAL(th_err)
th_file_validate_path(th_dir* dir, th_string path, th_allocator* allocator)
{
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    th_heap_string realpath = {0};
    th_heap_string_init(&realpath, allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_path_resolve_against(path, dir, &realpath)) != TH_ERR_OK)
        goto cleanup;
    if (!th_path_is_within(th_heap_string_view(&realpath), dir)) {
        err = TH_ERR_HTTP(TH_CODE_FORBIDDEN);
        goto cleanup;
    }
    if (th_path_is_hidden(th_heap_string_view(&realpath))) {
        err = TH_ERR_HTTP(TH_CODE_FORBIDDEN);
        goto cleanup;
    }
cleanup:
    th_heap_string_deinit(&realpath);
    return err;
}

TH_PRIVATE(void)
th_file_init(th_file* stream)
{
    stream->fd = -1;
    th_file_mmap_init(&stream->view);
}

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, th_string path, th_open_opt opt)
{
    th_err err = TH_ERR_OK;
    if ((err = th_file_validate_path(dir, path, dir->allocator)) != TH_ERR_OK) {
        if (err == TH_ERR_SYSTEM(TH_ENOENT) && opt.create) {
            // resolve only the directory part
            size_t last_slash = th_string_find_last(path, 0, '/');
            if (last_slash == th_string_npos)
                last_slash = 0;
            th_string dirpath = th_string_substr(path, 0, last_slash);
            if ((err = th_file_validate_path(dir, dirpath, dir->allocator)) != TH_ERR_OK)
                return err;
        } else {
            return err;
        }
    }
#if defined(TH_CONFIG_OS_POSIX)
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int flags = O_NOFOLLOW;
    if (opt.read && opt.write)
        flags = O_RDWR;
    else if (opt.read)
        flags = O_RDONLY;
    else if (opt.write)
        flags = O_WRONLY;
    if (opt.create)
        flags |= O_CREAT;
    if (opt.truncate)
        flags |= O_TRUNC;
    int fd = openat(dir->fd, path_buf, flags, 0644);
    if (fd == -1)
        return TH_ERR_SYSTEM(errno);
    off_t pos = lseek(fd, 0, SEEK_END);
    if (pos == -1)
        goto cleanup_socket;
    if (lseek(fd, 0, SEEK_SET) == -1)
        goto cleanup_socket;
    stream->fd = fd;
    stream->size = (size_t)pos;
    return TH_ERR_OK;
cleanup_socket:
    close(fd);
    return TH_ERR_SYSTEM(errno);
#elif defined(TH_CONFIG_OS_MOCK)
    (void)dir;
    (void)opt;
    (void)path;
    int fd = th_mock_open();
    if (fd < 0)
        return TH_ERR_SYSTEM(-fd);
    stream->fd = fd;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read)
{
#if defined(TH_CONFIG_OS_POSIX)
    off_t ret = pread(stream->fd, addr, len, (off_t)offset);
    if (ret == -1) {
        *read = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *read = (size_t)ret;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)stream;
    (void)offset;
    int ret = th_mock_read(addr, len);
    if (ret < 0)
        return TH_ERR_SYSTEM(-ret);
    *read = (size_t)ret;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_err)
th_file_write(th_file* stream, const void* addr, size_t len, size_t offset, size_t* written)
{
#if defined(TH_CONFIG_OS_POSIX)
    off_t ret = pwrite(stream->fd, addr, len, (off_t)offset);
    if (ret == -1) {
        *written = 0;
        return TH_ERR_SYSTEM(errno);
    }
    *written = (size_t)ret;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)stream;
    (void)addr;
    (void)offset;
    int ret = th_mock_write(len);
    if (ret < 0)
        return TH_ERR_SYSTEM(-ret);
    *written = (size_t)ret;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_err)
th_file_get_view(th_file* stream, th_fileview* view, size_t offset, size_t len)
{
    th_err err = TH_ERR_OK;
    if (stream->view.addr == NULL
        || stream->view.offset > offset
        || stream->view.offset + stream->view.len < offset + 8 * 1024) {
        if ((err = th_file_mmap_map(&stream->view, stream, offset, len)) != TH_ERR_OK)
            return err;
    }
    view->ptr = (uint8_t*)stream->view.addr + (offset - stream->view.offset);
    view->len = stream->view.len - (offset - stream->view.offset);
    return TH_ERR_OK;
}

/**
 * We use DJB2 hash function, without multiplication,
 * as it's faster and good enough for our purposes.
 */
#define FSTAT_HASH_INIT 5381
#define FSTAT_HASH_NEXT(hash, val) ((hash << 5) + hash + val)

#if defined(TH_CONFIG_OS_POSIX)
TH_LOCAL(uint32_t)
th_file_stat_hash_posix(th_file* stream)
{
    struct stat st = {0};
    if (fstat(stream->fd, &st) == -1) {
        TH_LOG_ERROR("fstat failed: %s, can't calculate hash", strerror(errno));
        TH_ASSERT(0 && "fstat failed");
        return 0;
    }

    uint32_t hash = FSTAT_HASH_INIT;
#if defined(TH_CONFIG_OS_OSX)
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mtimespec.tv_sec);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mtimespec.tv_nsec);
#else
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mtime);
#endif
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_size);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_mode);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_ino);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_uid);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)st.st_gid);
    hash = FSTAT_HASH_NEXT(hash, (uint32_t)(st.st_nlink != 0));
    return hash;
}
#elif defined(TH_CONFIG_OS_WIN)
#error "Not implemented"
TH_LOCAL(uint32_t)
th_file_stat_hash_win(th_file* stream)
{
    (void)stream;
    return 0;
}
#elif defined(TH_CONFIG_OS_MOCK)
TH_LOCAL(uint32_t)
th_file_stat_hash_mock(th_file* stream)
{
    (void)stream;
    return 0;
}
#endif
#undef FSTAT_HASH_INIT
#undef FSTAT_HASH_NEXT

TH_PRIVATE(uint32_t)
th_file_stat_hash(th_file* stream)
{
#if defined(TH_CONFIG_OS_POSIX)
    return th_file_stat_hash_posix(stream);
#elif defined(TH_CONFIG_OS_WIN)
    return th_file_stat_hash_win(stream);
#elif defined(TH_CONFIG_OS_MOCK)
    return th_file_stat_hash_mock(stream);
#else
    return 0;
#endif
}

TH_PRIVATE(void)
th_file_close(th_file* stream)
{
    th_file_mmap_deinit(&stream->view);
#if defined(TH_CONFIG_OS_POSIX)
    if (stream->fd != -1)
        close(stream->fd);
#elif defined(TH_CONFIG_OS_MOCK)
    if (stream->fd != -1)
        th_mock_close();
#endif
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
    return (th_fcache_id){th_heap_string_view(&entry->path), entry->dir};
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
    th_heap_string_deinit(&entry->path);
    th_allocator_free(entry->allocator, entry);
}

TH_LOCAL(void)
th_fcache_entry_init(th_fcache_entry* entry, th_fcache* cache, th_allocator* allocator)
{
    entry->allocator = allocator ? allocator : th_default_allocator_get();
    th_refcounted_init(&entry->base, th_fcache_entry_actual_destroy);
    th_file_init(&entry->stream);
    th_heap_string_init(&entry->path, entry->allocator);
    entry->cache = cache;
    entry->next = NULL;
    entry->prev = NULL;
}

TH_LOCAL(th_err)
th_fcache_entry_open(th_fcache_entry* entry, th_string root, th_string path)
{
    th_err err = TH_ERR_OK;
    th_dir* dir = th_dir_mgr_get(&entry->cache->dir_mgr, root);
    if (!dir)
        return TH_ERR_INVALID_ARG;
    th_open_opt opt = {.read = true};
    if ((err = th_file_openat(&entry->stream, dir, path, opt)) != TH_ERR_OK) {
        TH_LOG_INFO("Failed to open file at %.*s: %s", (int)path.len, path.ptr, th_strerror(err));
        goto cleanup;
    }
    if ((err = th_heap_string_set(&entry->path, path)) != TH_ERR_OK) {
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
th_fcache_init(th_fcache* cache, th_allocator* allocator)
{
    cache->allocator = allocator ? allocator : th_default_allocator_get();
    th_dir_mgr_init(&cache->dir_mgr, cache->allocator);
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
th_fcache_try_get(th_fcache* cache, th_string root, th_string path)
{
    th_dir* dir = th_dir_mgr_get(&cache->dir_mgr, root);
    if (!dir)
        return NULL;
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

TH_PRIVATE(th_err)
th_fcache_add_dir(th_fcache* cache, th_string label, th_string path)
{
    return th_dir_mgr_add(&cache->dir_mgr, label, path);
}

TH_PRIVATE(th_dir*)
th_fcache_find_dir(th_fcache* cache, th_string label)
{
    return th_dir_mgr_get(&cache->dir_mgr, label);
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
th_fcache_get(th_fcache* cache, th_string root, th_string path, th_fcache_entry** out)
{
    th_fcache_entry* entry = th_fcache_try_get(cache, root, path);
    if (entry) {
        *out = entry;
        return TH_ERR_OK;
    }
    entry = th_allocator_alloc(cache->allocator, sizeof(th_fcache_entry));
    if (!entry)
        return TH_ERR_BAD_ALLOC;
    th_fcache_entry_init(entry, cache, cache->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_entry_open(entry, root, path)) != TH_ERR_OK) {
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
    th_dir_mgr_deinit(&cache->dir_mgr);
}
/* End of src/th_fcache.c */
/* Start of src/th_dir.c */

#if defined(TH_CONFIG_OS_POSIX)
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined(TH_CONFIG_OS_MOCK)
#endif

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_allocator* allocator)
{
    dir->allocator = allocator ? allocator : th_default_allocator_get();
    dir->fd = -1;
    th_heap_string_init(&dir->path, dir->allocator);
}

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_string path)
{
    th_err err = TH_ERR_OK;
    if ((err = th_path_resolve(path, &dir->path)) != TH_ERR_OK)
        return err;
#if defined(TH_CONFIG_OS_POSIX)
    if (path.len > TH_CONFIG_MAX_PATH_LEN)
        return TH_ERR_INVALID_ARG;
    char path_buf[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    memcpy(path_buf, path.ptr, path.len);
    path_buf[path.len] = '\0';
    int fd = open(path_buf, O_RDONLY | O_DIRECTORY);
    if (fd < 0)
        return TH_ERR_SYSTEM(errno);
    dir->fd = fd;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)path;
    int fd = th_mock_open();
    if (fd < 0)
        return TH_ERR_SYSTEM(-fd);
    dir->fd = fd;
    return TH_ERR_OK;
#endif
}

TH_PRIVATE(th_string)
th_dir_get_path(th_dir* dir)
{
    return th_heap_string_view(&dir->path);
}

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir)
{
    th_heap_string_deinit(&dir->path);
#if defined(TH_CONFIG_OS_POSIX)
    if (dir->fd >= 0) {
        int ret = close(dir->fd);
        (void)ret;
        TH_ASSERT(ret == 0 && "This should not happen");
    }
#elif defined(TH_CONFIG_OS_MOCK)
    if (dir->fd >= 0) {
        int ret = th_mock_close();
        (void)ret;
        TH_ASSERT(ret == 0 && "This should not happen");
    }
#endif
}
/* End of src/th_dir.c */
/* Start of src/th_dir_mgr.c */

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator)
{
    mgr->allocator = allocator ? allocator : th_default_allocator_get();
    th_dir_map_init(&mgr->map, allocator);
    th_heap_string_vec_init(&mgr->heap_strings, allocator);
}

TH_LOCAL(bool)
th_dir_mgr_label_exists(th_dir_mgr* mgr, th_string label)
{
    return th_dir_map_find(&mgr->map, label) != NULL;
}

TH_LOCAL(th_err)
th_dir_mgr_store_string(th_dir_mgr* mgr, th_string str)
{
    th_heap_string heap_str = {0};
    th_heap_string_init(&heap_str, mgr->allocator);
    if (th_heap_string_set(&heap_str, str) != TH_ERR_OK) {
        return TH_ERR_BAD_ALLOC;
    }
    if (th_heap_string_vec_push_back(&mgr->heap_strings, heap_str) != TH_ERR_OK) {
        th_heap_string_deinit(&heap_str);
        return TH_ERR_BAD_ALLOC;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_string)
th_dir_mgr_get_last_string(th_dir_mgr* mgr)
{
    return th_heap_string_view(th_heap_string_vec_end(&mgr->heap_strings) - 1);
}

TH_LOCAL(void)
th_dir_mgr_remove_last_string(th_dir_mgr* mgr)
{
    th_heap_string_deinit(th_heap_string_vec_end(&mgr->heap_strings) - 1);
    th_heap_string_vec_resize(&mgr->heap_strings, th_heap_string_vec_size(&mgr->heap_strings) - 1);
}

TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_string label, th_string path)
{
    th_err err = TH_ERR_OK;
    if (th_dir_mgr_label_exists(mgr, label))
        return TH_ERR_INVALID_ARG;
    th_dir dir = {0};
    th_dir_init(&dir, mgr->allocator);
    if ((err = th_dir_open(&dir, path)) != TH_ERR_OK)
        goto cleanup_dir;
    if ((err = th_dir_mgr_store_string(mgr, label)) != TH_ERR_OK)
        goto cleanup_dir;
    if ((err = th_dir_map_set(&mgr->map, th_dir_mgr_get_last_string(mgr), dir)) != TH_ERR_OK)
        goto cleanup_string;
    return TH_ERR_OK;
cleanup_string:
    th_dir_mgr_remove_last_string(mgr);
cleanup_dir:
    th_dir_deinit(&dir);
    return err;
}

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_string label)
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
    th_heap_string_vec_deinit(&mgr->heap_strings);
}
/* End of src/th_dir_mgr.c */
/* Start of src/th_string.c */

#include <stdbool.h>


size_t th_string_npos = (size_t)-1;

TH_PRIVATE(bool)
th_string_is_uint(th_string str)
{
    for (size_t i = 0; i < str.len; i++) {
        if (str.ptr[i] < '0' || str.ptr[i] > '9') {
            return false;
        }
    }
    return true;
}

TH_PRIVATE(th_err)
th_string_to_uint(th_string str, unsigned int* out)
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
th_string_eq(th_string a, th_string b)
{
    if (a.len != b.len) {
        return 0;
    }
    for (size_t i = 0; i < a.len; i++) {
        if (a.ptr[i] != b.ptr[i]) {
            return 0;
        }
    }
    return 1;
}

TH_PRIVATE(size_t)
th_string_find_first(th_string str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[i] == c) {
            return i;
        }
    }
    return th_string_npos;
}

TH_PRIVATE(size_t)
th_string_find_first_not(th_string str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[i] != c) {
            return i;
        }
    }
    return th_string_npos;
}

TH_PRIVATE(size_t)
th_string_find_first_of(th_string str, size_t start, const char* chars)
{
    for (size_t i = start; i < str.len; i++) {
        for (size_t j = 0; chars[j] != '\0'; j++) {
            if (str.ptr[i] == chars[j]) {
                return i;
            }
        }
    }
    return th_string_npos;
}

TH_PRIVATE(size_t)
th_string_find_last(th_string str, size_t start, char c)
{
    for (size_t i = start; i < str.len; i++) {
        if (str.ptr[str.len - i - 1] == c) {
            return i;
        }
    }
    return th_string_npos;
}

TH_PRIVATE(th_string)
th_string_substr(th_string str, size_t start, size_t len)
{
    if (start >= str.len) {
        return th_string_make(str.ptr + len, 0);
    }
    if (len == th_string_npos || start + len > str.len) {
        len = str.len - start;
    }
    return th_string_make(str.ptr + start, len);
}

TH_PRIVATE(th_string)
th_string_trim(th_string str)
{
    size_t start = 0;
    while (start < str.len && (str.ptr[start] == ' ' || str.ptr[start] == '\t')) {
        start++;
    }
    size_t end = str.len;
    while (end > start && (str.ptr[end - 1] == ' ' || str.ptr[end - 1] == '\t')) {
        end--;
    }
    return th_string_substr(str, start, end - start);
}

TH_PRIVATE(size_t)
th_string_hash(th_string str)
{
    return th_hash_bytes(str.ptr, str.len);
}
/* End of src/th_string.c */
/* Start of src/th_heap_string.c */

#include <ctype.h>

#define TH_HEAP_STRING_SMALL (sizeof(char*) + sizeof(size_t) + sizeof(size_t) - 2)
#define TH_HEAP_STRING_ALIGNUP(size) TH_ALIGNUP(size, 16)
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
th_heap_string_init(th_heap_string* self, th_allocator* allocator)
{
    th_detail_small_string_init(&self->impl.small, allocator);
}

TH_PRIVATE(th_err)
th_heap_string_init_with(th_heap_string* self, th_string str, th_allocator* allocator)
{
    th_heap_string_init(self, allocator);
    return th_heap_string_set(self, str);
}

TH_LOCAL(void)
th_detail_small_string_set(th_detail_small_string* self, th_string str)
{
    TH_ASSERT(str.len <= TH_HEAP_STRING_SMALL_MAX_LEN);
    if (str.len > 0)
        memcpy(self->buf, str.ptr, str.len);
    self->buf[str.len] = '\0';
    self->len = str.len & 0x7F;
}

TH_LOCAL(th_err)
th_detail_large_string_set(th_detail_large_string* self, th_string str)
{
    size_t required_capacity = str.len + 1;
    if (self->capacity < required_capacity) {
        size_t new_capacity = TH_HEAP_STRING_ALIGNUP(required_capacity);
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
th_heap_string_small_to_large(th_heap_string* self, size_t capacity)
{
    TH_ASSERT(self->impl.small.small);
    th_detail_large_string large = {0};
    capacity = TH_HEAP_STRING_ALIGNUP(capacity);
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
th_heap_string_set(th_heap_string* self, th_string str)
{
    TH_ASSERT(str.ptr != NULL && "Invalid string");
    if (self->impl.small.small) {
        if (str.len <= TH_HEAP_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_set(&self->impl.small, str);
            return TH_ERR_OK;
        } else {
            th_err err = th_heap_string_small_to_large(self, str.len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_set(&self->impl.large, str);
}

TH_LOCAL(void)
th_detail_small_string_append(th_detail_small_string* self, th_string str)
{
    TH_ASSERT(self->len + str.len <= TH_HEAP_STRING_SMALL_MAX_LEN);
    memcpy(self->buf + self->len, str.ptr, str.len);
    self->len += str.len & 0x7F;
    self->buf[self->len] = '\0';
}

TH_LOCAL(th_err)
th_detail_large_string_append(th_detail_large_string* self, th_string str)
{
    size_t required_capacity = self->len + str.len + 1;
    if (required_capacity > self->capacity) {
        size_t new_capacity = TH_HEAP_STRING_ALIGNUP(required_capacity);
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
th_heap_string_append(th_heap_string* self, th_string str)
{
    if (self->impl.small.small) {
        if (self->impl.small.len + str.len <= TH_HEAP_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_append(&self->impl.small, str);
            return TH_ERR_OK;
        } else {
            th_err err = th_heap_string_small_to_large(self, self->impl.small.len + str.len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_append(&self->impl.large, str);
}

TH_PRIVATE(th_err)
th_heap_string_append_cstr(th_heap_string* self, const char* str)
{
    return th_heap_string_append(self, th_string_make(str, strlen(str)));
}

TH_PRIVATE(th_err)
th_heap_string_push_back(th_heap_string* self, char c)
{
    return th_heap_string_append(self, (th_string){&c, 1});
}

TH_LOCAL(void)
th_detail_small_string_resize(th_detail_small_string* self, size_t new_len, char fill)
{
    TH_ASSERT(new_len <= TH_HEAP_STRING_SMALL_MAX_LEN && "Invalid length");
    memset(self->buf + self->len, fill, new_len - self->len);
    self->len = new_len & 0x7F;
    self->buf[new_len] = '\0';
}

TH_LOCAL(th_err)
th_detail_large_string_resize(th_detail_large_string* self, size_t new_len, char fill)
{
    size_t required_capacity = new_len + 1;
    if (required_capacity > self->capacity) {
        size_t new_capacity = TH_HEAP_STRING_ALIGNUP(required_capacity);
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
th_heap_string_resize(th_heap_string* self, size_t new_len, char fill)
{
    if (self->impl.small.small) {
        if (new_len <= TH_HEAP_STRING_SMALL_MAX_LEN) {
            th_detail_small_string_resize(&self->impl.small, new_len, fill);
            return TH_ERR_OK;
        } else {
            th_err err = th_heap_string_small_to_large(self, new_len + 1);
            if (err != TH_ERR_OK)
                return err;
        }
    }
    return th_detail_large_string_resize(&self->impl.large, new_len, fill);
}

TH_PRIVATE(th_string)
th_heap_string_view(const th_heap_string* self)
{
    if (self->impl.small.small) {
        return (th_string){self->impl.small.buf, self->impl.small.len};
    } else {
        return (th_string){self->impl.large.ptr, self->impl.large.len};
    }
}

TH_PRIVATE(const char*)
th_heap_string_data(const th_heap_string* self)
{
    if (self->impl.small.small) {
        return self->impl.small.buf;
    } else {
        return self->impl.large.ptr;
    }
}

TH_PRIVATE(char*)
th_heap_string_at(th_heap_string* self, size_t index)
{
    TH_ASSERT(index < th_heap_string_len(self) && "Index out of bounds");
    if (self->impl.small.small) {
        return &self->impl.small.buf[index];
    } else {
        return &self->impl.large.ptr[index];
    }
}

TH_PRIVATE(size_t)
th_heap_string_len(const th_heap_string* self)
{
    if (self->impl.small.small) {
        return self->impl.small.len;
    } else {
        return self->impl.large.len;
    }
}

TH_PRIVATE(void)
th_heap_string_clear(th_heap_string* self)
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
th_heap_string_to_lower(th_heap_string* self)
{
    char* ptr = th_heap_string_at(self, 0);
    size_t n = th_heap_string_len(self);
    for (size_t i = 0; i < n; i++) {
        ptr[i] = (char)tolower((int)ptr[i]);
    }
}

TH_PRIVATE(bool)
th_heap_string_eq(const th_heap_string* self, th_string other)
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

//TH_PRIVATE(uint32_t)
//th_heap_string_hash(const th_heap_string* self)
//{
//    const char* ptr = NULL;
//    size_t n = 0;
//    if (self->impl.small.small) {
//        ptr = self->impl.small.buf;
//        n = self->impl.small.len;
//    } else {
//        ptr = self->impl.large.ptr;
//        n = self->impl.large.len;
//    }
//    return th_hash_bytes(ptr, n);
//}

TH_PRIVATE(void)
th_heap_string_deinit(th_heap_string* self)
{
    if (!self->impl.small.small) {
        th_allocator_free(self->impl.large.allocator, self->impl.large.ptr);
    }
}
/* End of src/th_heap_string.c */
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
th_http_restart(th_http* http)
{
    http->read_bytes = 0;
    http->parsed_bytes = 0;
    th_request_parser_reset(&http->parser);
    th_request_reset(&http->request);
    th_response_reset(&http->response);
    http->state = TH_HTTP_STATE_READ_REQUEST;
    th_socket_async_read(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, 0), th_buf_vec_size(&http->buf), &http->io_handler.base);
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
    http->state = TH_HTTP_STATE_WRITE_RESPONSE;
    th_response_async_write(&http->response, th_conn_get_socket(http->conn), &http->io_handler.base);
}

TH_LOCAL(void)
th_http_write_error_response(th_http* http, th_err err)
{
    th_response_set_code(&http->response, TH_ERR_CODE(err));
    if (th_heap_string_len(&http->request.uri_path) == 0) {
        // Set default error message
        th_printf_body(&http->response, "%d %s", TH_ERR_CODE(err), th_http_strerror((int)err));
    }
    if (http->close) {
        th_response_add_header(&http->response, TH_STRING("Connection"), TH_STRING("close"));
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
    TH_LOG_ERROR("%p: Trying send a HTTP/1.1 response to a HTTP/1.0 client, sending 400 Bad Request instead", http);
    th_response_set_body(&http->response, TH_STRING("HTTP/1.1 required for this request"));
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
    if (strcmp(th_heap_string_data(&request->uri_path), "*") != 0) {
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
    if ((err = th_response_add_header(response, TH_STRING("Allow"), th_string_make(allow, pos))) != TH_ERR_OK)
        return err;
    if ((err = th_response_add_header(response, TH_STRING("Content-Type"), TH_STRING("text/plain"))) != TH_ERR_OK)
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
        th_response_add_header(response, TH_STRING("Connection"), TH_STRING("close"));
        http->close = true;
    } else {
        th_response_add_header(response, TH_STRING("Connection"), TH_STRING("keep-alive"));
    }
    TH_LOG_TRACE("%p: Write response %p", http, response);
    th_http_write_response(http);
}

TH_LOCAL(void)
th_http_handle_read_request(th_http* http, size_t len, th_err err)
{
    if (err != TH_ERR_OK) {
        TH_LOG_DEBUG("%p: Read error: %s", http, th_strerror(err));
        http->close = TH_HTTP_CLOSE; // No other choice if we can't even read the request
        th_http_complete(http);
        return;
    }
    http->read_bytes += len;
    size_t parsed = 0;
    th_string parser_input = (th_string){.ptr = th_buf_vec_at(&http->buf, http->parsed_bytes),
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
        th_socket_async_read(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, http->read_bytes),
                             th_buf_vec_size(&http->buf) - http->read_bytes, &http->io_handler.base);
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
        th_socket_async_read_exact(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, http->read_bytes),
                                   remaining, &http->io_handler.base);
    }
}

TH_LOCAL(void)
th_http_handle_write_response(th_http* http, size_t len, th_err err)
{
    (void)len;
    if (err != TH_ERR_OK) {
        TH_LOG_ERROR("%p: Write error: %s", http, th_strerror(err));
        http->close = TH_HTTP_CLOSE; // Connection is broken, close it
    } else {
        TH_LOG_TRACE("%p: Write response of %d bytes", http, (int)len);
    }
    th_http_complete(http);
}

TH_LOCAL(void)
th_http_io_handler_fn(void* self, size_t len, th_err err)
{
    th_http_io_handler* handler = self;
    th_http* http = handler->http;
    switch (http->state) {
        {
        case TH_HTTP_STATE_READ_REQUEST:
            th_http_handle_read_request(http, len, err);
            break;
        case TH_HTTP_STATE_WRITE_RESPONSE:
            th_http_handle_write_response(http, len, err);
            break;
        default:
            TH_ASSERT(0 && "Invalid state");
            break;
        }
    }
}

TH_LOCAL(void)
th_http_io_handler_init(th_http_io_handler* handler, th_http* http)
{
    th_io_handler_init(&handler->base, th_http_io_handler_fn, NULL);
    handler->http = http;
}

TH_LOCAL(void)
th_http_start(void* self)
{
    th_http* http = self;
    TH_LOG_TRACE("%p: Starting", http);
    th_buf_vec_resize(&http->buf, TH_CONFIG_SMALL_HEADER_LEN);
    th_socket_async_read(th_conn_get_socket(http->conn), th_buf_vec_at(&http->buf, 0), th_buf_vec_size(&http->buf), &http->io_handler.base);
}

TH_LOCAL(void)
th_http_init(th_http* http, const th_conn_tracker* tracker, th_conn* conn,
             th_router* router, th_fcache* fcache, th_allocator* allocator)
{
    allocator = allocator ? allocator : th_default_allocator_get();
    th_http_io_handler_init(&http->io_handler, http);
    th_request_parser_init(&http->parser);
    th_request_init(&http->request, fcache, allocator);
    th_response_init(&http->response, fcache, allocator);
    th_buf_vec_init(&http->buf, allocator);
    http->tracker = tracker;
    http->conn = conn;
    http->router = router;
    http->fcache = fcache;
    http->allocator = allocator;
    http->read_bytes = 0;
    http->parsed_bytes = 0;
    http->state = TH_HTTP_STATE_READ_REQUEST;
    http->close = TH_HTTP_KEEP_ALIVE;
}

TH_LOCAL(th_err)
th_http_create(th_http** out, const th_conn_tracker* tracker, th_conn* conn,
               th_router* router, th_fcache* fcache, th_allocator* allocator)
{
    th_http* http = th_allocator_alloc(allocator, sizeof(th_http));
    if (!http)
        return TH_ERR_BAD_ALLOC;
    th_http_init(http, tracker, conn, router, fcache, allocator);
    *out = http;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_http_upgrader_upgrade(void* self, th_conn* conn)
{
    th_http_upgrader* upgrader = self;
    th_http* http = NULL;
    th_err err = TH_ERR_OK;
    if ((err = th_http_create(&http, upgrader->tracker, conn, upgrader->router, upgrader->fcache, upgrader->allocator)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to create http instance: %s", th_strerror(err));
        th_conn_destroy(conn);
        return;
    }
    th_http_start(http);
}

TH_PRIVATE(void)
th_http_upgrader_init(th_http_upgrader* upgrader, const th_conn_tracker* tracker, th_router* router,
                      th_fcache* fcache, th_allocator* allocator)
{
    th_conn_upgrader_init(&upgrader->base, th_http_upgrader_upgrade);
    upgrader->tracker = tracker;
    upgrader->router = router;
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
/* Start of src/th_io_op.c */


TH_PRIVATE(th_err)
th_io_op_read(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_read(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_read(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_readv(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_readv(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_readv(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_write(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_write(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_write(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_writev(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_writev(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_writev(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_send(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_send(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_send(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_sendv(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_sendv(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_sendv(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_accept(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_accept(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    return th_io_op_posix_accept(self, result);
#endif
}

TH_PRIVATE(th_err)
th_io_op_sendfile(void* self, size_t* result)
{
#if defined(TH_CONFIG_OS_MOCK)
    return th_io_op_mock_sendfile(self, result);
#elif defined(TH_CONFIG_OS_POSIX)
    th_io_task* iot = self;
    if (iot->len2 < (8 * 1024)) {
        return th_io_op_posix_sendfile_buffered(self, result);
    }
#if defined(TH_CONFIG_WITH_BSD_SENDFILE)
    return th_io_op_bsd_sendfile(self, result);
#endif
#if defined(TH_CONFIG_WITH_LINUX_SENDFILE)
    return th_io_op_linux_sendfile(self, result);
#endif
    return th_io_op_posix_sendfile_mmap(self, result);
#endif // TH_CONFIG_HAVE_SENDFILE
}
/* End of src/th_io_op.c */
/* Start of src/th_timer.c */

#ifdef TH_CONFIG_OS_POSIX
#include <errno.h>
#include <time.h>
#elif defined(TH_CONFIG_OS_WIN)
#include <windows.h>
#endif

TH_PRIVATE(void)
th_timer_init(th_timer* timer)
{
    timer->expire = 0;
}

TH_LOCAL(th_err)
th_timer_monotonic_now(time_t* out)
{
#if defined(TH_CONFIG_OS_POSIX)
    struct timespec ts = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret != 0) {
        return TH_ERR_SYSTEM(errno);
    }
    *out = ts.tv_sec;
    return TH_ERR_OK;
#elif defined(TH_CONFIG_OS_WIN)
    (void)out;
    return TH_ERR_NOSUPPORT;
#elif defined(TH_CONFIG_OS_MOCK)
    (void)out;
    return TH_ERR_NOSUPPORT;
#endif
}

TH_PRIVATE(th_err)
th_timer_set(th_timer* timer, th_duration duration)
{
    time_t now = 0;
    th_err err = th_timer_monotonic_now(&now);
    TH_ASSERT(err == TH_ERR_OK && "th_timer_monotonic_now failed");
    if (err != TH_ERR_OK)
        return err;
    timer->expire = now + duration.seconds;
    return TH_ERR_OK;
}

TH_PRIVATE(bool)
th_timer_expired(th_timer* timer)
{
    time_t now = 0;
    th_err err = th_timer_monotonic_now(&now);
    TH_ASSERT(err == TH_ERR_OK && "th_timer_monotonic_now failed");
    /* We don't return the error here, as it's already handled in th_timer_set
     * and we can safely assume that the error won't happen here. */
    if (err != TH_ERR_OK)
        return true;
    return now >= timer->expire;
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
        th_task_destroy(task);
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
        th_socket_cancel(th_conn_get_socket(client));
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
th_url_decode_next(th_string str, size_t* pos, char* out, th_url_decode_type type)
{
    size_t i = *pos;
    if (str.ptr[i] == '%') {
        char c = 0;
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
        *out = c;
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

/*
TH_PRIVATE(th_err)
th_url_decode_inplace(char* str, size_t* in_out_len, th_url_decode_type type)
{
    size_t i = 0;
    size_t j = 0;
    size_t len = *in_out_len;
    while (i < len) {
        char c;
        th_err err = th_url_decode_next(str, &i, &c, type);
        if (err != TH_ERR_OK) {
            return err;
        }
        str[j++] = c;
    }
    str[j] = '\0';
    *in_out_len = j;
    return TH_ERR_OK;
}
*/

TH_PRIVATE(th_err)
th_url_decode_string(th_string input, th_heap_string* output, th_url_decode_type type)
{
    th_heap_string_clear(output);

    th_err err = TH_ERR_OK;
    if (input.len == 0)
        return TH_ERR_OK;
    size_t i = 0;
    while (i < input.len) {
        char c;
        if ((err = th_url_decode_next(input, &i, &c, type)) != TH_ERR_OK) {
            return err;
        }
        if ((err = th_heap_string_push_back(output, c)) != TH_ERR_OK) {
            return err;
        }
    }
    return TH_ERR_OK;
}
/* End of src/th_url_decode.c */
/* Start of src/th_path.c */


#include <stdlib.h>

#if defined(TH_CONFIG_OS_POSIX)
#include <errno.h>
#include <limits.h>

TH_LOCAL(th_err)
th_path_resolve_posix(th_string path, th_heap_string* out)
{
    char in[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    size_t pos = 0;
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, path.ptr, path.len);
    in[pos] = '\0';
    th_heap_string_resize(out, TH_CONFIG_MAX_PATH_LEN, 0);
    char* out_ptr = th_heap_string_at(out, 0);
    char* ret = realpath(in, out_ptr);
    if (ret == NULL)
        return TH_ERR_SYSTEM(errno);
    th_heap_string_resize(out, strlen(out_ptr), 0);
    return TH_ERR_OK;
}
/*
TH_LOCAL(th_err)
th_path_resolve_against_posix(th_dir* dir, th_string path, th_heap_string* out)
{
    char in[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    th_string root = th_dir_get_path(dir);
    size_t pos = 0;
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, root.ptr, root.len);
    pos += th_fmt_str_append(in, pos, sizeof(in) - pos, "/");
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, path.ptr, path.len);
    th_heap_string_resize(out, TH_CONFIG_MAX_PATH_LEN, 0);
    char* out_ptr = th_heap_string_data(out);
    char* ret = realpath(in, out_ptr);
    if (ret == NULL)
        return TH_ERR_SYSTEM(errno);
    th_heap_string_resize(out, strlen(out_ptr), 0);

    return TH_ERR_OK;
}
*/
#elif defined(TH_CONFIG_OS_MOCK)
TH_LOCAL(th_err)
th_path_resolve_mock(th_string path, th_heap_string* out)
{
    (void)path;
    th_heap_string_clear(out);
    th_heap_string_set(out, path);
    return TH_ERR_OK;
}
#endif

TH_PRIVATE(th_err)
th_path_resolve(th_string path, th_heap_string* out)
{
#if defined(TH_CONFIG_OS_POSIX)
    return th_path_resolve_posix(path, out);
#elif defined(TH_CONFIG_OS_MOCK)
    return th_path_resolve_mock(path, out);
#else
    (void)path;
    (void)out;
    TH_ASSERT(0 && "Not implemented");
    return TH_ERR_NOSUPPORT;
#endif
}

TH_PRIVATE(th_err)
th_path_resolve_against(th_string path, th_dir* dir, th_heap_string* out)
{
    char in[TH_CONFIG_MAX_PATH_LEN + 1] = {0};
    th_string root = th_dir_get_path(dir);
    size_t pos = 0;
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, root.ptr, root.len);
    pos += th_fmt_str_append(in, pos, sizeof(in) - pos, "/");
    pos += th_fmt_strn_append(in, pos, sizeof(in) - pos, path.ptr, path.len);
    return th_path_resolve(th_string_make(in, pos), out);
}

TH_PRIVATE(bool)
th_path_is_within(th_string realpath, th_dir* dir)
{
    th_string root = th_dir_get_path(dir);
    if (realpath.len < root.len)
        return false;
    return th_string_eq(th_string_make(realpath.ptr, root.len), root);
}

TH_PRIVATE(bool)
th_path_is_hidden(th_string path)
{
    size_t pos = 0;
    while ((pos = th_string_find_first(path, pos, '/')) != th_string_npos) {
        if (path.ptr[++pos] == '.')
            return true;
    }
    return false;
}
/* End of src/th_path.c */
/* Start of src/th_upload.c */

TH_PRIVATE(void)
th_upload_init(th_upload* upload, th_string buffer, th_fcache* fcache, th_allocator* allocator)
{
    th_heap_string_init(&upload->name, allocator);
    th_heap_string_init(&upload->filename, allocator);
    th_heap_string_init(&upload->content_type, allocator);
    upload->data = buffer;
    upload->fcache = fcache;
}

TH_PRIVATE(void)
th_upload_deinit(th_upload* upload)
{
    th_heap_string_deinit(&upload->name);
    th_heap_string_deinit(&upload->filename);
    th_heap_string_deinit(&upload->content_type);
}

TH_PRIVATE(th_err)
th_upload_set_name(th_upload* upload, th_string name)
{
    return th_heap_string_set(&upload->name, name);
}

TH_PRIVATE(th_err)
th_upload_set_filename(th_upload* upload, th_string filename)
{
    return th_heap_string_set(&upload->filename, filename);
}

TH_PRIVATE(th_err)
th_upload_set_content_type(th_upload* upload, th_string content_type)
{
    return th_heap_string_set(&upload->content_type, content_type);
}

// Public API

TH_PUBLIC(th_upload_info)
th_upload_get_info(const th_upload* upload)
{
    return (th_upload_info){
        .name = th_heap_string_data(&upload->name),
        .filename = th_heap_string_data(&upload->filename),
        .content_type = th_heap_string_data(&upload->content_type),
        .size = upload->data.len,
    };
}

TH_PUBLIC(th_buffer)
th_upload_get_data(const th_upload* upload)
{
    return (th_buffer){upload->data.ptr, upload->data.len};
}

TH_PUBLIC(th_err)
th_upload_save(const th_upload* upload, const char* dir_label, const char* filepath)
{
    th_dir* dir = th_fcache_find_dir(upload->fcache, th_string_from_cstr(dir_label));
    if (!dir)
        return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
    th_err err = TH_ERR_OK;
    th_open_opt opt = {.create = true, .write = true, .truncate = true};
    th_file file;
    if ((err = th_file_openat(&file, dir, th_string_from_cstr(filepath), opt)) != TH_ERR_OK)
        return err;
    size_t total_written = 0;
    while (total_written < upload->data.len) {
        size_t written = 0;
        if ((err = th_file_write(&file, upload->data.ptr + total_written, upload->data.len - total_written, total_written, &written)) != TH_ERR_OK) {
            th_file_close(&file);
            return err;
        }
        total_written += written;
    }
    th_file_close(&file);
    return TH_ERR_OK;
}
/* End of src/th_upload.c */
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
    if (th_buf_vec_size(&data->buf) < size) {
        (void)th_buf_vec_resize(&data->buf, size);
        size = th_buf_vec_size(&data->buf);
    }
    return size;
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


#include <openssl/err.h>
#include <openssl/ssl.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_context"

TH_PRIVATE(th_err)
th_ssl_context_init(th_ssl_context* context, const char* key, const char* cert)
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    context->ctx = SSL_CTX_new(TLS_server_method());
    if (!context->ctx) {
        TH_LOG_FATAL("Failed to create SSL context");
        goto cleanup;
    }

    if (SSL_CTX_use_certificate_chain_file(context->ctx, cert) <= 0) {
        TH_LOG_FATAL("Failed to load certificate file");
        goto cleanup;
    }

    if (SSL_CTX_use_PrivateKey_file(context->ctx, key, SSL_FILETYPE_PEM) <= 0) {
        TH_LOG_FATAL("Failed to load private key file");
        goto cleanup;
    }

    if (!SSL_CTX_set_min_proto_version(context->ctx, TLS1_3_VERSION)) {
        TH_LOG_FATAL("Failed to set minimum protocol version");
        goto cleanup;
    }

    if (SSL_CTX_set_cipher_list(context->ctx, "MEDIUM:HIGH:!aNULL!MD5:!RC4!3DES") <= 0) {
        TH_LOG_FATAL("Failed to set cipher list");
        goto cleanup;
    }

    SSL_CTX_set_session_cache_mode(context->ctx, SSL_SESS_CACHE_OFF);
    context->smem_method = NULL;
    return TH_ERR_OK;
cleanup:
    if (context->ctx) {
        SSL_CTX_free(context->ctx);
        context->ctx = NULL;
    }
    return th_ssl_handle_error_stack();
}

TH_PRIVATE(void)
th_ssl_context_deinit(th_ssl_context* context)
{
    if (context->smem_method)
        BIO_meth_free(context->smem_method);
    if (context->ctx)
        SSL_CTX_free(context->ctx);
}
#endif
/* End of src/th_ssl_context.c */
/* Start of src/th_ssl_socket.c */

#if TH_WITH_SSL

#include <assert.h>
#include <errno.h>

#include <openssl/err.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl_socket"

#define TH_SSL_STATE_CLEAR INT_MIN

/* th_ssl_socket functions begin */

TH_LOCAL(void)
th_ssl_socket_set_fd_impl(void* self, int fd);

TH_LOCAL(void)
th_ssl_socket_cancel_impl(void* self);

TH_LOCAL(th_allocator*)
th_ssl_socket_get_allocator_impl(void* self);

TH_LOCAL(th_context*)
th_ssl_socket_get_context_impl(void* self);

TH_LOCAL(void)
th_ssl_socket_async_write_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_writev_impl(void* self, th_iov* addr, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_read_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_readv_impl(void* self, th_iov* iov, size_t len, th_socket_handler* on_complete);

TH_LOCAL(void)
th_ssl_socket_async_sendfile_impl(void* self, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_socket_handler* on_complete);

TH_PRIVATE(th_err)
th_ssl_socket_init(th_ssl_socket* socket, th_context* context, th_ssl_context* ssl_context, th_allocator* allocator)
{
    static const th_socket_methods methods = {
        .set_fd = th_ssl_socket_set_fd_impl,
        .cancel = th_ssl_socket_cancel_impl,
        .get_allocator = th_ssl_socket_get_allocator_impl,
        .get_context = th_ssl_socket_get_context_impl,
        .async_write = th_ssl_socket_async_write_impl,
        .async_writev = th_ssl_socket_async_writev_impl,
        .async_read = th_ssl_socket_async_read_impl,
        .async_readv = th_ssl_socket_async_readv_impl,
        .async_sendfile = th_ssl_socket_async_sendfile_impl,
    };
    socket->base.methods = &methods;
    th_tcp_socket_init(&socket->tcp_socket, context, allocator);

    th_err err = TH_ERR_OK;
    socket->ssl = SSL_new(ssl_context->ctx);
    if (!socket->ssl) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_tcp_socket;
    }
    socket->wbio = BIO_new(th_smem_bio(ssl_context));
    if (!socket->wbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_ssl;
    }
    socket->rbio = BIO_new(th_smem_bio(ssl_context));
    if (!socket->rbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_wbio;
    }
    th_smem_bio_setup_buf(socket->wbio, th_socket_get_allocator(&socket->base), TH_CONFIG_MAX_SSL_WRITE_BUF_LEN);
    th_smem_bio_setup_buf(socket->rbio, th_socket_get_allocator(&socket->base), TH_CONFIG_MAX_SSL_READ_BUF_LEN);
    SSL_set_bio(socket->ssl, socket->rbio, socket->wbio);
    SSL_set_mode(socket->ssl, SSL_MODE_ENABLE_PARTIAL_WRITE);
    return TH_ERR_OK;
cleanup_wbio:
    BIO_free(socket->wbio);
cleanup_ssl:
    SSL_free(socket->ssl);
cleanup_tcp_socket:
    th_tcp_socket_deinit(&socket->tcp_socket);
    if (err == TH_ERR_SSL(SSL_ERROR_SSL))
        th_ssl_log_error_stack();
    return err;
}

TH_LOCAL(void)
th_ssl_socket_set_fd_impl(void* self, int fd)
{
    th_ssl_socket* sock = self;
    th_tcp_socket_set_fd(&sock->tcp_socket, fd);
}

TH_LOCAL(void)
th_ssl_socket_cancel_impl(void* self)
{
    th_ssl_socket* sock = self;
    th_tcp_socket_cancel(&sock->tcp_socket);
}

TH_LOCAL(th_allocator*)
th_ssl_socket_get_allocator_impl(void* self)
{
    th_ssl_socket* sock = self;
    return th_tcp_socket_get_allocator(&sock->tcp_socket);
}

TH_LOCAL(th_context*)
th_ssl_socket_get_context_impl(void* self)
{
    th_ssl_socket* sock = self;
    return th_tcp_socket_get_context(&sock->tcp_socket);
}

TH_PRIVATE(void)
th_ssl_socket_set_mode(th_ssl_socket* socket, th_ssl_socket_mode mode)
{
    if (mode == TH_SSL_SOCKET_MODE_SERVER) {
        SSL_set_accept_state(socket->ssl);
    } else {
        SSL_set_connect_state(socket->ssl);
    }
}

typedef enum th_ssl_io_state {
    TH_SSL_IO_STATE_NONE,
    TH_SSL_IO_STATE_READ,
    TH_SSL_IO_STATE_WRITE,
} th_ssl_io_state;

/* th_ssl_socket helper functions begin */

TH_LOCAL(size_t)
th_ssl_fill_buffer(char* buf, size_t buf_len, th_iov* iov, size_t iov_len)
{
    size_t bufpos = 0;
    for (size_t i = 0; i < iov_len; i++) {
        size_t avail = buf_len - bufpos;
        if (avail == 0)
            break;
        size_t to_copy = TH_MIN(avail, iov[i].len);
        memcpy(buf + bufpos, iov[i].base, to_copy);
        bufpos += to_copy;
    }
    return bufpos;
}

TH_LOCAL(th_err)
th_ssl_socket_write_buffer(th_ssl_socket* s, char* buffer, size_t length, size_t* result)
{
    int ret = SSL_write(s->ssl, buffer, (int)length);
    if (ret > 0) {
        *result = (size_t)ret;
        return TH_ERR_OK;
    } else {
        return TH_ERR_SSL(SSL_get_error(s->ssl, ret));
    }
}

#define TH_SSL_SOCKET_WRITE_BUF_LEN (16 * 1024)
TH_LOCAL(th_err)
th_ssl_socket_writev_with_file(th_ssl_socket* s, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, size_t* result)
{
    char buffer[TH_SSL_SOCKET_WRITE_BUF_LEN];
    size_t iov_total = th_iov_bytes(iov, iovcnt);
    size_t bufpos = th_ssl_fill_buffer(buffer, TH_SSL_SOCKET_WRITE_BUF_LEN, iov, iovcnt);
    if (bufpos < iov_total) { // incomplete write
        return th_ssl_socket_write_buffer(s, buffer, bufpos, result);
    }
    if (stream) {
        size_t bytes_read = 0;
        size_t readlen = TH_MIN(TH_SSL_SOCKET_WRITE_BUF_LEN - bufpos, len);
        th_err err = th_file_read(stream, buffer + bufpos, readlen, offset, &bytes_read);
        if (err != TH_ERR_OK && bufpos == 0) {
            return err;
        }
        bufpos += bytes_read;
    }
    return th_ssl_socket_write_buffer(s, buffer, bufpos, result);
}

TH_LOCAL(th_err)
th_ssl_socket_readv(th_ssl_socket* s, th_iov* iov, size_t len, size_t* out)
{
    th_err err = TH_ERR_OK;
    size_t result = 0;
    for (size_t i = 0; i < len; i++) {
        int ret = SSL_read(s->ssl, iov[i].base, (int)iov[i].len);
        if (ret <= 0) {
            if (result == 0)
                err = TH_ERR_SSL(SSL_get_error(s->ssl, ret));
            break;
        }
        result += (size_t)ret;
        if ((size_t)ret < iov[i].len)
            break;
    }
    *out = result;
    return err;
}

TH_LOCAL(th_err)
th_ssl_socket_handshake(th_ssl_socket* s)
{
    int ret = SSL_do_handshake(s->ssl);
    if (ret == 1) {
        return TH_ERR_OK;
    } else {
        return TH_ERR_SSL(SSL_get_error(s->ssl, ret));
    }
}

/* th_ssl_socket helper functions end */
/* th_ssl_socket_io_handler begin */

/** th_ssl_socket_io_handler
 * @brief I/O handler for SSL socket.
 */
typedef struct th_ssl_socket_io_handler {
    th_io_composite base;
    th_allocator* allocator;
    th_iov buffer;
    th_ssl_socket* socket;
    void (*handle_result)(void* self, size_t result);
    size_t result; // last successful SSL_read/SSL_write/SSL_handshake result
    size_t depth;
    th_ssl_io_state state;
} th_ssl_socket_io_handler;

TH_LOCAL(void)
th_ssl_socket_io_handler_destroy(void* self)
{
    th_ssl_socket_io_handler* handler = self;
    th_allocator_free(handler->allocator, handler);
}

TH_LOCAL(void)
th_ssl_socket_io_handler_complete(th_ssl_socket_io_handler* handler, size_t result, th_err err)
{
    if (handler->depth > 0) {
        th_io_composite_complete((th_io_composite*)handler, result, err);
    } else {
        th_context_dispatch_composite_completion(th_socket_get_context((th_socket*)handler->socket), (th_io_composite*)handler, result, err);
        th_ssl_socket_io_handler_destroy(handler);
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_read_fn(th_ssl_socket_io_handler* handler, size_t result, th_err err)
{
    th_ssl_socket* socket = handler->socket;
    if (err != TH_ERR_OK) {
        th_smem_bio_set_eof(socket->rbio);
        th_ssl_socket_io_handler_complete(handler, 0, err);
        return;
    }
    th_smem_bio_inc_write_pos(socket->rbio, result);
    handler->handle_result(handler, handler->result);
}

TH_LOCAL(void)
th_ssl_socket_io_handler_write_fn(th_ssl_socket_io_handler* handler, size_t result, th_err err)
{
    th_ssl_socket* socket = handler->socket;
    if (err != TH_ERR_OK) {
        th_ssl_socket_io_handler_complete(handler, 0, err);
        return;
    }
    th_smem_bio_inc_read_pos(socket->wbio, result);
    handler->handle_result(handler, handler->result);
}

TH_LOCAL(void)
th_ssl_socket_io_handler_fn(void* self, size_t result, th_err err)
{
    th_ssl_socket_io_handler* handler = self;
    ++handler->depth;
    switch (handler->state) {
    case TH_SSL_IO_STATE_READ:
        handler->state = TH_SSL_IO_STATE_NONE; // reset state
        th_ssl_socket_io_handler_read_fn(handler, result, err);
        break;
    case TH_SSL_IO_STATE_WRITE:
        handler->state = TH_SSL_IO_STATE_NONE; // reset state
        th_ssl_socket_io_handler_write_fn(handler, result, err);
        break;
    case TH_SSL_IO_STATE_NONE:
        th_ssl_socket_io_handler_complete(handler, result, err);
        break;
    default:
        TH_ASSERT(0 && "Invalid state");
        break;
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_init(th_ssl_socket_io_handler* handler, th_ssl_socket* socket,
                              void (*handle_result)(void* self, size_t len),
                              th_socket_handler* on_complete, th_allocator* allocator)
{
    th_io_composite_init(&handler->base, th_ssl_socket_io_handler_fn, th_ssl_socket_io_handler_destroy, on_complete);
    handler->allocator = allocator;
    handler->socket = socket;
    handler->handle_result = handle_result;
    handler->state = TH_SSL_IO_STATE_NONE;
    handler->depth = 0;
    handler->result = 0;
}

TH_LOCAL(void)
th_ssl_socket_io_handler_writev_with_file(th_ssl_socket_io_handler* handler, th_iov* iov, size_t iovcnt,
                                          th_file* stream, size_t offset, size_t len, th_io_composite_forward_type type)
{
    th_err err = TH_ERR_OK;
    size_t result = 0;
    th_ssl_socket* socket = handler->socket;
    if ((err = th_ssl_socket_writev_with_file(socket, iov, iovcnt,
                                              stream, offset, len, &result))
        != TH_ERR_OK) {
        if (TH_ERR_CODE(err) == SSL_ERROR_WANT_READ) {
            TH_LOG_TRACE("SSL_write wants read, switching to async read");
            handler->state = TH_SSL_IO_STATE_READ;
            th_smem_bio_get_wbuf(socket->rbio, &handler->buffer);
            th_tcp_socket_async_read(&socket->tcp_socket, handler->buffer.base, handler->buffer.len,
                                     (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else {
            th_ssl_socket_io_handler_complete(handler, result, err);
        }
    } else {
        TH_LOG_TRACE("SSL_write %d bytes", (int)result);
        handler->result = result;
        handler->state = TH_SSL_IO_STATE_WRITE;
        th_smem_bio_get_rdata(socket->wbio, &handler->buffer);
        th_socket_async_write_exact(&socket->tcp_socket.base, handler->buffer.base, handler->buffer.len,
                                    (th_socket_handler*)th_io_composite_forward(&handler->base, type));
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_readv(th_ssl_socket_io_handler* handler, th_iov* iov, size_t iovcnt,
                               th_io_composite_forward_type type)
{
    th_err err = TH_ERR_OK;
    size_t result = 0;
    th_ssl_socket* socket = handler->socket;
    if (((err = th_ssl_socket_readv(socket, iov, iovcnt, &result)) != TH_ERR_OK)
        || (BIO_pending(socket->wbio) > 0)) {
        if (BIO_pending(socket->wbio) > 0) {
            th_smem_bio_get_rdata(socket->wbio, &handler->buffer);
            TH_LOG_TRACE("SSL_read wants write, switching to async write");
            handler->state = TH_SSL_IO_STATE_WRITE;
            if (result > 0)
                handler->result = result;
            th_socket_async_write_exact(&socket->tcp_socket.base, handler->buffer.base, handler->buffer.len,
                                        (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_WANT_READ) {
            TH_LOG_TRACE("SSL_read wants read, switching to async read");
            handler->state = TH_SSL_IO_STATE_READ;
            th_smem_bio_get_wbuf(socket->rbio, &handler->buffer);
            th_tcp_socket_async_read(&socket->tcp_socket, handler->buffer.base, handler->buffer.len,
                                     (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_ZERO_RETURN) {
            TH_LOG_TRACE("SSL_read zero return");
            th_ssl_socket_io_handler_complete(handler, 0, TH_ERR_EOF);
        } else {
            th_ssl_log_error_stack();
            th_ssl_socket_io_handler_complete(handler, 0, err);
        }
    } else {
        th_ssl_socket_io_handler_complete(handler, result, TH_ERR_OK);
    }
}

TH_LOCAL(void)
th_ssl_socket_io_handler_handshake(th_ssl_socket_io_handler* handler,
                                   th_io_composite_forward_type type)
{
    th_err err = TH_ERR_OK;
    th_ssl_socket* socket = handler->socket;
    if (((err = th_ssl_socket_handshake(socket)) != TH_ERR_OK)
        || (BIO_pending(socket->wbio) > 0)) {
        if (BIO_pending(socket->wbio) > 0) {
            if (err == TH_ERR_OK)
                handler->result = 1; // handshake done
            th_smem_bio_get_rdata(socket->wbio, &handler->buffer);
            TH_LOG_TRACE("SSL_handshake wants write, switching to async write");
            handler->state = TH_SSL_IO_STATE_WRITE;
            th_socket_async_write_exact(&socket->tcp_socket.base, handler->buffer.base, handler->buffer.len,
                                        (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_WANT_READ) {
            TH_LOG_TRACE("SSL_handshake wants read, switching to async read");
            handler->state = TH_SSL_IO_STATE_READ;
            th_smem_bio_get_wbuf(socket->rbio, &handler->buffer);
            th_tcp_socket_async_read(&socket->tcp_socket, handler->buffer.base, handler->buffer.len,
                                     (th_socket_handler*)th_io_composite_forward(&handler->base, type));
        } else if (TH_ERR_CODE(err) == SSL_ERROR_ZERO_RETURN) {
            TH_LOG_TRACE("SSL_handshake zero return");
            th_ssl_socket_io_handler_complete(handler, 0, TH_ERR_EOF);
        } else {
            th_ssl_log_error_stack();
            th_ssl_socket_io_handler_complete(handler, 0, err);
        }
    } else {
        th_ssl_socket_io_handler_complete(handler, 1, TH_ERR_OK);
    }
}

/* th_ssl_socket_async_writev begin */
/* th_ssl_socket_async_write begin */

TH_LOCAL(void)
th_ssl_socket_async_write_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete)
{
    th_socket* socket = self;
    (void)addr;
    (void)len;
    TH_LOG_ERROR("th_ssl_socket_async_write not implemented");
    th_context_dispatch_handler(th_socket_get_context(socket), on_complete, 0, TH_ERR_NOSUPPORT);
}

/* th_ssl_socket_async_write end */
/* th_ssl_socket_async_writev begin */

typedef struct th_ssl_socket_write_handler {
    th_ssl_socket_io_handler base;
    th_iov* addr;
    size_t len;
} th_ssl_socket_writev_handler;

TH_LOCAL(void)
th_ssl_socket_writev_handler_fn(void* self, size_t result)
{
    th_ssl_socket_writev_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_writev_with_file(&handler->base, handler->addr, handler->len, NULL, 0, 0, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_writev_handler_create(th_ssl_socket_writev_handler** out, th_ssl_socket* socket, th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_writev_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_writev_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_ssl_socket_io_handler_init(&handler->base, socket,
                                  th_ssl_socket_writev_handler_fn, on_complete, allocator);
    handler->addr = NULL;
    handler->len = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_socket_async_writev_impl(void* self, th_iov* addr, size_t len, th_socket_handler* on_complete)
{
    TH_ASSERT(self);
    TH_ASSERT(addr);
    TH_ASSERT(on_complete);
    th_err err = TH_ERR_OK;
    th_ssl_socket* socket = self;
    th_ssl_socket_writev_handler* handler = NULL;
    if ((err = th_ssl_socket_writev_handler_create(&handler, socket, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(&socket->base), on_complete, 0, err);
        return;
    }
    handler->addr = addr;
    handler->len = len;
    th_ssl_socket_io_handler_writev_with_file(&handler->base, addr, len, NULL, 0, 0, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_writev end */
/* th_ssl_socket_async_read begin */

typedef struct th_ssl_socket_read_handler {
    th_ssl_socket_io_handler base;
    th_iov iov;
} th_ssl_socket_read_handler;

TH_LOCAL(void)
th_ssl_socket_read_handler_fn(void* self, size_t result)
{
    th_ssl_socket_read_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_readv(&handler->base, &handler->iov, 1, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_read_handler_create(th_ssl_socket_read_handler** out, th_ssl_socket* socket,
                                  th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_read_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_read_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_ssl_socket_io_handler_init(&handler->base, socket, th_ssl_socket_read_handler_fn, on_complete, allocator);
    handler->iov = (th_iov){0};
    *out = handler;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_socket_async_read_impl(void* self, void* addr, size_t len, th_socket_handler* on_complete)
{
    TH_ASSERT(self);
    TH_ASSERT(addr);
    TH_ASSERT(on_complete);
    th_err err = TH_ERR_OK;
    th_ssl_socket* socket = self;
    th_ssl_socket_read_handler* handler = NULL;
    if ((err = th_ssl_socket_read_handler_create(&handler, socket, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(&socket->base), on_complete, 0, err);
        return;
    }
    handler->iov = (th_iov){.base = addr, .len = len};
    th_ssl_socket_io_handler_readv(&handler->base, &handler->iov, 1, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_read end */
/* th_ssl_socket_async_readv begin */

TH_LOCAL(void)
th_ssl_socket_async_readv_impl(void* self, th_iov* iov, size_t len, th_socket_handler* on_complete)
{
    (void)self;
    (void)iov;
    (void)len;
    (void)on_complete;
    // Don't support readv for now as we don't need it
    TH_ASSERT(0 && "Not implemented");
    return;
}

/* th_ssl_socket_async_readv end */
/* th_ssl_socket_async_sendfile begin */

typedef struct th_ssl_sendfile_handler {
    th_ssl_socket_io_handler base;
    th_iov* headers;
    size_t num_headers;
    th_file* stream;
    size_t offset;
    size_t len;
} th_ssl_socket_sendfile_handler;

TH_LOCAL(void)
th_ssl_socket_sendfile_handler_fn(void* self, size_t result)
{
    th_ssl_socket_sendfile_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_writev_with_file(&handler->base, handler->headers, handler->num_headers, handler->stream, handler->offset, handler->len, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_sendfile_handler_create(th_ssl_socket_sendfile_handler** out, th_ssl_socket* socket,
                                      th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_sendfile_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_sendfile_handler));
    if (!handler)
        return TH_ERR_BAD_ALLOC;
    th_ssl_socket_io_handler_init(&handler->base, socket, th_ssl_socket_sendfile_handler_fn, on_complete, allocator);
    handler->headers = NULL;
    handler->num_headers = 0;
    handler->stream = NULL;
    handler->offset = 0;
    *out = handler;
    return TH_ERR_OK;
}

TH_LOCAL(void)
th_ssl_socket_async_sendfile_impl(void* self, th_iov* iov, size_t iovcnt, th_file* stream, size_t offset, size_t len, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_ssl_socket* sock = self;
    th_ssl_socket_sendfile_handler* handler = NULL;
    if ((err = th_ssl_socket_sendfile_handler_create(&handler, sock, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(self), on_complete, 0, err);
        return;
    }
    handler->headers = iov;
    handler->num_headers = iovcnt;
    handler->stream = stream;
    handler->offset = offset;
    th_ssl_socket_io_handler_writev_with_file(&handler->base, iov, iovcnt, stream, offset, len, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_sendfile end */
/* th_ssl_socket_async_handshake begin */

typedef struct th_ssl_socket_handshake_handler {
    th_ssl_socket_io_handler base;
} th_ssl_socket_handshake_handler;

TH_LOCAL(void)
th_ssl_socket_handshake_handler_fn(void* self, size_t result)
{
    th_ssl_socket_handshake_handler* handler = self;
    if (result > 0) {
        th_ssl_socket_io_handler_complete(&handler->base, result, TH_ERR_OK);
    } else {
        th_ssl_socket_io_handler_handshake(&handler->base, TH_IO_COMPOSITE_FORWARD_COPY);
    }
}

TH_LOCAL(th_err)
th_ssl_socket_handshake_handler_create(th_ssl_socket_handshake_handler** out, th_ssl_socket* socket, th_socket_handler* on_complete)
{
    th_allocator* allocator = th_socket_get_allocator((th_socket*)socket);
    th_ssl_socket_handshake_handler* handler = th_allocator_alloc(allocator, sizeof(th_ssl_socket_handshake_handler));
    if (!handler) {
        return TH_ERR_BAD_ALLOC;
    }
    th_ssl_socket_io_handler_init(&handler->base, socket, th_ssl_socket_handshake_handler_fn, on_complete, allocator);
    *out = handler;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_ssl_socket_async_handshake(th_ssl_socket* socket, th_socket_handler* on_complete)
{
    th_err err = TH_ERR_OK;
    th_ssl_socket_handshake_handler* handler = NULL;
    if ((err = th_ssl_socket_handshake_handler_create(&handler, socket, on_complete)) != TH_ERR_OK) {
        th_context_dispatch_handler(th_socket_get_context(&socket->base), on_complete, 0, err);
        return;
    }
    th_smem_ensure_buf_size(socket->rbio, TH_CONFIG_SMALL_SSL_BUF_LEN);
    th_ssl_socket_io_handler_handshake(&handler->base, TH_IO_COMPOSITE_FORWARD_MOVE);
}

/* th_ssl_socket_async_handshake end */
/* th_ssl_socket_async_shutdown begin */

TH_PRIVATE(void)
th_ssl_socket_async_shutdown(th_ssl_socket* socket, th_socket_handler* on_complete)
{
    (void)socket;
    (void)on_complete;
    TH_ASSERT(0 && "Not implemented");
}

/* th_ssl_socket_async_shutdown end */

TH_PRIVATE(void)
th_ssl_socket_close(th_ssl_socket* sock)
{
    th_tcp_socket_close(&sock->tcp_socket);
}

TH_PRIVATE(void)
th_ssl_socket_deinit(th_ssl_socket* sock)
{
    th_ssl_socket_close(sock);
    SSL_free(sock->ssl);
    th_tcp_socket_deinit(&sock->tcp_socket);
}
#endif
/* End of src/th_ssl_socket.c */
/* Start of src/th_ssl_error.c */

#if TH_WITH_SSL


#include <openssl/err.h>
#include <openssl/ssl.h>

#undef TH_LOG_TAG
#define TH_LOG_TAG "ssl"

TH_PRIVATE(void)
th_ssl_log_error_stack(void)
{
    unsigned long code;
    while ((code = ERR_get_error())) {
        TH_LOG_ERROR("%s", ERR_reason_error_string(code));
    }
}

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

TH_PRIVATE(th_err)
th_ssl_handle_error_stack(void)
{
    th_ssl_log_error_stack();
    return TH_ERR_SSL(SSL_ERROR_SSL);
}

#endif // TH_WITH_SSL
/* End of src/th_ssl_error.c */
