#ifndef TH_IO_COMPOSITE_H
#define TH_IO_COMPOSITE_H

#include "th_io_task.h"
#include "th_utility.h"

/** th_io_composite
 *@brief I/O composite task, inherits from th_io_handler.
 * and contains a pointer to another I/O handler that will be called
 * when the composite task is completed.
 */
typedef struct th_io_composite {
    th_io_handler base;
    th_io_handler* on_complete;
    void (*destroy)(void* self);
    unsigned int refcount;
} th_io_composite;

TH_PRIVATE(void)
th_io_composite_unref(void* self);

TH_INLINE(void)
th_io_composite_init(th_io_composite* composite, void (*fn)(void* self, size_t result, th_err err), void (*destroy)(void* self), th_io_handler* on_complete)
{
    th_io_handler_init(&composite->base, fn, th_io_composite_unref);
    composite->destroy = destroy;
    composite->on_complete = on_complete;
    composite->refcount = 1;
}

static inline void
th_io_composite_complete(th_io_composite* composite, size_t result, th_err err)
{
    th_io_handler_complete(composite->on_complete, result, err);
}

TH_INLINE(th_io_composite*)
th_io_composite_ref(th_io_composite* composite)
{
    ++composite->refcount;
    return composite;
}

typedef enum th_io_composite_forward_type {
    TH_IO_COMPOSITE_FORWARD_MOVE,
    TH_IO_COMPOSITE_FORWARD_COPY
} th_io_composite_forward_type;

TH_INLINE(th_io_composite*)
th_io_composite_forward(th_io_composite* composite, th_io_composite_forward_type type) TH_MAYBE_UNUSED;

TH_INLINE(th_io_composite*)
th_io_composite_forward(th_io_composite* composite, th_io_composite_forward_type type)
{
    switch (type) {
    case TH_IO_COMPOSITE_FORWARD_MOVE:
        return composite;
    case TH_IO_COMPOSITE_FORWARD_COPY:
        return th_io_composite_ref(composite);
        break;
    default:
        return NULL;
        break;
    }
}

#endif
