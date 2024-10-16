#ifndef TH_UPLOAD_H
#define TH_UPLOAD_H

#include <th.h>

#include "th_config.h"
#include "th_fcache.h"
#include "th_heap_string.h"

struct th_upload {
    th_heap_string name;
    th_heap_string filename;
    th_heap_string content_type;
    th_string data;
    th_fcache* fcache;
};

TH_PRIVATE(void)
th_upload_init(th_upload* upload, th_string buffer, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_upload_deinit(th_upload* upload);

TH_PRIVATE(th_err)
th_upload_set_name(th_upload* upload, th_string name);

TH_PRIVATE(th_err)
th_upload_set_filename(th_upload* upload, th_string filename);

TH_PRIVATE(th_err)
th_upload_set_content_type(th_upload* upload, th_string content_type);

#endif
