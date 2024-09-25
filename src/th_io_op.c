#include "th_io_op.h"

#include "th_io_op_bsd.h"
#include "th_io_op_linux.h"
#include "th_io_op_mock.h"
#include "th_io_op_posix.h"
#include "th_io_task.h"

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
