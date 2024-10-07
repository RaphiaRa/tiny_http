#include "th_hashmap.h"
#include "th_test.h"

#include <stdint.h>
#include <string.h>

static uint32_t th_test_hash(const char* str)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        hash ^= str[i];
        hash *= 16777619;
    }
    return hash;
}

static bool th_test_eq(const char* a, const char* b)
{
    if (a == b)
        return true;
    return a && b && strcmp(a, b) == 0;
}

TH_DEFINE_HASHMAP(th_test_map, const char*, int, th_test_hash, th_test_eq, NULL)

static uint32_t th_test_hash_int(int i)
{
    return (uint32_t)i;
}

static bool th_test_eq_int(int a, int b)
{
    return a == b;
}

TH_DEFINE_HASHMAP(th_test_map_int, int, int, th_test_hash_int, th_test_eq_int, -1)

TH_TEST_BEGIN(hashmap)
{
    TH_TEST_CASE_BEGIN(hashmap_init)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(map.capacity == 0);
        TH_EXPECT(map.size == 0);
        TH_EXPECT(map.entries == NULL);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_set)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", 3) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", 4) == TH_ERR_OK);
        int* value = th_test_map_try_get(&map, "key2");
        TH_EXPECT(value && *value == 3);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_try_get)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", 3) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", 4) == TH_ERR_OK);
        int* value = th_test_map_try_get(&map, "key2");
        TH_EXPECT(value && *value == 3);
        value = th_test_map_try_get(&map, "key4");
        TH_EXPECT(value == NULL);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_set_existing)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key", 2) == TH_ERR_OK);
        int* value = th_test_map_try_get(&map, "key");
        TH_EXPECT(value && *value == 2);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_set_many)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        char* key_buf[1000];
        int val_buf[1000];
        for (size_t i = 0; i < 1000; i++) {
            key_buf[i] = th_allocator_alloc(th_default_allocator_get(), 32);
            snprintf(key_buf[i], 32, "key%d", (int)i);
            val_buf[i] = (int)i;
            TH_EXPECT(th_test_map_set(&map, key_buf[i], val_buf[i]) == TH_ERR_OK);
        }
        for (size_t i = 0; i < 1000; i++) {
            int* value = th_test_map_try_get(&map, key_buf[i]);
            TH_EXPECT(value && *value == val_buf[i]);
        }
        for (size_t i = 0; i < 1000; i++) {
            th_allocator_free(th_default_allocator_get(), key_buf[i]);
        }
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_iterator)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        int values[4] = {0, 1, 2, 3};
        TH_EXPECT(th_test_map_set(&map, "key", values[0]) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", values[1]) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", values[2]) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", values[3]) == TH_ERR_OK);
        th_test_map_iter iter = th_test_map_begin(&map);
        while (iter != NULL) {
            TH_EXPECT(iter->value == values[iter->value]);
            values[iter->value] = -1;
            iter = th_test_map_next(&map, iter);
        }
        for (size_t i = 0; i < 4; i++) {
            TH_EXPECT(values[i] == -1);
        }
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_erase_middle)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", 3) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", 4) == TH_ERR_OK);
        th_test_map_entry* entry = th_test_map_find(&map, "key1");
        TH_EXPECT(entry);
        th_test_map_erase(&map, entry);
        entry = th_test_map_find(&map, "key1");
        TH_EXPECT(entry == NULL);
        TH_EXPECT(map.size == 3);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_erase_begin)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", 3) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", 4) == TH_ERR_OK);
        th_test_map_entry* entry = th_test_map_find(&map, "key");
        TH_EXPECT(entry);
        th_test_map_erase(&map, entry);
        entry = th_test_map_find(&map, "key");
        TH_EXPECT(entry == NULL);
        TH_EXPECT(map.size == 3);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_erase_end)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", 3) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", 4) == TH_ERR_OK);
        th_test_map_entry* entry = th_test_map_find(&map, "key3");
        TH_EXPECT(entry);
        th_test_map_erase(&map, entry);
        entry = th_test_map_find(&map, "key3");
        TH_EXPECT(entry == NULL);
        TH_EXPECT(map.size == 3);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_erase_and_set)
    {
        th_test_map map = {0};
        th_test_map_init(&map, NULL);
        TH_EXPECT(th_test_map_set(&map, "key", 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key1", 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key2", 3) == TH_ERR_OK);
        TH_EXPECT(th_test_map_set(&map, "key3", 4) == TH_ERR_OK);
        th_test_map_entry* entry = th_test_map_find(&map, "key1");
        TH_EXPECT(entry);
        th_test_map_erase(&map, entry);
        entry = th_test_map_find(&map, "key1");
        TH_EXPECT(entry == NULL);
        TH_EXPECT(map.size == 3);
        TH_EXPECT(th_test_map_set(&map, "key1", 5) == TH_ERR_OK);
        int* value = th_test_map_try_get(&map, "key1");
        TH_EXPECT(value && *value == 5);
        th_test_map_deinit(&map);
    }
    TH_TEST_CASE_END
    TH_TEST_CASE_BEGIN(hashmap_int_set_and_find)
    {
        th_test_map_int map = {0};
        th_test_map_int_init(&map, NULL);
        TH_EXPECT(th_test_map_int_set(&map, 3, 1) == TH_ERR_OK);
        TH_EXPECT(th_test_map_int_set(&map, 5, 2) == TH_ERR_OK);
        TH_EXPECT(th_test_map_int_set(&map, 8, 3) == TH_ERR_OK);
        int* value = th_test_map_int_try_get(&map, 5);
        TH_EXPECT(value && *value == 2);
        th_test_map_int_deinit(&map);
    }
    TH_TEST_CASE_END
}
TH_TEST_END
