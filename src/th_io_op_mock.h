#ifndef TH_IO_OP_MOCK_H
#define TH_IO_OP_MOCK_H

#include <th.h>

#include "th_config.h"

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

#endif
