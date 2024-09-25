#include "th_align.h"
#include "th_allocator.h"
#include "th_test.h"

#include <stdint.h>

typedef struct th_test_object {
    struct th_test_object* next;
    struct th_test_object* prev;
} th_test_object;

TH_DEFINE_OBJ_POOL_ALLOCATOR(th_test_pool_allocator, th_test_object, prev, next)

TH_TEST_BEGIN(allocator)
{
    TH_TEST_CASE_BEGIN(default_allocator)
    {
        th_allocator* allocator = th_default_allocator_get();
        void* ptr = th_allocator_alloc(allocator, 1024);
        TH_EXPECT(ptr != NULL);
        th_allocator_free(allocator, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), NULL);
        void* ptr = th_allocator_alloc(&allocator.base, 1000);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        TH_EXPECT((ptrdiff_t)ptr % TH_ALIGNOF(th_max_align) == 0);
        th_allocator_free(&allocator.base, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator_overflow_with_fallback)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), th_default_allocator_get());
        void* ptr = th_allocator_alloc(&allocator.base, 2000);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf || (char*)ptr < (char*)buf + sizeof(buf));
        th_allocator_free(&allocator.base, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator_overflow_without_fallback)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), NULL);
        void* ptr = th_allocator_alloc(&allocator.base, 2000);
        TH_EXPECT(ptr == NULL);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator_alloc_release)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), NULL);
        void* ptr = th_allocator_alloc(&allocator.base, 1000);
        th_allocator_free(&allocator.base, ptr);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        ptr = th_allocator_alloc(&allocator.base, 1000);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        th_allocator_free(&allocator.base, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator_realloc)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), NULL);
        void* ptr = th_allocator_alloc(&allocator.base, 1000);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        ptr = th_allocator_realloc(&allocator.base, ptr, 500);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        th_allocator_free(&allocator.base, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator_realloc_null)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), NULL);
        void* ptr = th_allocator_realloc(&allocator.base, NULL, 500);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        th_allocator_free(&allocator.base, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(arena_allocator_realloc_out_of_bounds_fallback)
    {
        th_arena_allocator allocator = {0};
        uint8_t buf[1024] = {0};
        th_arena_allocator_init(&allocator, buf, sizeof(buf), th_default_allocator_get());
        void* ptr = th_allocator_alloc(&allocator.base, 1000);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr >= (char*)buf && (char*)ptr < (char*)buf + sizeof(buf));
        ptr = th_allocator_realloc(&allocator.base, ptr, 2000);
        TH_EXPECT(ptr != NULL);
        TH_EXPECT((char*)ptr < (char*)buf || (char*)ptr >= (char*)buf + sizeof(buf));
        th_allocator_free(&allocator.base, ptr);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(pool_allocator)
    {
        th_test_pool_allocator pool = {0};
        th_test_pool_allocator_init(&pool, th_default_allocator_get(), 10, 10);
        th_test_object* obj = (th_test_object*)th_test_pool_allocator_alloc(&pool, sizeof(th_test_object));
        TH_EXPECT(obj != NULL);
        th_test_pool_allocator_free(&pool, obj);
        th_test_pool_allocator_deinit(&pool);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(pool_allocator_alloc_free)
    {
        th_test_pool_allocator pool = {0};
        th_test_pool_allocator_init(&pool, th_default_allocator_get(), 10, 10);
        th_test_object* obj = (th_test_object*)th_test_pool_allocator_alloc(&pool, sizeof(th_test_object));
        TH_EXPECT(obj != NULL);
        th_test_pool_allocator_free(&pool, obj);
        obj = (th_test_object*)th_test_pool_allocator_alloc(&pool, sizeof(th_test_object));
        TH_EXPECT(obj != NULL);
        th_test_pool_allocator_free(&pool, obj);
        th_test_pool_allocator_deinit(&pool);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(pool_allocator_alloc_multiple)
    {
        th_test_pool_allocator pool = {0};
        th_test_pool_allocator_init(&pool, th_default_allocator_get(), 10, 10);
        th_test_object* obj1 = (th_test_object*)th_test_pool_allocator_alloc(&pool, sizeof(th_test_object));
        th_test_object* obj2 = (th_test_object*)th_test_pool_allocator_alloc(&pool, sizeof(th_test_object));
        th_test_object* obj3 = (th_test_object*)th_test_pool_allocator_alloc(&pool, sizeof(th_test_object));
        TH_EXPECT(obj1 != NULL);
        TH_EXPECT(obj2 != NULL);
        TH_EXPECT(obj3 != NULL);
        th_test_pool_allocator_free(&pool, obj1);
        th_test_pool_allocator_free(&pool, obj2);
        th_test_pool_allocator_free(&pool, obj3);
        th_test_pool_allocator_deinit(&pool);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
