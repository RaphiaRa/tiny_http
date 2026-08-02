#include "th_dir_mgr.h"

TH_PRIVATE(void)
th_dir_mgr_init(th_dir_mgr* mgr, th_allocator* allocator)
{
    mgr->allocator = allocator ? allocator : th_default_allocator_get();
    th_dir_map_init(&mgr->map, allocator);
    th_string_vec_init(&mgr->strings, allocator);
}

TH_LOCAL(bool)
th_dir_mgr_label_exists(th_dir_mgr* mgr, th_str label)
{
    return th_dir_map_find(&mgr->map, label) != NULL;
}

TH_LOCAL(th_err)
th_dir_mgr_store_string(th_dir_mgr* mgr, th_str str)
{
    th_string owned = {0};
    th_string_init(&owned, mgr->allocator);
    if (th_string_set(&owned, str) != TH_ERR_OK) {
        return TH_ERR_BAD_ALLOC;
    }
    if (th_string_vec_push_back(&mgr->strings, owned) != TH_ERR_OK) {
        th_string_deinit(&owned);
        return TH_ERR_BAD_ALLOC;
    }
    return TH_ERR_OK;
}

TH_LOCAL(th_str)
th_dir_mgr_get_last_string(th_dir_mgr* mgr)
{
    return th_string_view(th_string_vec_end(&mgr->strings) - 1);
}

TH_LOCAL(void)
th_dir_mgr_remove_last_string(th_dir_mgr* mgr)
{
    th_string_deinit(th_string_vec_end(&mgr->strings) - 1);
    th_string_vec_resize(&mgr->strings, th_string_vec_size(&mgr->strings) - 1);
}

TH_PRIVATE(th_err)
th_dir_mgr_add(th_dir_mgr* mgr, th_str label, th_dir dir)
{
    th_err err = TH_ERR_OK;
    if (th_dir_mgr_label_exists(mgr, label)) {
        th_dir_deinit(&dir);
        return TH_ERR_INVALID_ARG;
    }
    if ((err = th_dir_mgr_store_string(mgr, label)) != TH_ERR_OK) {
        th_dir_deinit(&dir);
        return err;
    }
    if ((err = th_dir_map_set(&mgr->map, th_dir_mgr_get_last_string(mgr), dir)) != TH_ERR_OK) {
        th_dir_mgr_remove_last_string(mgr);
        th_dir_deinit(&dir);
        return err;
    }
    return TH_ERR_OK;
}

TH_PRIVATE(th_dir*)
th_dir_mgr_get(th_dir_mgr* mgr, th_str label)
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
    th_string_vec_deinit(&mgr->strings);
}
