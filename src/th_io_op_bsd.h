#ifndef IO_OP_BSD_H
#define IO_OP_BSD_H

#include "th_config.h"

#if defined(TH_CONFIG_WITH_BSD_SENDFILE)
TH_PRIVATE(th_err)
th_io_op_bsd_sendfile(void* self, size_t* result) TH_MAYBE_UNUSED;
#endif

#endif
