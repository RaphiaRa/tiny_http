#include "th_io_composite.h"

TH_PRIVATE(void)
th_io_composite_unref(void* self)
{
    th_io_composite* composite = self;
    TH_ASSERT(composite->refcount > 0 && "Invalid refcount");
    if (--composite->refcount == 0) {
        if (composite->on_complete)
            th_io_handler_destroy(TH_MOVE_PTR(composite->on_complete));
        composite->destroy(composite);
    }
}
