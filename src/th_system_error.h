#ifndef TH_SYSTEM_ERR_H
#define TH_SYSTEM_ERR_H

#include "th_config.h"

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

#endif
