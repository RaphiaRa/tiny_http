#include "th_upload.h"

TH_PRIVATE(void)
th_upload_init(th_upload* upload, th_string buffer, th_fcache* fcache, th_allocator* allocator)
{
    th_heap_string_init(&upload->name, allocator);
    th_heap_string_init(&upload->filename, allocator);
    th_heap_string_init(&upload->content_type, allocator);
    upload->data = buffer;
    upload->fcache = fcache;
}

TH_PRIVATE(void)
th_upload_deinit(th_upload* upload)
{
    th_heap_string_deinit(&upload->name);
    th_heap_string_deinit(&upload->filename);
    th_heap_string_deinit(&upload->content_type);
}

TH_PRIVATE(th_err)
th_upload_set_name(th_upload* upload, th_string name)
{
    return th_heap_string_set(&upload->name, name);
}

TH_PRIVATE(th_err)
th_upload_set_filename(th_upload* upload, th_string filename)
{
    return th_heap_string_set(&upload->filename, filename);
}

TH_PRIVATE(th_err)
th_upload_set_content_type(th_upload* upload, th_string content_type)
{
    return th_heap_string_set(&upload->content_type, content_type);
}

// Public API

TH_PUBLIC(th_upload_info)
th_upload_get_info(const th_upload* upload)
{
    return (th_upload_info){
        .name = th_heap_string_data(&upload->name),
        .filename = th_heap_string_data(&upload->filename),
        .content_type = th_heap_string_data(&upload->content_type),
        .size = upload->data.len,
    };
}

TH_PUBLIC(th_buffer)
th_upload_get_data(const th_upload* upload)
{
    return (th_buffer){upload->data.ptr, upload->data.len};
}

TH_PUBLIC(th_err)
th_upload_save(const th_upload* upload, const char* dir_label, const char* filepath)
{
    th_dir* dir = th_fcache_find_dir(upload->fcache, th_string_from_cstr(dir_label));
    if (!dir)
        return TH_ERR_HTTP(TH_CODE_NOT_FOUND);
    th_err err = TH_ERR_OK;
    th_open_opt opt = {.create = true, .write = true, .truncate = true};
    th_file file;
    if ((err = th_file_openat(&file, dir, th_string_from_cstr(filepath), opt)) != TH_ERR_OK)
        return err;
    size_t total_written = 0;
    while (total_written < upload->data.len) {
        size_t written = 0;
        if ((err = th_file_write(&file, upload->data.ptr + total_written, upload->data.len - total_written, total_written, &written)) != TH_ERR_OK) {
            th_file_close(&file);
            return err;
        }
        total_written += written;
    }
    th_file_close(&file);
    return TH_ERR_OK;
}
