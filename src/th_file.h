#ifndef TH_FSTREAM_H
#define TH_FSTREAM_H

#include <th.h>

#include "th_dir.h"
#include "th_filepath.h"

#include <sys/stat.h>

typedef struct th_open_opt {
    bool read;
    bool write;
    bool create;
    bool truncate;
} th_open_opt;

/** th_file_ops
 * @brief The raw syscalls a th_file performs. Injected at construction time
 * so tests can fake a file fd without touching the filesystem. Each method
 * behaves like the underlying syscall: TH_ERR_OK (with any out-params set)
 * on success, TH_ERR_SYSTEM(errno) on failure.
 */
typedef struct th_file_ops {
    th_err (*openat)(void* self, int dirfd, const char* path, int flags, int* fd);
    th_err (*seek)(void* self, int fd, int whence, size_t* pos);
    th_err (*read)(void* self, int fd, void* addr, size_t len, size_t offset, size_t* read);
    th_err (*write)(void* self, int fd, const void* addr, size_t len, size_t offset, size_t* written);
    th_err (*stat)(void* self, int fd, struct stat* out);
    void (*close)(void* self, int fd);
} th_file_ops;

TH_PRIVATE(th_file_ops*)
th_file_ops_os(void);

typedef struct th_file {
    th_file_ops* ops;
    int fd;
    size_t size;
} th_file;

TH_PRIVATE(void)
th_file_init(th_file* stream, th_file_ops* ops);

TH_PRIVATE(th_err)
th_file_openat(th_file* stream, th_dir* dir, const th_filepath* path, th_open_opt opt);

TH_PRIVATE(th_err)
th_file_read(th_file* stream, void* addr, size_t len, size_t offset, size_t* read) TH_MAYBE_UNUSED;

TH_PRIVATE(th_err)
th_file_write(th_file* stream, const void* addr, size_t len, size_t offset, size_t* written) TH_MAYBE_UNUSED;

TH_PRIVATE(uint32_t)
th_file_stat_hash(th_file* stream);

TH_PRIVATE(void)
th_file_close(th_file* stream);

TH_PRIVATE(void)
th_file_deinit(th_file* stream);

#endif
