#include "th_dir_mgr.h"

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator)
{
    mgr->allocator = allocator ? allocator : th_default_allocator_get();
    th_dir_map_init(&mgr->map, allocator);
    th_heap_string_vec_init(&mgr->heap_strings, allocator);
}

TH_LOCAL(bool)
th_dir_mgr_label_exists(th_dir_mgr* mgr, th_string label)
{
    return th_dir_map_find(&mgr->map, label) != NULL;
}

TH_LOCAL(th_err)
th_dir_mgr_store_string(th_dir_mgr* mgr, th_string str)
{
    th_heap_string heap_str = {0};
    th_heap_string_init(&heap_str, mgr->allocator);
    if (th_heap_string_set(&heap_str, str) != TH_ERR_OK) {
        return TH_ERR_BAD_ALLOC;
    }
    if (th_heap_string_vec_push_back(&mgr->heap_strings, heap_str) != TH_ERR_OK) {
        th_heap_string_deinit(&heap_str);
        return TH_ERR_BAD_ALLOC;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_string)
th_dir_mgr_get_last_string(th_dir_mgr* mgr)
{
    return th_heap_string_view(th_heap_string_vec_end(&mgr->heap_strings) - 1);
}

TH_LOCAL(void)
th_dir_mgr_remove_last_string(th_dir_mgr* mgr)
{
    th_heap_string_deinit(th_heap_string_vec_end(&mgr->heap_strings) - 1);
    th_heap_string_vec_resize(&mgr->heap_strings, th_heap_string_vec_size(&mgr->heap_strings) - 1);
}

TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_string label, th_string path)
{
    th_err err = TH_ERR_OK;
    if (th_dir_mgr_label_exists(mgr, label))
        return TH_ERR_INVALID_ARG;
    th_dir dir = {0};
    th_dir_init(&dir, mgr->allocator);
    if ((err = th_dir_open(&dir, path)) != TH_ERR_OK)
        goto cleanup_dir;
    if ((err = th_dir_mgr_store_string(mgr, label)) != TH_ERR_OK)
        goto cleanup_dir;
    if ((err = th_dir_map_set(&mgr->map, th_dir_mgr_get_last_string(mgr), dir)) != TH_ERR_OK)
        goto cleanup_string;
    return TH_ERR_OK;
cleanup_dir:
    th_dir_deinit(&dir);
cleanup_string:
    th_dir_mgr_remove_last_string(mgr);
    return err;
}

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_string label)
{
    th_dir_map_iter it = th_dir_map_find(&mgr->map, label);
    if (it == NULL)
        return NULL;
    return &it->value;
}

TH_PRIVATE(void)
th_dir_mgr_deinit(th_dir_mgr* mgr)
{
    th_dir_map_iter it = th_dir_map_begin(&mgr->map);
    while (it != NULL) {
        th_dir_deinit(&it->value);
        it = th_dir_map_next(&mgr->map, it);
    }
    th_dir_map_deinit(&mgr->map);
    th_heap_string_vec_deinit(&mgr->heap_strings);
}
