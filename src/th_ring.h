#ifndef TH_RING_H
#define TH_RING_H

#include <th.h>

#include "th_allocator.h"
#include "th_config.h"
#include "th_iov.h"
#include "th_queue.h"

#include <stddef.h>

typedef struct th_ring_chunk {
    struct th_ring_chunk* next;
    unsigned char* data;
    size_t capacity;
    // head/tail only ever increase - never wrapped themselves, so
    // len = tail - head and full = (tail - head == capacity) always hold.
    // Actual buffer offsets are head % capacity / tail % capacity.
    size_t head;
    size_t tail;
} th_ring_chunk;

/* th_ring_chunk_queue declarations begin */

#ifndef TH_RING_CHUNK_QUEUE
#define TH_RING_CHUNK_QUEUE
TH_DEFINE_QUEUE(th_ring_chunk_queue, th_ring_chunk)
#endif

/* th_ring_chunk_queue declarations end */

/** th_ring
 * @brief FIFO byte queue backed by a linked list of ring-buffer chunks.
 * Writes always land in the tail chunk; once it's full a new, twice as
 * large chunk is appended. Reads (peek/consume) only ever touch the head
 * chunk, which is freed once fully consumed.
 */
typedef struct th_ring {
    th_ring_chunk_queue chunks;
    size_t len;              // total bytes currently queued, across all chunks
    size_t initial_capacity; // size of the first chunk, allocated lazily on first write
    size_t max_len;          // th_ring_write rejects anything that would exceed this
    th_allocator* allocator;
} th_ring;

TH_PRIVATE(void)
th_ring_init(th_ring* rb, th_allocator* allocator, size_t initial_capacity, size_t max_len);

TH_PRIVATE(void)
th_ring_deinit(th_ring* rb);

/** th_ring_write
 * @brief Queues parts as one message (never split across chunks),
 * growing (doubling the tail chunk) if it doesn't have room.
 *
 * - TH_ERR_INVALID_ARG: total size alone exceeds max_len, retrying never helps
 * - TH_ERR_SYSTEM(TH_EAGAIN): fits under max_len, but a chunk allocation failed
 */
TH_PRIVATE(th_err)
th_ring_write(th_ring* rb, const th_iov* parts, size_t partcnt);

/** th_ring_peek
 * @brief Fills iov[0..1] with the head chunk's queued bytes (iov[1] only
 * used if that chunk's queued run wraps past the end of its buffer).
 * @return Number of iov entries filled (0, 1, or 2).
 */
TH_PRIVATE(size_t)
th_ring_peek(th_ring* rb, th_iov iov[2]);

/** th_ring_consume
 * @brief Marks the oldest len queued bytes as sent, freeing that space.
 * Frees the head chunk once it's fully drained.
 */
TH_PRIVATE(void)
th_ring_consume(th_ring* rb, size_t len);

#endif
