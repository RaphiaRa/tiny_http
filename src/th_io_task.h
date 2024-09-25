#ifndef TH_IO_TASK_H
#define TH_IO_TASK_H

#include <th.h>

#include "th_allocator.h"
#include "th_file.h"
#include "th_iov.h"
#include "th_task.h"

/** th_io_handler
 *@brief I/O operation completion handler, inherits from th_task.
 * and contains the result of the operation.
 */
typedef struct th_io_handler {
    th_task base;
    void (*fn)(void* self, size_t result, th_err err);
    size_t result;
    th_err err;
} th_io_handler;

TH_PRIVATE(void)
th_io_handler_fn(void* self);

TH_INLINE(void)
th_io_handler_init(th_io_handler* handler, void (*fn)(void* self, size_t result, th_err err), void (*destroy)(void* self))
{
    th_task_init(&handler->base, th_io_handler_fn, destroy);
    handler->fn = fn;
}

TH_INLINE(void)
th_io_handler_set_result(th_io_handler* handler, size_t result, th_err err)
{
    handler->result = result;
    handler->err = err;
}

TH_INLINE(void)
th_io_handler_complete(th_io_handler* handler, size_t result, th_err err)
{
    th_io_handler_set_result(handler, result, err);
    th_task_complete(&handler->base);
}

TH_INLINE(void)
th_io_handler_destroy(th_io_handler* handler)
{
    th_task_destroy(&handler->base);
}

// some aliases

typedef th_io_handler th_write_handler;
typedef th_io_handler th_read_handler;
#define th_write_handler_init th_io_handler_init
#define th_read_handler_init th_io_handler_init
#define th_write_handler_complete th_io_handler_complete
#define th_read_handler_complete th_io_handler_complete

typedef enum th_io_open_flag {
    TH_IO_OPEN_FLAG_RDONLY = 1 << 0,
    TH_IO_OPEN_FLAG_DIR = 1 << 1,
} th_io_open_flag;

/** th_io_op
 *@brief I/O operation type.
 */
typedef enum th_io_op_type {
    TH_IO_OP_TYPE_NONE = 0,
    TH_IO_OP_TYPE_READ = 1,
    TH_IO_OP_TYPE_WRITE = 2,
    TH_IO_OP_TYPE_MAX = TH_IO_OP_TYPE_WRITE
} th_io_op_type;
#define TH_IO_OP(opc, type) ((opc) | ((type) << 8))
#define TH_IO_OP_TYPE(op) ((op) >> 8)
typedef enum th_io_op {
    TH_IO_OP_ACCEPT = TH_IO_OP(0, TH_IO_OP_TYPE_READ),
    TH_IO_OP_READ = TH_IO_OP(1, TH_IO_OP_TYPE_READ),
    TH_IO_OP_WRITE = TH_IO_OP(2, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_WRITEV = TH_IO_OP(3, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_SEND = TH_IO_OP(4, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_SENDV = TH_IO_OP(5, TH_IO_OP_TYPE_WRITE),
    TH_IO_OP_READV = TH_IO_OP(6, TH_IO_OP_TYPE_READ),
    TH_IO_OP_OPENAT = TH_IO_OP(7, TH_IO_OP_TYPE_NONE),
    TH_IO_OP_OPEN = TH_IO_OP(8, TH_IO_OP_TYPE_NONE),
    TH_IO_OP_CLOSE = TH_IO_OP(9, TH_IO_OP_TYPE_NONE),
    TH_IO_OP_SENDFILE = TH_IO_OP(10, TH_IO_OP_TYPE_WRITE),
} th_io_op;

/** th_io_task
 *@brief I/O task, inherits from th_task.
 * Contains the I/O operation type and the I/O operation arguments.
 */
typedef struct th_io_task {
    th_task base;
    th_allocator* allocator;
    th_err (*fn)(void* self, size_t* result);
    th_io_handler* on_complete;
    void* addr;
    void* addr2;
    size_t len;
    size_t len2;
    size_t offset;
    unsigned int flags;
    int fd;
    enum th_io_op op;
} th_io_task;

TH_PRIVATE(th_io_task*)
th_io_task_create(th_allocator* allocator);

/*
TH_PRIVATE(void)
th_io_task_to_string(char* buf, size_t len, th_io_task* iot);
*/

TH_PRIVATE(void)
th_io_task_prepare_read(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete);

/*
TH_PRIVATE(void)
th_io_task_prepare_write(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_writev(th_io_task* iot, int fd, th_iov* iov, size_t len, th_io_handler* on_complete);
*/

TH_PRIVATE(void)
th_io_task_prepare_send(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_sendv(th_io_task* iot, int fd, th_iov* iov, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_readv(th_io_task* iot, int fd, th_iov* iov, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_sendfile(th_io_task* iot, th_file* file, int sfd, th_iov* header, size_t iovcnt,
                            size_t offset, size_t len, th_io_handler* on_complete);

TH_PRIVATE(void)
th_io_task_prepare_accept(th_io_task* iot, int fd, void* addr, void* addrlen, th_io_handler* on_complete);

/** th_io_task_execute
 * @brief Executes the I/O task and leaves the completion handler untouched.
 * @param iot I/O task.
 * @param result Result of the I/O operation.
 * @return Error code.
 */
TH_PRIVATE(th_err)
th_io_task_execute(th_io_task* iot, size_t* result);

/** th_io_task_try_execute
 * @brief Tries to execute the I/O task and returns the completion handler
 * if the I/O operation was completed.
 * @param iot I/O task.
 * @return Completion handler.
 */
TH_PRIVATE(th_io_handler*)
th_io_task_try_execute(th_io_task* iot);

TH_PRIVATE(void)
th_io_task_destroy(th_io_task* iot);

/** th_io_task_abort
 * @brief Aborts the I/O task. Sets the error code and returns the completion handler.
 * @param iot I/O task.
 * @param err Error code.
 */
TH_PRIVATE(th_io_handler*)
th_io_task_abort(th_io_task* iot, th_err err);

#endif
