#ifndef TH_DIR_MGR_H
#define TH_DIR_MGR_H

#include <th.h>

#include "th_allocator.h"
#include "th_dir.h"
#include "th_hashmap.h"
#include "th_heap_string.h"
#include "th_string.h"

TH_DEFINE_HASHMAP(th_dir_map, th_string, th_dir, th_string_hash, th_string_eq, (th_string){0})

typedef struct th_dir_mgr {
    th_allocator* allocator;
    th_dir_map map;
    th_heap_string_vec heap_strings;
} th_dir_mgr;

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator);

TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_string label, th_string path);

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_string label);

TH_PRIVATE(void)
th_dir_mgr_deinit(th_dir_mgr* mgr);

#endif
