#include "th_ring.h"

#include "th_align.h"
#include "th_system_error.h"

#include <string.h>

// Chunk header + backing buffer live in one allocation - data points at
// an offset into the same block, rounded up so it's th_max_align-aligned.
#define TH_RING_CHUNK_HEADER_LEN TH_ALIGNUP(sizeof(th_ring_chunk), TH_ALIGNOF(th_max_align))

TH_LOCAL(th_ring_chunk*)
th_ring_chunk_create(th_allocator* allocator, size_t capacity)
{
    th_ring_chunk* chunk = th_allocator_alloc(allocator, TH_RING_CHUNK_HEADER_LEN + capacity);
    if (!chunk)
        return NULL;
    chunk->data = (unsigned char*)chunk + TH_RING_CHUNK_HEADER_LEN;
    chunk->capacity = capacity;
    chunk->head = 0;
    chunk->tail = 0;
    return chunk;
}

TH_LOCAL(size_t)
th_ring_chunk_len(const th_ring_chunk* chunk)
{
    return chunk->tail - chunk->head;
}

TH_LOCAL(size_t)
th_ring_chunk_free_space(const th_ring_chunk* chunk)
{
    return chunk->capacity - th_ring_chunk_len(chunk);
}

TH_LOCAL(void)
th_ring_chunk_write(th_ring_chunk* chunk, const void* data, size_t len)
{
    if (len == 0)
        return;
    size_t offset = chunk->tail % chunk->capacity;
    size_t first = chunk->capacity - offset < len ? chunk->capacity - offset : len;
    memcpy(chunk->data + offset, data, first);
    memcpy(chunk->data, (const unsigned char*)data + first, len - first);
    chunk->tail += len;
}

TH_PRIVATE(void)
th_ring_init(th_ring* rb, th_allocator* allocator, size_t initial_capacity, size_t max_len)
{
    rb->chunks = th_ring_chunk_queue_make();
    rb->len = 0;
    rb->initial_capacity = initial_capacity;
    rb->max_len = max_len;
    rb->allocator = allocator ? allocator : th_default_allocator_get();
}

TH_PRIVATE(void)
th_ring_deinit(th_ring* rb)
{
    th_ring_chunk* chunk;
    while ((chunk = th_ring_chunk_queue_pop(&rb->chunks)) != NULL)
        th_allocator_free(rb->allocator, chunk);
}

TH_LOCAL(size_t)
th_ring_parts_len(const th_iov* parts, size_t partcnt)
{
    size_t len = 0;
    for (size_t i = 0; i < partcnt; ++i)
        len += parts[i].len;
    return len;
}

TH_PRIVATE(th_err)
th_ring_write(th_ring* rb, const th_iov* parts, size_t partcnt)
{
    size_t len = th_ring_parts_len(parts, partcnt);
    if (rb->len + len > rb->max_len)
        return TH_ERR_INVALID_ARG;

    th_ring_chunk* tail_chunk = rb->chunks.tail;
    if (!tail_chunk || th_ring_chunk_free_space(tail_chunk) < len) {
        size_t capacity = tail_chunk ? tail_chunk->capacity * 2 : rb->initial_capacity;
        if (capacity < len)
            capacity = len;
        th_ring_chunk* chunk = th_ring_chunk_create(rb->allocator, capacity);
        if (!chunk)
            return TH_ERR_SYSTEM(TH_EAGAIN);

        // an empty tail chunk is unreachable once anything is queued
        // behind it - peek/consume only ever advance from chunks.head
        if (tail_chunk && th_ring_chunk_len(tail_chunk) == 0) {
            th_ring_chunk_queue_pop(&rb->chunks);
            th_allocator_free(rb->allocator, tail_chunk);
        }
        th_ring_chunk_queue_push(&rb->chunks, chunk);
        tail_chunk = chunk;
    }

    for (size_t i = 0; i < partcnt; ++i)
        th_ring_chunk_write(tail_chunk, parts[i].base, parts[i].len);
    rb->len += len;
    return TH_ERR_OK;
}

TH_PRIVATE(size_t)
th_ring_peek(th_ring* rb, th_iov iov[2])
{
    th_ring_chunk* chunk = rb->chunks.head;
    size_t len = chunk ? th_ring_chunk_len(chunk) : 0;
    if (len == 0)
        return 0;

    size_t offset = chunk->head % chunk->capacity;
    size_t first = chunk->capacity - offset < len ? chunk->capacity - offset : len;
    iov[0].base = chunk->data + offset;
    iov[0].len = first;
    if (first == len)
        return 1;

    iov[1].base = chunk->data;
    iov[1].len = len - first;
    return 2;
}

TH_PRIVATE(void)
th_ring_consume(th_ring* rb, size_t len)
{
    th_ring_chunk* chunk = rb->chunks.head;
    chunk->head += len;
    rb->len -= len;

    bool drained = th_ring_chunk_len(chunk) == 0;
    bool sole_chunk = chunk == rb->chunks.tail;
    if (!drained || (sole_chunk && chunk->capacity <= rb->initial_capacity))
        return;

    th_ring_chunk_queue_pop(&rb->chunks);
    th_allocator_free(rb->allocator, chunk);
}
