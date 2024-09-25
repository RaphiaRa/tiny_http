#include "th_io_task.h"
#include "th_io_op.h"
#include "th_system_error.h"
#include "th_utility.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

TH_PRIVATE(void)
th_io_handler_fn(void* self)
{
    th_io_handler* handler = self;
    handler->fn(self, handler->result, handler->err);
}

TH_LOCAL(void)
th_io_task_destroy_impl(void* self)
{
    th_io_task* iot = self;
    if (iot->on_complete) {
        th_io_handler_destroy(iot->on_complete);
        iot->on_complete = NULL;
    }
    th_allocator_free(iot->allocator, iot);
}

TH_LOCAL(void)
th_io_task_fn(void* self)
{
    th_io_task* iot = self;
    size_t result = 0;
    th_err err = th_io_task_execute(iot, &result);
    if (iot->on_complete) {
        th_io_handler_complete(iot->on_complete, result, err);
    }
}

TH_PRIVATE(th_io_task*)
th_io_task_create(th_allocator* allocator)
{
    th_io_task* iot = th_allocator_alloc(allocator, sizeof(th_io_task));
    if (!iot)
        return NULL;
    th_task_init(&iot->base, th_io_task_fn, th_io_task_destroy_impl);
    iot->allocator = allocator;
    iot->on_complete = NULL;
    return iot;
}

/*
TH_PRIVATE(void)
th_io_task_to_string(char* buf, size_t len, th_io_task* iot)
{
    const char* op_str = NULL;
    switch (iot->op) {
    case TH_IO_OP_OPEN:
        op_str = "OPEN";
        break;
    case TH_IO_OP_OPENAT:
        op_str = "OPENAT";
        break;
    case TH_IO_OP_CLOSE:
        op_str = "CLOSE";
        break;
    case TH_IO_OP_READ:
        op_str = "READ";
        break;
    case TH_IO_OP_WRITE:
        op_str = "WRITE";
        break;
    case TH_IO_OP_WRITEV:
        op_str = "WRITEV";
        break;
    case TH_IO_OP_READV:
        op_str = "READV";
        break;
    case TH_IO_OP_SENDFILE:
        op_str = "SENDFILE";
        break;
    case TH_IO_OP_ACCEPT:
        op_str = "ACCEPT";
        break;
    default:
        op_str = "UNKNOWN";
        break;
    }
    snprintf(buf, len, "th_io_task(%s, fd=%d, fd2=%d, addr=%p, len=%u)", op_str, iot->fd, iot->fd2, iot->addr, (unsigned int)iot->len);
}
*/

TH_LOCAL(void)
th_io_task_prepare_read_write(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete, enum th_io_op op)
{
    iot->op = op;
    iot->fd = fd;
    iot->addr = addr;
    iot->len = len;
    iot->on_complete = on_complete;
}

TH_PRIVATE(void)
th_io_task_prepare_read(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_read;
    th_io_task_prepare_read_write(iot, fd, addr, len, on_complete, TH_IO_OP_READ);
}

/*
TH_PRIVATE(void)
th_io_task_prepare_write(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_write;
    th_io_task_prepare_read_write(iot, fd, addr, len, on_complete, TH_IO_OP_WRITE);
}

TH_PRIVATE(void)
th_io_task_prepare_writev(th_io_task* iot, int fd, th_iov* iov, size_t iovcnt, th_io_handler* on_complete)
{
    iot->fn = th_io_op_writev;
    th_io_task_prepare_read_write(iot, fd, iov, iovcnt, on_complete, TH_IO_OP_WRITEV);
}

*/

TH_PRIVATE(void)
th_io_task_prepare_send(th_io_task* iot, int fd, void* addr, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_send;
    th_io_task_prepare_read_write(iot, fd, addr, len, on_complete, TH_IO_OP_SEND);
}

TH_PRIVATE(void)
th_io_task_prepare_sendv(th_io_task* iot, int fd, th_iov* iov, size_t iovcnt, th_io_handler* on_complete)
{
    iot->fn = th_io_op_sendv;
    th_io_task_prepare_read_write(iot, fd, iov, iovcnt, on_complete, TH_IO_OP_SENDV);
}

TH_PRIVATE(void)
th_io_task_prepare_readv(th_io_task* iot, int fd, th_iov* iov, size_t iovcnt, th_io_handler* on_complete)
{
    iot->fn = th_io_op_readv;
    th_io_task_prepare_read_write(iot, fd, iov, iovcnt, on_complete, TH_IO_OP_READV);
}

TH_PRIVATE(void)
th_io_task_prepare_sendfile(th_io_task* iot, th_file* file, int sfd, th_iov* header, size_t iovcnt,
                            size_t offset, size_t len, th_io_handler* on_complete)
{
    iot->fn = th_io_op_sendfile;
    iot->op = TH_IO_OP_SENDFILE;
    iot->fd = sfd;
    iot->addr2 = file;
    iot->addr = header;
    iot->len = iovcnt;
    iot->offset = offset;
    iot->len2 = len;
    iot->flags = 0;
    iot->on_complete = on_complete;
}

TH_PRIVATE(void)
th_io_task_prepare_accept(th_io_task* iot, int fd, void* addr, void* addrlen, th_io_handler* on_complete)
{
    iot->fn = th_io_op_accept;
    iot->op = TH_IO_OP_ACCEPT;
    iot->fd = fd;
    iot->addr = addr;
    iot->addr2 = addrlen;
    iot->on_complete = on_complete;
}

TH_PRIVATE(th_err)
th_io_task_execute(th_io_task* iot, size_t* result)
{
    return iot->fn(iot, result);
}

TH_PRIVATE(th_io_handler*)
th_io_task_try_execute(th_io_task* iot)
{
    size_t result = 0;
    th_err err = th_io_task_execute(iot, &result);
    if (err == TH_ERR_SYSTEM(TH_EAGAIN)
        || err == TH_ERR_SYSTEM(TH_EWOULDBLOCK)) {
        return NULL;
    }
    th_io_handler* on_complete = TH_MOVE_PTR(iot->on_complete);
    th_task_destroy(&iot->base);
    th_io_handler_set_result(on_complete, result, err);
    return on_complete;
}

TH_PRIVATE(void)
th_io_task_destroy(th_io_task* iot)
{
    th_task_destroy(&iot->base);
}

TH_PRIVATE(th_io_handler*)
th_io_task_abort(th_io_task* iot, th_err err)
{
    th_io_handler* on_complete = TH_MOVE_PTR(iot->on_complete);
    th_io_handler_set_result(on_complete, 0, err);
    th_io_task_destroy(iot);
    return on_complete;
}
