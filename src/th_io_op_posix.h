#ifndef IO_OP_UNIX_H
#define IO_OP_UNIX_H

#include <th.h>

#include "th_config.h"

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
#endif
