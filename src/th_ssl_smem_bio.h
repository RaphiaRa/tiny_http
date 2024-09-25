#ifndef TH_SSL_SMEM_BIO_H
#define TH_SSL_SMEM_BIO_H

#include "th_config.h"

#if TH_WITH_SSL

#include <openssl/bio.h>

#include "th_allocator.h"
#include "th_iov.h"
#include "th_ssl_context.h"

TH_PRIVATE(BIO_METHOD*)
th_smem_bio(th_ssl_context* ssl_context);

TH_PRIVATE(void)
th_smem_bio_setup_buf(BIO* bio, th_allocator* allocator, size_t max_len);

TH_PRIVATE(size_t)
th_smem_ensure_buf_size(BIO* bio, size_t size);

TH_PRIVATE(void)
th_smem_bio_set_eof(BIO* bio);

TH_PRIVATE(void)
th_smem_bio_get_rdata(BIO* bio, th_iov* buf);

TH_PRIVATE(void)
th_smem_bio_get_wbuf(BIO* bio, th_iov* buf);

TH_PRIVATE(void)
th_smem_bio_inc_read_pos(BIO* bio, size_t len);

TH_PRIVATE(void)
th_smem_bio_inc_write_pos(BIO* bio, size_t len);

#endif
#endif
