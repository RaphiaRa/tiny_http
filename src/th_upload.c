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
    (void)upload;
    (void)dir_label;
    (void)filepath;
    return TH_ERR_NOSUPPORT;
}
