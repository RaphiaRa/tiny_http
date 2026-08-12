#include "th_test.h"
#include "th_allocator.h"

#include <assert.h>
#include <stdlib.h>

typedef struct th_alloc_list {
    void* ptr;
    struct th_alloc_list* next;
} th_alloc_list;

typedef struct th_test_allocator {
    th_allocator base;
    th_alloc_list* list;
    int fail_countdown; /* -1 = disabled; 0 = fail the next alloc/realloc */
} th_test_allocator;

/* -1 (disabled) never reaches 0, so it's left untouched. */
static bool
th_test_allocator_should_fail(th_test_allocator* allocator)
{
    if (allocator->fail_countdown < 0)
        return false;
    if (allocator->fail_countdown-- == 0) {
        allocator->fail_countdown = -1;
        return true;
    }
    return false;
}

static void*
th_test_allocator_track(th_test_allocator* allocator, size_t size)
{
    void* ptr = calloc(1, size);
    if (!ptr)
        return NULL;
    th_alloc_list* node = calloc(1, sizeof(th_alloc_list));
    if (!node) {
        free(ptr);
        return NULL;
    }
    node->ptr = ptr;
    node->next = allocator->list;
    allocator->list = node;
    return ptr;
}

static void*
th_test_allocator_alloc(void* self, size_t size)
{
    th_test_allocator* allocator = self;
    if (th_test_allocator_should_fail(allocator))
        return NULL;
    return th_test_allocator_track(allocator, size);
}

static void*
th_test_allocator_realloc(void* self, void* ptr, size_t size)
{
    th_test_allocator* allocator = self;
    if (th_test_allocator_should_fail(allocator))
        return NULL;
    for (th_alloc_list* node = allocator->list; node != NULL; node = node->next) {
        if (node->ptr == ptr) {
            void* new_ptr = realloc(ptr, size);
            if (!new_ptr)
                return NULL;
            node->ptr = new_ptr;
            return new_ptr;
        }
    }
    return th_test_allocator_track(allocator, size);
}

static void
th_test_allocator_free(void* self, void* ptr)
{
    th_test_allocator* allocator = self;
    th_alloc_list* prev = NULL;
    for (th_alloc_list* node = allocator->list; node != NULL; node = node->next) {
        if (node->ptr == ptr) {
            if (prev)
                prev->next = node->next;
            else
                allocator->list = node->next;
            free(node->ptr);
            free(node);
            return;
        }
        prev = node;
    }
    assert(0 && "th_test_allocator_free: invalid pointer");
}

int th_test_allocator_outstanding(void)
{
    th_test_allocator* allocator = (th_test_allocator*)th_default_allocator_get();
    int count = 0;
    for (th_alloc_list* node = allocator->list; node != NULL; node = node->next)
        ++count;
    return count;
}

void th_test_allocator_fail_after(int n)
{
    th_test_allocator* allocator = (th_test_allocator*)th_default_allocator_get();
    allocator->fail_countdown = n;
}

void th_test_setup(void)
{
    static th_test_allocator allocator = {
        .base = {
            .alloc = th_test_allocator_alloc,
            .realloc = th_test_allocator_realloc,
            .free = th_test_allocator_free,
        },
        .list = NULL,
        .fail_countdown = -1,
    };
    th_default_allocator_set(&allocator.base);
}

void th_test_teardown(void)
{
    th_test_allocator* allocator = (th_test_allocator*)th_default_allocator_get();
    for (th_alloc_list* node = allocator->list; node != NULL;) {
        th_alloc_list* next = node->next;
        free(node->ptr);
        free(node);
        node = next;
    }
    allocator->list = NULL;
    allocator->fail_countdown = -1;
    th_default_allocator_set(NULL);
}
