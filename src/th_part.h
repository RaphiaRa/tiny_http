#ifndef TH_PART_H
#define TH_PART_H

#include <th.h>

#include "th_config.h"
#include "th_dir_mgr.h"
#include "th_file.h"
#include "th_string.h"

struct th_part {
    th_string name;
    th_string filename;
    th_string content_type;
    th_str content;
};

TH_PRIVATE(void)
th_part_init(th_part* part, th_str content, th_allocator* allocator);

TH_PRIVATE(void)
th_part_deinit(th_part* part);

TH_PRIVATE(th_err)
th_part_set_name(th_part* part, th_str name);

TH_PRIVATE(th_err)
th_part_set_filename(th_part* part, th_str filename);

TH_PRIVATE(th_err)
th_part_set_content_type(th_part* part, th_str content_type);

#endif
