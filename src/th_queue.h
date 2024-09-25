#ifndef TH_QUEUE_H
#define TH_QUEUE_H

#include "th_config.h"

#include <stdbool.h>

/** Generic queue implementation.
 * that works with any struct that has a next pointer.
 */
#define TH_DEFINE_QUEUE(NAME, T)                                 \
    typedef struct NAME {                                        \
        T* head;                                                 \
        T* tail;                                                 \
    } NAME;                                                      \
                                                                 \
    TH_INLINE(NAME)                                              \
    NAME##_make(void) TH_MAYBE_UNUSED;                           \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push(NAME* queue, T* item) TH_MAYBE_UNUSED;           \
                                                                 \
    TH_INLINE(T*)                                                \
    NAME##_pop(NAME* queue) TH_MAYBE_UNUSED;                     \
                                                                 \
    TH_INLINE(bool)                                              \
    NAME##_empty(NAME* queue) TH_MAYBE_UNUSED;                   \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push_queue(NAME* queue, NAME* other) TH_MAYBE_UNUSED; \
                                                                 \
    TH_INLINE(NAME)                                              \
    NAME##_make(void)                                            \
    {                                                            \
        return (NAME){.head = NULL, .tail = NULL};               \
    }                                                            \
                                                                 \
    TH_INLINE(bool)                                              \
    NAME##_empty(NAME* queue)                                    \
    {                                                            \
        return queue->head == NULL;                              \
    }                                                            \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push(NAME* queue, T* item)                            \
    {                                                            \
        if (queue->head == NULL) {                               \
            queue->head = item;                                  \
        } else {                                                 \
            queue->tail->next = item;                            \
        }                                                        \
        queue->tail = item;                                      \
        item->next = NULL;                                       \
    }                                                            \
                                                                 \
    TH_INLINE(void)                                              \
    NAME##_push_queue(NAME* queue, NAME* other)                  \
    {                                                            \
        if (queue->head == NULL) {                               \
            *queue = *other;                                     \
        } else if (other->head) {                                \
            queue->tail->next = other->head;                     \
            queue->tail = other->tail;                           \
        }                                                        \
        *other = NAME##_make();                                  \
    }                                                            \
                                                                 \
    TH_INLINE(T*)                                                \
    NAME##_pop(NAME* queue)                                      \
    {                                                            \
        T* item = queue->head;                                   \
        if (item) {                                              \
            queue->head = item->next;                            \
            item->next = NULL;                                   \
        }                                                        \
        return item;                                             \
    }

#endif
