#include "th_ssl_session.h"

#if TH_WITH_SSL

#include "th_ssl_smem_bio.h"

#include <openssl/err.h>

TH_PRIVATE(th_err)
th_ssl_session_init(th_ssl_session* session, th_ssl_context* context, th_ssl_ops* ops, th_allocator* allocator)
{
    session->ops = ops;
    th_err err = TH_ERR_OK;
    session->ssl = session->ops->new_ssl(session->ops, context->ctx);
    if (!session->ssl) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_none;
    }
    session->wbio = BIO_new(th_smem_bio(context));
    if (!session->wbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_ssl;
    }
    session->rbio = BIO_new(th_smem_bio(context));
    if (!session->rbio) {
        err = TH_ERR_SSL(SSL_ERROR_SSL);
        goto cleanup_wbio;
    }
    th_smem_bio_setup_buf(session->wbio, allocator, TH_CONFIG_MAX_SSL_WRITE_BUF_LEN);
    th_smem_bio_setup_buf(session->rbio, allocator, TH_CONFIG_MAX_SSL_READ_BUF_LEN);
    session->ops->set_bio(session->ops, session->ssl, session->rbio, session->wbio);
    session->ops->set_accept_state(session->ops, session->ssl);
    session->ops->set_partial_write(session->ops, session->ssl);
    return TH_ERR_OK;
cleanup_wbio:
    BIO_free(session->wbio);
cleanup_ssl:
    session->ops->free_ssl(session->ops, session->ssl);
cleanup_none:
    return err;
}

TH_PRIVATE(void)
th_ssl_session_deinit(th_ssl_session* session)
{
    /* SSL_set_bio transferred ownership of rbio/wbio to session->ssl;
     * SSL_free (behind free_ssl) frees them, so don't BIO_free here. */
    session->ops->free_ssl(session->ops, session->ssl);
}

TH_LOCAL(th_ssl_result)
th_ssl_session_classify(th_ssl_session* session, int ret)
{
    if (BIO_pending(session->wbio) > 0)
        return TH_SSL_WANT_WRITE;
    int code = session->ops->get_error(session->ops, session->ssl, ret);
    if (code == SSL_ERROR_WANT_READ)
        return TH_SSL_WANT_READ;
    if (code == SSL_ERROR_WANT_WRITE)
        return TH_SSL_WANT_WRITE;
    return TH_SSL_ERROR;
}

TH_LOCAL(th_err)
th_ssl_session_error(th_ssl_session* session, int ret)
{
    int code = session->ops->get_error(session->ops, session->ssl, ret);
    if (code == SSL_ERROR_ZERO_RETURN)
        return TH_ERR_EOF;
    return TH_ERR_SSL(code);
}

TH_PRIVATE(th_ssl_result)
th_ssl_session_handshake(th_ssl_session* session, th_err* err)
{
    int ret = session->ops->do_handshake(session->ops, session->ssl);
    if (ret == 1) {
        *err = TH_ERR_OK;
        return BIO_pending(session->wbio) > 0 ? TH_SSL_WANT_WRITE : TH_SSL_DONE;
    }
    th_ssl_result result = th_ssl_session_classify(session, ret);
    if (result == TH_SSL_ERROR) {
        *err = th_ssl_session_error(session, ret);
        return result;
    }
    *err = TH_ERR_OK;
    return result;
}

TH_PRIVATE(th_ssl_result)
th_ssl_session_read(th_ssl_session* session, void* buf, size_t len, size_t* out, th_err* err)
{
    int ret = session->ops->read(session->ops, session->ssl, buf, (int)len);
    if (ret > 0) {
        *out = (size_t)ret;
        *err = TH_ERR_OK;
        return BIO_pending(session->wbio) > 0 ? TH_SSL_WANT_WRITE : TH_SSL_DONE;
    }
    *out = 0;
    th_ssl_result result = th_ssl_session_classify(session, ret);
    if (result == TH_SSL_ERROR) {
        *err = th_ssl_session_error(session, ret);
        return result;
    }
    *err = TH_ERR_OK;
    return result;
}

TH_PRIVATE(th_ssl_result)
th_ssl_session_write(th_ssl_session* session, const void* buf, size_t len, size_t* out, th_err* err)
{
    int ret = session->ops->write(session->ops, session->ssl, buf, (int)len);
    if (ret > 0) {
        *out = (size_t)ret;
        *err = TH_ERR_OK;
        return TH_SSL_WANT_WRITE;
    }
    *out = 0;
    int code = session->ops->get_error(session->ops, session->ssl, ret);
    if (code == SSL_ERROR_WANT_READ) {
        *err = TH_ERR_OK;
        return TH_SSL_WANT_READ;
    }
    *err = th_ssl_session_error(session, ret);
    return TH_SSL_ERROR;
}

TH_PRIVATE(void)
th_ssl_session_get_ciphertext_out(th_ssl_session* session, th_iov* iov)
{
    th_smem_bio_get_rdata(session->wbio, iov);
}

TH_PRIVATE(void)
th_ssl_session_consume_ciphertext_out(th_ssl_session* session, size_t n)
{
    th_smem_bio_inc_read_pos(session->wbio, n);
}

TH_PRIVATE(void)
th_ssl_session_get_ciphertext_in_buf(th_ssl_session* session, th_iov* iov)
{
    th_smem_ensure_buf_size(session->rbio, TH_CONFIG_MAX_SSL_READ_BUF_LEN);
    th_smem_bio_get_wbuf(session->rbio, iov);
}

TH_PRIVATE(void)
th_ssl_session_fed_ciphertext_in(th_ssl_session* session, size_t n)
{
    if (n == 0) {
        th_smem_bio_set_eof(session->rbio);
        return;
    }
    th_smem_bio_inc_write_pos(session->rbio, n);
}

#endif
