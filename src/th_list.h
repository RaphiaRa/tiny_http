#ifndef TH_LIST
#define TH_LIST

#include "th_config.h"
#include "th_utility.h"

/** Generic doubly linked list implementation.
 * that works with any struct that has a next and prev pointer.
 */
#define TH_DEFINE_LIST(NAME, T, PREV, NEXT)                                         \
    typedef struct NAME {                                                           \
        T* head;                                                                    \
        T* tail;                                                                    \
    } NAME;                                                                         \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_push_back(NAME* list, T* item) TH_MAYBE_UNUSED;                          \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_pop_front(NAME* list) TH_MAYBE_UNUSED;                                   \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_front(NAME* list) TH_MAYBE_UNUSED;                                       \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_erase(NAME* list, T* item) TH_MAYBE_UNUSED;                              \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_next(T* item) TH_MAYBE_UNUSED;                                           \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_push_back(NAME* list, T* item)                                           \
    {                                                                               \
        TH_ASSERT(item != NULL);                                                    \
        if (list->head == NULL) {                                                   \
            list->head = item;                                                      \
            item->PREV = NULL;                                                      \
        } else {                                                                    \
            list->tail->NEXT = item;                                                \
            item->PREV = list->tail;                                                \
        }                                                                           \
        list->tail = item;                                                          \
        item->NEXT = NULL;                                                          \
    }                                                                               \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_pop_front(NAME* list)                                                    \
    {                                                                               \
        T* item = list->head;                                                       \
        if (item) {                                                                 \
            list->head = item->NEXT;                                                \
            if (list->head) {                                                       \
                list->head->PREV = NULL;                                            \
            } else {                                                                \
                list->tail = NULL;                                                  \
            }                                                                       \
            item->NEXT = NULL;                                                      \
        }                                                                           \
        return item;                                                                \
    }                                                                               \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_front(NAME* list)                                                        \
    {                                                                               \
        return list->head;                                                          \
    }                                                                               \
                                                                                    \
    TH_INLINE(void)                                                                 \
    NAME##_erase(NAME* list, T* item)                                               \
    {                                                                               \
        TH_ASSERT(item != NULL);                                                    \
        TH_ASSERT((item->NEXT || item == list->tail) && "Item is not in the list"); \
        TH_ASSERT((item->PREV || item == list->head) && "Item is not in the list"); \
        T* next = item->NEXT;                                                       \
        T* prev = item->PREV;                                                       \
        if (prev) {                                                                 \
            prev->NEXT = next;                                                      \
            item->PREV = NULL;                                                      \
        } else {                                                                    \
            list->head = next;                                                      \
        }                                                                           \
        if (next) {                                                                 \
            next->PREV = prev;                                                      \
            item->NEXT = NULL;                                                      \
        } else {                                                                    \
            list->tail = prev;                                                      \
        }                                                                           \
    }                                                                               \
                                                                                    \
    TH_INLINE(T*)                                                                   \
    NAME##_next(T* item)                                                            \
    {                                                                               \
        return item->NEXT;                                                          \
    }

#endif
