#include "th_fcache.h"
#include "th_log.h"

#undef TH_LOG_TAG
#define TH_LOG_TAG "fcache"

TH_LOCAL(th_fcache_id)
th_fcache_entry_id(th_fcache_entry* entry)
{
    return (th_fcache_id){th_string_view(&entry->path), entry->dir};
}

TH_LOCAL(void)
th_fcache_entry_actual_destroy(void* self)
{
    th_fcache_entry* entry = self;
    // Remove entry from cache
    th_fcache_map_iter it = th_fcache_map_find(&entry->cache->map, th_fcache_entry_id(entry));
    if (it != NULL) {
        th_fcache_map_erase(&entry->cache->map, it);
    }
    th_file_deinit(&entry->stream);
    th_string_deinit(&entry->path);
    th_allocator_free(entry->allocator, entry);
}

TH_LOCAL(void)
th_fcache_entry_init(th_fcache_entry* entry, th_fcache* cache, th_allocator* allocator)
{
    entry->allocator = allocator ? allocator : th_default_allocator_get();
    th_refcounted_init(&entry->base, th_fcache_entry_actual_destroy);
    th_file_init(&entry->stream, cache->file_ops);
    th_string_init(&entry->path, entry->allocator);
    entry->cache = cache;
    entry->next = NULL;
    entry->prev = NULL;
}

TH_LOCAL(th_err)
th_fcache_entry_open(th_fcache_entry* entry, th_dir* dir, th_str path)
{
    th_err err = TH_ERR_OK;
    th_filepath filepath;
    th_open_opt opt = {.read = true};
    if ((err = th_filepath_init(&filepath, path)) != TH_ERR_OK) {
        TH_LOG_INFO("Invalid file path %.*s: %s", (int)path.len, path.ptr, th_strerror(err));
        goto cleanup;
    }
    if ((err = th_file_openat(&entry->stream, dir, &filepath, opt)) != TH_ERR_OK) {
        TH_LOG_INFO("Failed to open file at %.*s: %s", (int)path.len, path.ptr, th_strerror(err));
        goto cleanup;
    }
    if ((err = th_string_set(&entry->path, path)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to set path: %s", th_strerror(err));
        goto cleanup_fstream;
    }
    entry->stat_hash = th_file_stat_hash(&entry->stream);
    entry->dir = dir;
    return TH_ERR_OK;
cleanup_fstream:
    th_file_deinit(&entry->stream);
cleanup:
    return err;
}

TH_LOCAL(th_fcache_entry*)
th_fcache_entry_ref(th_fcache_entry* entry)
{
    th_refcounted_ref(&entry->base);
    return entry;
}

TH_PRIVATE(void)
th_fcache_entry_unref(th_fcache_entry* entry)
{
    th_refcounted_unref(&entry->base);
}

TH_PRIVATE(void)
th_fcache_init(th_fcache* cache, th_file_ops* file_ops, th_allocator* allocator)
{
    cache->allocator = allocator ? allocator : th_default_allocator_get();
    cache->file_ops = file_ops;
    th_fcache_map_init(&cache->map, cache->allocator);
    cache->list = (th_fcache_list){NULL, NULL};
    cache->num_cached = 0;
    cache->max_cached = TH_CONFIG_MAX_CACHED_FDS;
}

TH_LOCAL(void)
th_fcache_erase(th_fcache* cache, th_fcache_entry* entry)
{
    th_fcache_list_erase(&cache->list, entry);
    th_fcache_entry_unref(entry);
    --cache->num_cached;
}

TH_LOCAL(th_fcache_entry*)
th_fcache_try_get(th_fcache* cache, th_dir* dir, th_str path)
{
    th_fcache_entry** v = th_fcache_map_try_get(&cache->map, (th_fcache_id){path, dir});
    if (!v)
        return NULL;
    th_fcache_entry* entry = *v;
    // Check if the file has been modified
    uint32_t hash = th_file_stat_hash(&entry->stream);
    if (hash != entry->stat_hash) {
        TH_LOG_TRACE("File has been modified, don't use cached entry");
        th_fcache_erase(cache, entry);
        return NULL;
    }
    // Move entry to the back of the list
    th_fcache_list_erase(&cache->list, entry);
    th_fcache_list_push_back(&cache->list, entry);
    return th_fcache_entry_ref(entry);
}

TH_LOCAL(th_err)
th_fcache_insert(th_fcache* cache, th_fcache_entry* entry)
{
    if (cache->num_cached == cache->max_cached) {
        // Evict the first entry
        th_fcache_entry* first = th_fcache_list_front(&cache->list);
        th_fcache_erase(cache, first);
    }
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_map_set(&cache->map, th_fcache_entry_id(entry), entry)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to insert entry into map: %s", th_strerror(err));
        return err;
    }
    th_fcache_list_push_back(&cache->list, th_fcache_entry_ref(entry));
    cache->num_cached++;
    return TH_ERR_OK;
}

TH_PRIVATE(th_err)
th_fcache_get(th_fcache* cache, th_dir* dir, th_str path, th_fcache_entry** out)
{
    th_fcache_entry* entry = th_fcache_try_get(cache, dir, path);
    if (entry) {
        *out = entry;
        return TH_ERR_OK;
    }
    entry = th_allocator_alloc(cache->allocator, sizeof(th_fcache_entry));
    if (!entry)
        return TH_ERR_BAD_ALLOC;
    th_fcache_entry_init(entry, cache, cache->allocator);
    th_err err = TH_ERR_OK;
    if ((err = th_fcache_entry_open(entry, dir, path)) != TH_ERR_OK) {
        th_allocator_free(cache->allocator, entry);
        return err;
    }
    if ((err = th_fcache_insert(cache, entry)) != TH_ERR_OK) {
        TH_LOG_ERROR("Failed to insert fcache entry");
        th_fcache_entry_unref(entry);
        return err;
    }
    *out = entry;
    return TH_ERR_OK;
}

TH_PRIVATE(void)
th_fcache_deinit(th_fcache* cache)
{
    th_fcache_entry* entry = NULL;
    while ((entry = th_fcache_list_pop_front(&cache->list))) {
        th_fcache_entry_unref(entry);
    }
    th_fcache_map_deinit(&cache->map);
}
