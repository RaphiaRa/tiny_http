#ifndef TH_FSTREAM_H
#define TH_FSTREAM_H

#include <th.h>

#include "th_dir.h"

typedef struct th_file_mmap {
    void* addr;
    size_t offset;
    size_t len;
} th_file_mmap;

typedef struct th_file {
    int fd;
    size_t size;
    th_file_mmap view;
} th_file;

TH_PRIVATE(void)
th_file_init(th_file* stream);

typedef struct th_open_opt {
    bool read;
    bool write;
    bool create;
} th_open_opt;

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, th_string path, th_open_opt opt);

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read) TH_MAYBE_UNUSED;

typedef struct th_fileview {
    void* ptr;
    size_t len;
} th_fileview;

TH_PRIVATE(th_err)
th_file_get_view(th_file* stream, th_fileview* view, size_t offset, size_t len);

TH_PRIVATE(void)
th_file_close(th_file* stream);

TH_PRIVATE(void)
th_file_deinit(th_file* stream);

#endif
