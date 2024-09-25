#include "th_io_op_linux.h"

#if defined(TH_CONFIG_WITH_LINUX_SENDFILE)
TH_PRIVATE(th_err)
th_io_op_linux_sendfile(void* self, size_t* result)
{
    (void)self;
    (void)result;
    return TH_ERR_NOSUPPORT;
}
#endif
