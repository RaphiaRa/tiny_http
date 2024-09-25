#ifndef TH_REFCOUNTED_H
#define TH_REFCOUNTED_H

#include <th.h>

typedef struct th_refcounted {
    unsigned int refcount;
    void (*destroy)(void* self);
} th_refcounted;

TH_INLINE(void)
th_refcounted_init(th_refcounted* refcounted, void (*destroy)(void* self))
{
    refcounted->refcount = 1;
    refcounted->destroy = destroy;
}

TH_INLINE(th_refcounted*)
th_refcounted_ref(th_refcounted* refcounted)
{
    ++refcounted->refcount;
    return refcounted;
}

TH_INLINE(void)
th_refcounted_unref(th_refcounted* refcounted)
{
    TH_ASSERT(refcounted->refcount > 0 && "Invalid refcount");
    if (--refcounted->refcount == 0) {
        refcounted->destroy(refcounted);
    }
}

#endif
