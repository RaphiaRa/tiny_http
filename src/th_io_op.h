#ifndef TH_IO_OP_H
#define TH_IO_OP_H

#include <th.h>

#include "th_config.h"

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

#endif
