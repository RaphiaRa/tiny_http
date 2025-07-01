#include <th.h>

#include "th_io_op_bsd.h"

#if defined(TH_CONFIG_WITH_BSD_SENDFILE)
#include "th_file.h"
#include "th_io_op_posix.h"
#include "th_io_task.h"
#include "th_system_error.h"

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
