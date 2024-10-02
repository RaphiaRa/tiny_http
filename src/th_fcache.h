#ifndef FCACHE_H
#define FCACHE_H

#include <th.h>

#include "th_config.h"
#include "th_dir.h"
#include "th_dir_mgr.h"
#include "th_file.h"
#include "th_hashmap.h"
#include "th_heap_string.h"
#include "th_queue.h"
#include "th_refcounted.h"
#include "th_timer.h"

typedef struct th_fcache th_fcache;
typedef struct th_fcache_entry th_fcache_entry;
struct th_fcache_entry {
    th_refcounted base;
    th_file stream;
    th_heap_string path;
    th_dir* dir;
    th_allocator* allocator;
    th_fcache* cache;
    th_fcache_entry* next;
    th_fcache_entry* prev;
    uint32_t stat_hash;
};

typedef struct th_fcache_id {
    th_string path;
    th_dir* dir;
} th_fcache_id;

TH_INLINE(bool)
th_fcache_id_eq(th_fcache_id a, th_fcache_id b)
{
    return a.dir == b.dir && th_string_eq(a.path, b.path);
}

TH_INLINE(uint32_t)
th_fcache_id_hash(th_fcache_id id)
{
    return th_string_hash(id.path) + id.dir->fd;
}

TH_DEFINE_HASHMAP(th_fcache_map, th_fcache_id, th_fcache_entry*, th_fcache_id_hash, th_fcache_id_eq, (th_fcache_id){0})
TH_DEFINE_LIST(th_fcache_list, th_fcache_entry, prev, next)

struct th_fcache {
    th_allocator* allocator;
    th_dir_mgr dir_mgr;
    th_fcache_map map;
    th_fcache_list list;
    size_t num_cached;
    size_t max_cached;
};

// fcache entry functions

TH_PRIVATE(void)
th_fcache_entry_unref(th_fcache_entry* entry);

// fcache functions

TH_PRIVATE(void)
th_fcache_init(th_fcache* cache, th_allocator* allocator);

TH_PRIVATE(th_err)
th_fcache_get(th_fcache* cache, th_string root, th_string path, th_fcache_entry** out);

TH_PRIVATE(th_err)
th_fcache_add_root(th_fcache* cache, th_string label, th_string path);

TH_PRIVATE(void)
th_fcache_deinit(th_fcache* cache);

#endif
