#ifndef TH_IO_OP_LINUX_H
#define TH_IO_OP_LINUX_H

#include <th.h>

#include "th_config.h"

#if defined(TH_CONFIG_WITH_LINUX_SENDFILE)
TH_PRIVATE(th_err)
th_io_op_linux_sendfile(void* self, size_t* result) TH_MAYBE_UNUSED;
#endif

#endif
