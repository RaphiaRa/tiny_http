#ifndef TH_DIR_H
#define TH_DIR_H

#include <th.h>

#include "th_config.h"
#include "th_str.h"
#include "th_string.h"

typedef struct th_dir {
    th_allocator* allocator;
    th_string path;
    int fd;
} th_dir;

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_allocator* allocator);

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_str path);

TH_PRIVATE(th_str)
th_dir_get_path(th_dir* dir);

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir);

#endif
