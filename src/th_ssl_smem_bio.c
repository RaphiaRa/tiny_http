#include "th_ssl_smem_bio.h"
#if TH_WITH_SSL

#include <assert.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <string.h>

#include "th_utility.h"
#include "th_vec.h"

typedef struct th_static_bio_data {
    th_buf_vec buf;
    size_t max_len;
    size_t read_pos;
    size_t write_pos;
    int eof;
} th_static_bio_data;

TH_LOCAL(int)
th_smem_new(BIO* bio);

TH_LOCAL(int)
th_smem_free(BIO* bio);

TH_LOCAL(int)
th_smem_read(BIO* bio, char* out, int outl);

TH_LOCAL(int)
th_smem_write(BIO* bio, const char* in, int inl);

TH_LOCAL(long)
th_smem_ctrl(BIO* bio, int cmd, long num, void* ptr);

TH_PRIVATE(BIO_METHOD*)
th_smem_bio(th_ssl_context* ssl_context)
{
    if (ssl_context->smem_method == NULL) {
        ssl_context->smem_method = BIO_meth_new(BIO_TYPE_MEM, "th_smem");
        BIO_meth_set_write(ssl_context->smem_method, th_smem_write);
        BIO_meth_set_read(ssl_context->smem_method, th_smem_read);
        BIO_meth_set_ctrl(ssl_context->smem_method, th_smem_ctrl);
        BIO_meth_set_create(ssl_context->smem_method, th_smem_new);
        BIO_meth_set_destroy(ssl_context->smem_method, th_smem_free);
    }
    return ssl_context->smem_method;
}

TH_PRIVATE(void)
th_smem_bio_setup_buf(BIO* bio, th_allocator* allocator, size_t max_len)
{
    th_static_bio_data* data = BIO_get_data(bio);
    th_buf_vec_init(&data->buf, allocator);
    data->max_len = max_len;
    data->read_pos = 0;
    data->write_pos = 0;
    BIO_set_init(bio, 1);
}

TH_PRIVATE(size_t)
th_smem_ensure_buf_size(BIO* bio, size_t size)
{
    th_static_bio_data* data = BIO_get_data(bio);
    size = TH_MIN(size, data->max_len);
    if (th_buf_vec_size(&data->buf) < size) {
        (void)th_buf_vec_resize(&data->buf, size);
        size = th_buf_vec_size(&data->buf);
    }
    return size;
}

TH_PRIVATE(void)
th_smem_bio_get_rdata(BIO* bio, th_iov* buf)
{
    th_static_bio_data* bio_data = BIO_get_data(bio);
    buf->len = (bio_data->write_pos - bio_data->read_pos);
    buf->base = th_buf_vec_begin(&bio_data->buf) + bio_data->read_pos;
}

TH_PRIVATE(void)
th_smem_bio_get_wbuf(BIO* bio, th_iov* buf)
{
    th_static_bio_data* bio_data = BIO_get_data(bio);
    buf->len = th_buf_vec_size(&bio_data->buf) - bio_data->write_pos;
    buf->base = th_buf_vec_begin(&bio_data->buf) + bio_data->write_pos;
}

TH_PRIVATE(void)
th_smem_bio_inc_read_pos(BIO* bio, size_t len)
{
    th_static_bio_data* data = BIO_get_data(bio);
    data->read_pos += len;
    if (data->read_pos == data->write_pos) {
        data->read_pos = 0;
        data->write_pos = 0;
    }
}

TH_PRIVATE(void)
th_smem_bio_inc_write_pos(BIO* bio, size_t len)
{
    th_static_bio_data* data = BIO_get_data(bio);
    data->write_pos += len;
}

TH_PRIVATE(void)
th_smem_bio_set_eof(BIO* bio)
{
    th_static_bio_data* data = BIO_get_data(bio);
    data->eof = 1;
}

TH_LOCAL(int)
th_smem_new(BIO* bio)
{
    th_static_bio_data* data = OPENSSL_malloc(sizeof(th_static_bio_data));
    if (data == NULL)
        return 0;
    data->eof = 0;
    data->read_pos = 0;
    data->write_pos = 0;
    BIO_set_data(bio, data);
    return 1;
}

TH_LOCAL(int)
th_smem_free(BIO* bio)
{
    TH_ASSERT(bio);
    if (BIO_get_init(bio))
        th_buf_vec_deinit(&((th_static_bio_data*)BIO_get_data(bio))->buf);
    BIO_set_init(bio, 0);
    OPENSSL_free(BIO_get_data(bio));
    BIO_set_data(bio, NULL);
    return 1;
}

TH_LOCAL(int)
th_smem_read(BIO* bio, char* out, int outl)
{
    th_static_bio_data* data = BIO_get_data(bio);
    TH_ASSERT(data);
    TH_ASSERT(out);
    TH_ASSERT(outl > 0);
    size_t s = TH_MIN((size_t)outl, data->write_pos - data->read_pos);
    if (s == 0) {
        if (data->eof)
            return 0;
        BIO_set_retry_read(bio);
        return -1;
    }
    memcpy(out, th_buf_vec_begin(&data->buf) + data->read_pos, s);
    th_smem_bio_inc_read_pos(bio, s);
    return (int)s;
}

TH_LOCAL(int)
th_smem_write(BIO* bio, const char* in, int inlen)
{
    th_static_bio_data* data = BIO_get_data(bio);
    if (data->eof) // no more writing after eof
        return 0;
    TH_ASSERT(data);
    TH_ASSERT(in);
    TH_ASSERT(inlen > 0);
    size_t buflen = th_buf_vec_size(&data->buf);
    if (data->write_pos + (size_t)inlen > buflen)
        buflen = th_smem_ensure_buf_size(bio, data->write_pos + (size_t)inlen);
    size_t s = TH_MIN((size_t)inlen, buflen - data->write_pos);
    if (s == 0) {
        BIO_set_retry_write(bio);
        return -1;
    }
    memcpy(th_buf_vec_begin(&data->buf) + data->write_pos, in, s);
    data->write_pos += s;
    return (int)s;
}

TH_LOCAL(long)
th_smem_ctrl(BIO* bio, int cmd, long num, void* ptr)
{
    (void)num;
    th_static_bio_data* data = BIO_get_data(bio);
    TH_ASSERT(data);
    long ret = 1;

    switch (cmd) {
    case BIO_CTRL_RESET:
        data->read_pos = 0;
        data->write_pos = 0;
        data->eof = 0;
        break;
    case BIO_CTRL_EOF:
        ret = (data->eof && data->read_pos == data->write_pos);
        break;
    case BIO_CTRL_INFO: {
        ret = (long)th_buf_vec_size(&data->buf);
        if (ptr != NULL)
            *(void**)ptr = th_buf_vec_begin(&data->buf);
        break;
    }
    case BIO_CTRL_PENDING:
        ret = (long)(data->write_pos - data->read_pos);
        break;
    case BIO_CTRL_WPENDING:
        ret = 0;
        break;
    case BIO_CTRL_DUP:
    case BIO_CTRL_FLUSH:
        ret = 1;
        break;
    default:
        ret = 0;
        break;
    }
    return ret;
}
#endif
