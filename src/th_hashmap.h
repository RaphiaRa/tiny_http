#ifndef TH_HASHMAP_H
#define TH_HASHMAP_H

#include <th.h>

#include "th_allocator.h"
#include "th_hash.h"
#include "th_utility.h"

#include <string.h>

#define TH_DEFINE_HASHMAP2(NAME, K, V, HASH, K_EQ, K_NULL, K_DEINIT, V_DEINIT)                                                      \
    typedef struct NAME##_entry {                                                                                                   \
        K key;                                                                                                                      \
        V value;                                                                                                                    \
    } NAME##_entry;                                                                                                                 \
                                                                                                                                    \
    typedef struct NAME {                                                                                                           \
        NAME##_entry* entries;                                                                                                      \
        size_t size;                                                                                                                \
        size_t capacity;                                                                                                            \
        size_t end;                                                                                                                 \
        size_t begin;                                                                                                               \
        th_allocator* allocator;                                                                                                    \
    } NAME;                                                                                                                         \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_init(NAME* map, th_allocator* allocator) TH_MAYBE_UNUSED;                                                                \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_reset(NAME* map) TH_MAYBE_UNUSED;                                                                                        \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_reserve(NAME* map, size_t capacity) TH_MAYBE_UNUSED;                                                                     \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_deinit(NAME* map) TH_MAYBE_UNUSED;                                                                                       \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_set(NAME* map, K key, V value) TH_MAYBE_UNUSED;                                                                          \
                                                                                                                                    \
    TH_INLINE(V*)                                                                                                                   \
    NAME##_try_get(const NAME* map, K key) TH_MAYBE_UNUSED;                                                                         \
                                                                                                                                    \
    typedef NAME##_entry* NAME##_iter;                                                                                              \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_find(const NAME* map, K key) TH_MAYBE_UNUSED;                                                                            \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_erase(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                   \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_begin(NAME* map) TH_MAYBE_UNUSED;                                                                                        \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_next(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                    \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_prev(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                    \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_init(NAME* map, th_allocator* allocator)                                                                                 \
    {                                                                                                                               \
        map->allocator = allocator;                                                                                                 \
        if (map->allocator == NULL) {                                                                                               \
            map->allocator = th_default_allocator_get();                                                                            \
        }                                                                                                                           \
        map->entries = NULL;                                                                                                        \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_deinit(NAME* map)                                                                                                        \
    {                                                                                                                               \
        NAME##_reset(map);                                                                                                          \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_reset(NAME* map)                                                                                                         \
    {                                                                                                                               \
        if (map->entries) {                                                                                                         \
            if (map->size > 0) {                                                                                                    \
                for (size_t i = map->begin; i < map->end; i++) {                                                                    \
                    NAME##_entry* entry = &map->entries[i];                                                                         \
                    if (!K_EQ(entry->key, K_NULL)) {                                                                                \
                        K_DEINIT(entry->key);                                                                                       \
                        V_DEINIT(entry->value);                                                                                     \
                    }                                                                                                               \
                }                                                                                                                   \
            }                                                                                                                       \
            th_allocator_free(map->allocator, map->entries);                                                                        \
        }                                                                                                                           \
        map->entries = NULL;                                                                                                        \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_reserve(NAME* map, size_t capacity)                                                                                      \
    {                                                                                                                               \
        if (map->capacity >= capacity) {                                                                                            \
            return TH_ERR_OK;                                                                                                       \
        }                                                                                                                           \
        capacity = th_next_pow2(capacity);                                                                                          \
        NAME##_entry* entries = (NAME##_entry*)th_allocator_realloc(map->allocator, map->entries, capacity * sizeof(NAME##_entry)); \
        if (entries == NULL) {                                                                                                      \
            return TH_ERR_BAD_ALLOC;                                                                                                \
        }                                                                                                                           \
        for (size_t i = map->capacity; i < capacity; i++) {                                                                         \
            entries[i] = (NAME##_entry){.key = K_NULL};                                                                             \
        }                                                                                                                           \
        map->entries = entries;                                                                                                     \
        map->capacity = capacity;                                                                                                   \
        return TH_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_LOCAL(void)                                                                                                                  \
    NAME##_update_begin_end(NAME* map, size_t new_index)                                                                            \
    {                                                                                                                               \
        if (map->size == 1) {                                                                                                       \
            map->begin = new_index;                                                                                                 \
            map->end = new_index + 1;                                                                                               \
        } else {                                                                                                                    \
            if (new_index < map->begin) {                                                                                           \
                map->begin = new_index;                                                                                             \
            }                                                                                                                       \
            if (new_index + 1 > map->end) {                                                                                         \
                map->end = new_index + 1;                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_LOCAL(th_err)                                                                                                                \
    NAME##_do_set(NAME* map, uint32_t hash, K key, V value)                                                                         \
    {                                                                                                                               \
        for (size_t i = hash; i < map->capacity; i++) {                                                                             \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                map->size++;                                                                                                        \
                NAME##_update_begin_end(map, i);                                                                                    \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                K_DEINIT(entry->key);                                                                                               \
                V_DEINIT(entry->value);                                                                                             \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
        for (size_t i = 0; i < hash; i++) {                                                                                         \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                map->size++;                                                                                                        \
                NAME##_update_begin_end(map, i);                                                                                    \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                K_DEINIT(entry->key);                                                                                               \
                V_DEINIT(entry->value);                                                                                             \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                return TH_ERR_OK;                                                                                                   \
            }                                                                                                                       \
        }                                                                                                                           \
        return TH_ERR_BAD_ALLOC;                                                                                                    \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_fix_hole(NAME* map, NAME##_entry* entry) TH_MAYBE_UNUSED;                                                                \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_fix_hole(NAME* map, NAME##_entry* entry)                                                                                 \
    {                                                                                                                               \
        size_t last_zeroed = entry - map->entries;                                                                                  \
        for (size_t i = entry - map->entries + 1; i < map->end; i++) {                                                              \
            uint32_t hash = 0;                                                                                                      \
            if (K_EQ(map->entries[i].key, K_NULL)) {                                                                                \
                break;                                                                                                              \
            } else if ((hash = (HASH(map->entries[i].key) & (map->capacity - 1))) <= last_zeroed) {                                 \
                map->entries[last_zeroed] = map->entries[i];                                                                        \
                map->entries[i].key = K_NULL;                                                                                       \
                last_zeroed = i;                                                                                                    \
            }                                                                                                                       \
        }                                                                                                                           \
        if (map->size == 0) {                                                                                                       \
            map->begin = 0;                                                                                                         \
            map->end = 0;                                                                                                           \
        } else if (last_zeroed == map->end - 1) {                                                                                   \
            map->end = NAME##_prev(map, &map->entries[last_zeroed]) - map->entries + 1;                                             \
        } else if (last_zeroed == map->begin) {                                                                                     \
            map->begin = NAME##_next(map, &map->entries[last_zeroed]) - map->entries;                                               \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_expand(NAME* map)                                                                                                        \
    {                                                                                                                               \
        th_err err = TH_ERR_OK;                                                                                                     \
        size_t old_capacity = map->capacity;                                                                                        \
        size_t new_capacity = old_capacity * 2;                                                                                     \
        if (new_capacity == 0) {                                                                                                    \
            new_capacity = 1;                                                                                                       \
        }                                                                                                                           \
        if ((err = NAME##_reserve(map, new_capacity)) != TH_ERR_OK) {                                                               \
            return err;                                                                                                             \
        }                                                                                                                           \
        /* Need to rehash all entries */                                                                                            \
        for (size_t i = 0; i < old_capacity; i++) {                                                                                 \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                /* rearranged == 0; */                                                                                              \
                continue;                                                                                                           \
            }                                                                                                                       \
            uint32_t hash = HASH(entry->key);                                                                                       \
            /* Don't need to rehash every entry */                                                                                  \
            hash &= (new_capacity - 1);                                                                                             \
            NAME##_entry e = *entry;                                                                                                \
            entry->key = K_NULL;                                                                                                    \
            --map->size;                                                                                                            \
            NAME##_fix_hole(map, entry);                                                                                            \
            if ((err = NAME##_do_set(map, hash, e.key, e.value)) != TH_ERR_OK) {                                                    \
                return err;                                                                                                         \
            }                                                                                                                       \
        }                                                                                                                           \
        return TH_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(th_err)                                                                                                               \
    NAME##_set(NAME* map, K key, V value)                                                                                           \
    {                                                                                                                               \
        if (map->size >= map->capacity / 2) {                                                                                       \
            th_err err = NAME##_expand(map);                                                                                        \
            if (err != TH_ERR_OK) {                                                                                                 \
                return err;                                                                                                         \
            }                                                                                                                       \
        }                                                                                                                           \
        uint32_t hash = HASH(key) & (map->capacity - 1);                                                                            \
        return NAME##_do_set(map, hash, key, value);                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_find(const NAME* map, K key)                                                                                             \
    {                                                                                                                               \
        uint32_t hash = HASH(key) & (map->capacity - 1);                                                                            \
        if (map->size == 0) {                                                                                                       \
            return NULL;                                                                                                            \
        }                                                                                                                           \
        for (size_t i = hash; i < map->end; i++) {                                                                                  \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                return NULL;                                                                                                        \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                return entry;                                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        for (size_t i = map->begin; i < hash; i++) {                                                                                \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                return NULL;                                                                                                        \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                return entry;                                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(void)                                                                                                                 \
    NAME##_erase(NAME* map, NAME##_entry* entry)                                                                                    \
    {                                                                                                                               \
        entry->key = K_NULL;                                                                                                        \
        map->size--;                                                                                                                \
        NAME##_fix_hole(map, entry);                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(V*)                                                                                                                   \
    NAME##_try_get(const NAME* map, K key)                                                                                          \
    {                                                                                                                               \
        NAME##_entry* entry = NAME##_find(map, key);                                                                                \
        if (entry) {                                                                                                                \
            return &entry->value;                                                                                                   \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_begin(NAME* map)                                                                                                         \
    {                                                                                                                               \
        if (map->begin == map->end)                                                                                                 \
            return NULL;                                                                                                            \
        return &map->entries[map->begin];                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_next(NAME* map, NAME##_entry* entry)                                                                                     \
    {                                                                                                                               \
        size_t i = entry - map->entries;                                                                                            \
        for (size_t j = i + 1; j < map->end; j++) {                                                                                 \
            NAME##_entry* e = &map->entries[j];                                                                                     \
            if (!K_EQ(e->key, K_NULL)) {                                                                                            \
                return e;                                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    TH_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_prev(NAME* map, NAME##_entry* entry)                                                                                     \
    {                                                                                                                               \
        size_t i = entry - map->entries;                                                                                            \
        for (size_t j = i - 1; j >= map->begin; j--) {                                                                              \
            NAME##_entry* e = &map->entries[j];                                                                                     \
            if (!K_EQ(e->key, K_NULL)) {                                                                                            \
                return e;                                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
        return NAME##_begin(map);                                                                                                   \
    }

/** TH_DEFINE_HASHMAP_FIND
 * Define find functions for alternative key types.
 * !!! Only makes sense if the HASH function for the alternative key type
 * is the same as the HASH function for the primary key type.
 */
#define TH_DEFINE_HASHMAP_FIND(NAME, METHOD, K, K_HASH, K_EQ, K_NULL) \
    TH_INLINE(NAME##_entry*)                                          \
    NAME##_##METHOD(const NAME* map, K key)                           \
    {                                                                 \
        uint32_t hash = K_HASH(key) & (map->capacity - 1);            \
        if (map->size == 0) {                                         \
            return NULL;                                              \
        }                                                             \
        for (size_t i = hash; i < map->end; i++) {                    \
            NAME##_entry* entry = &map->entries[i];                   \
            if (K_EQ(entry->key, K_NULL)) {                           \
                return NULL;                                          \
            }                                                         \
            if (K_EQ(entry->key, key)) {                              \
                return entry;                                         \
            }                                                         \
        }                                                             \
        for (size_t i = map->begin; i < hash; i++) {                  \
            NAME##_entry* entry = &map->entries[i];                   \
            if (K_EQ(entry->key, K_NULL)) {                           \
                return NULL;                                          \
            }                                                         \
            if (K_EQ(entry->key, key)) {                              \
                return entry;                                         \
            }                                                         \
        }                                                             \
        return NULL;                                                  \
    }

#define TH_DEFINE_HASHMAP(NAME, K, V, HASH, K_EQ, K_NULL) TH_DEFINE_HASHMAP2(NAME, K, V, HASH, K_EQ, K_NULL, (void), (void))
/* default hash maps begin */
/* th_cstr_map begin */

TH_INLINE(uint32_t)
th_cstr_hash(const char* str)
{
    return th_hash_cstr(str);
}

TH_INLINE(bool)
th_cstr_eq(const char* a, const char* b)
{
    if (!a || !b)
        return a == b;
    return *a == *b && (strcmp(a, b) == 0);
}

TH_DEFINE_HASHMAP(th_cstr_map, const char*, const char*, th_cstr_hash, th_cstr_eq, NULL)

/* th_cstr_map end */

#endif /* TH_HASHMAP_H */
