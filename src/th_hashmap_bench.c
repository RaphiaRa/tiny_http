#include "th_bench.h"
#include "th_hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t
th_bench_hash(const char* str)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; str[i] != '\0'; ++i) {
        hash ^= (uint32_t)str[i];
        hash *= 16777619;
    }
    return hash;
}

static bool
th_bench_eq(const char* a, const char* b)
{
    if (a == b)
        return true;
    return a && b && strcmp(a, b) == 0;
}

TH_DEFINE_HASHMAP(th_bench_map, const char*, int, th_bench_hash, th_bench_eq, NULL)

#define TH_BENCH_MAP_NUM_KEYS 10000

TH_BENCH_BEGIN(hashmap)
{
    TH_BENCH_CASE_BEGIN(find_key, 1000000)
    {
        char keys[TH_BENCH_MAP_NUM_KEYS][16];
        th_bench_map map = {0};
        th_bench_map_init(&map, NULL);
        for (size_t i = 0; i < TH_BENCH_MAP_NUM_KEYS; i++) {
            snprintf(keys[i], sizeof(keys[i]), "key%zu", i);
            th_bench_map_set(&map, keys[i], (int)i);
        }

        static size_t indices[TH_BENCH_MAP_NUM_KEYS];
        for (size_t i = 0; i < TH_ARRAY_SIZE(indices); i++)
            indices[i] = (size_t)rand() % TH_BENCH_MAP_NUM_KEYS;

        volatile int sink = 0;
        TH_BENCH_RUN_BEGIN
        {
            const char* key = keys[indices[th_bench_i % TH_ARRAY_SIZE(indices)]];
            sink = *th_bench_map_try_get(&map, key);
        }
        TH_BENCH_RUN_END
        (void)sink;

        th_bench_map_deinit(&map);
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(insert_delete_key, 1000000)
    {
        char keys[TH_BENCH_MAP_NUM_KEYS][16];
        th_bench_map map = {0};
        th_bench_map_init(&map, NULL);
        for (size_t i = 0; i < TH_BENCH_MAP_NUM_KEYS; i++)
            snprintf(keys[i], sizeof(keys[i]), "key%zu", i);

        static bool do_insert[1000000];
        static size_t indices[1000000];
        for (size_t i = 0; i < TH_ARRAY_SIZE(indices); i++) {
            do_insert[i] = rand() % 2 == 0;
            indices[i] = (size_t)rand() % TH_BENCH_MAP_NUM_KEYS;
        }

        TH_BENCH_RUN_BEGIN
        {
            const char* key = keys[indices[th_bench_i]];
            if (do_insert[th_bench_i]) {
                th_bench_map_set(&map, key, (int)indices[th_bench_i]);
            } else {
                th_bench_map_entry* entry = th_bench_map_find(&map, key);
                if (entry)
                    th_bench_map_erase(&map, entry);
            }
        }
        TH_BENCH_RUN_END

        th_bench_map_deinit(&map);
    }
    TH_BENCH_CASE_END

    TH_BENCH_CASE_BEGIN(delete_key, TH_BENCH_MAP_NUM_KEYS)
    {
        char keys[TH_BENCH_MAP_NUM_KEYS][16];
        th_bench_map map = {0};
        th_bench_map_init(&map, NULL);
        for (size_t i = 0; i < TH_BENCH_MAP_NUM_KEYS; i++) {
            snprintf(keys[i], sizeof(keys[i]), "key%zu", i);
            th_bench_map_set(&map, keys[i], (int)i);
        }

        static size_t order[TH_BENCH_MAP_NUM_KEYS];
        for (size_t i = 0; i < TH_ARRAY_SIZE(order); i++)
            order[i] = i;
        for (size_t i = TH_ARRAY_SIZE(order) - 1; i > 0; i--) {
            size_t j = (size_t)rand() % (i + 1);
            size_t tmp = order[i];
            order[i] = order[j];
            order[j] = tmp;
        }

        TH_BENCH_RUN_BEGIN
        {
            th_bench_map_entry* entry = th_bench_map_find(&map, keys[order[th_bench_i]]);
            th_bench_map_erase(&map, entry);
        }
        TH_BENCH_RUN_END

        th_bench_map_deinit(&map);
    }
    TH_BENCH_CASE_END
}
TH_BENCH_END
