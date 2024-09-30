#ifndef TH_CONFIG_H
#define TH_CONFIG_H

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
//#define TH_CONFIG_WITH_LINUX_SENDFILE 1
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
#define TH_CONFIG_MAX_CONNECTIONS 128
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

#endif
