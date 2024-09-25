#include "th_io_op_mock.h"

#if defined(TH_CONFIG_OS_MOCK)

#include "th_io_task.h"
#include "th_mock_syscall.h"

TH_PRIVATE(th_err)
th_io_op_mock_read(void* self, size_t* result)
{
    th_io_task* iot = self;
    int r = th_mock_read(iot->addr, iot->len);
    if (r < 0)
        return TH_ERR_SYSTEM(-r);
    else if (r == 0)
        return TH_ERR_EOF;
    *result = r;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_io_op_mock_readv(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    *result = 0;
    for (size_t i = 0; i < iot->len; ++i) {
        int r = th_mock_read(iov[i].base, iov[i].len);
        if (r < 0) {
            if (*result == 0)
                err = TH_ERR_SYSTEM(-r);
            break;
        } else if (r == 0 && *result == 0) {
            err = TH_ERR_EOF;
            break;
        }
        *result += r;
    }
    return err;
}

TH_PRIVATE(th_err)
th_io_op_mock_write(void* self, size_t* result)
{
    th_io_task* iot = self;
    int r = th_mock_write(iot->len);
    if (r < 0)
        return TH_ERR_SYSTEM(-r);
    *result = (size_t)r;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_io_op_mock_writev(void* self, size_t* result)
{
    th_err err = TH_ERR_OK;
    th_io_task* iot = self;
    th_iov* iov = iot->addr;
    *result = 0;
    for (size_t i = 0; i < iot->len; ++i) {
        int r = th_mock_write(iov[i].len);
        if (r < 0) {
            if (*result == 0)
                err = TH_ERR_SYSTEM(-r);
            break;
        }
        *result += r;
    }
    return err;
}

TH_PRIVATE(th_err)
th_io_op_mock_send(void* self, size_t* result)
{
    return th_io_op_mock_write(self, result);
}

TH_PRIVATE(th_err)
th_io_op_mock_sendv(void* self, size_t* result)
{
    return th_io_op_mock_writev(self, result);
}

TH_PRIVATE(th_err)
th_io_op_mock_accept(void* self, size_t* result)
{
    (void)self;
    int r = th_mock_accept();
    if (r < 0)
        return TH_ERR_SYSTEM(-r);
    *result = (size_t)r;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_io_op_mock_sendfile(void* self, size_t* result)
{
    th_io_task* iot = self;
    size_t len = th_iov_bytes(iot->addr, iot->len);
    len += iot->len2;
    int r = th_mock_write(len);
    if (r < 0)
        return TH_ERR_SYSTEM(-r);
    *result = (size_t)r;
    return TH_ERR_OK;
}

#endif
