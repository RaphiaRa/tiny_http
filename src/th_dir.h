#ifndef TH_DIR_H
#define TH_DIR_H

#include <th.h>

#include "th_config.h"
#include "th_str.h"
#include "th_string.h"

/** th_dir_ops
 * @brief The raw open/close syscalls a th_dir performs. Injected at
 * construction time so tests can fake a directory fd without touching the
 * filesystem. open behaves like the underlying syscall: TH_ERR_OK with
 * *fd set on success, TH_ERR_SYSTEM(errno) on failure.
 */
typedef struct th_dir_ops {
    th_err (*open)(void* self, const char* path, int* fd);
    void (*close)(void* self, int fd);
} th_dir_ops;

TH_PRIVATE(th_dir_ops*)
th_dir_ops_os(void);

typedef struct th_dir {
    th_allocator* allocator;
    th_dir_ops* ops;
    th_string path;
    int fd;
} th_dir;

TH_PRIVATE(void)
th_dir_init(th_dir* dir, th_dir_ops* ops, th_allocator* allocator);

TH_PRIVATE(th_err)
th_dir_open(th_dir* dir, th_str path);

TH_PRIVATE(th_str)
th_dir_get_path(th_dir* dir);

TH_PRIVATE(void)
th_dir_deinit(th_dir* dir);

#endif
