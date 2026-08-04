#include "th_part.h"

TH_PRIVATE(void)
th_part_init(th_part* part, th_str content, th_allocator* allocator)
{
    th_string_init(&part->name, allocator);
    th_string_init(&part->filename, allocator);
    th_string_init(&part->content_type, allocator);
    part->content = content;
}

TH_PRIVATE(void)
th_part_deinit(th_part* part)
{
    th_string_deinit(&part->name);
    th_string_deinit(&part->filename);
    th_string_deinit(&part->content_type);
}

TH_PRIVATE(th_err)
th_part_set_name(th_part* part, th_str name)
{
    return th_string_set(&part->name, name);
}

TH_PRIVATE(th_err)
th_part_set_filename(th_part* part, th_str filename)
{
    return th_string_set(&part->filename, filename);
}

TH_PRIVATE(th_err)
th_part_set_content_type(th_part* part, th_str content_type)
{
    return th_string_set(&part->content_type, content_type);
}

// Public API

TH_PUBLIC(const char*)
th_part_name(const th_part* part)
{
    return th_string_data(&part->name);
}

TH_PUBLIC(const char*)
th_part_filename(const th_part* part)
{
    return th_string_data(&part->filename);
}

TH_PUBLIC(const char*)
th_part_content_type(const th_part* part)
{
    return th_string_data(&part->content_type);
}

TH_PUBLIC(th_buffer)
th_part_content(const th_part* part)
{
    return (th_buffer){part->content.ptr, part->content.len};
}
