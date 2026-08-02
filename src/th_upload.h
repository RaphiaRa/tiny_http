#ifndef TH_UPLOAD_H
#define TH_UPLOAD_H

#include <th.h>

#include "th_config.h"
#include "th_dir_mgr.h"
#include "th_fcache.h"
#include "th_string.h"

struct th_upload {
    th_string name;
    th_string filename;
    th_string content_type;
    th_str data;
    th_dir_mgr* dir_mgr;
    th_fcache* fcache;
};

TH_PRIVATE(void)
th_upload_init(th_upload* upload, th_str buffer, th_dir_mgr* dir_mgr, th_fcache* fcache, th_allocator* allocator);

TH_PRIVATE(void)
th_upload_deinit(th_upload* upload);

TH_PRIVATE(th_err)
th_upload_set_name(th_upload* upload, th_str name);

TH_PRIVATE(th_err)
th_upload_set_filename(th_upload* upload, th_str filename);

TH_PRIVATE(th_err)
th_upload_set_content_type(th_upload* upload, th_str content_type);

#endif
