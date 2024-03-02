// DOCUMENTATION
//
// DS.H
//
// This is a single-header library that provides a set of data structures and
// utilities for C programs. The library is designed to be simple and easy to
// use, and it is intended to be used in small to medium-sized projects.
//
// Options:
// - DS_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of all the data structures and utilities
// - DS_PQ_IMPLEMENTATION: Define this macro in one source file to include the
//  implementation of the priority queue data structure
//  - DS_SB_IMPLEMENTATION: Define this macro in one source file to include the
//  implementation of the string builder utility
//  - DS_SS_IMPLEMENTATION: Define this macro in one source file to include the
//  implementation of the string slice utility
//  - DS_DA_IMPLEMENTATION: Define this macro in one source file to include the
//  implementation of the dynamic array data structure
//  - DS_LL_IMPLEMENTATION: Define this macro in one source file to include the
//  implementation of the linked list data structure
//
// MEMORY MANAGEMENT
//
// The memory management macros are used to allocate, reallocate and free
// memory, and to exit the program. The DS_MALLOC macro is used to allocate
// memory, the DS_REALLOC macro is used to reallocate memory, the DS_FREE macro
// is used to free memory, and the DS_EXIT macro is used to exit the program.
//
// Options:
// - DS_NO_STDLIB: Disables the use of the standard library
//
// LOGGING
//
// The logging macros are used to print messages to the standard output and
// standard error streams. The DS_LOG_ERROR macro is used to print error
// messages, and the DS_LOG_INFO macro is used to print informational messages.
// The DS_PANIC macro is used to print an error message and exit the program.
//
// Options:
// - DS_NO_STDIO: Disables the use of the standard input/output streams
// - DS_QUIET: Disables the use of the standard error stream
// - DS_NO_LOG_ERROR: Disables the use of the DS_LOG_ERROR macro
// - DS_NO_LOG_INFO: Disables the use of the DS_LOG_INFO macro
// - DS_NO_TERMINAL_COLORS: Disables the use of terminal colors in the output

#ifndef DS_H
#define DS_H

#ifndef DS_NO_STDLIB
#include <stdlib.h>
#include <string.h>

#define DS_MALLOC(sz) malloc(sz)
#define DS_REALLOC(ptr, old_sz, new_sz) realloc(ptr, new_sz)
#define DS_FREE(ptr) free(ptr)
#define DS_MEMCPY(dst, src, sz) memcpy(dst, src, sz)
#define DS_MEMCMP(ptr1, ptr2, sz) memcmp(ptr1, ptr2, sz)

#ifndef DS_EXIT
#define DS_EXIT(code) exit(code)
#endif
#else // DS_NO_STDLIB
#if defined(DS_MALLOC) && defined(DS_FREE) && defined(DS_EXIT)
// ok
#else
#error "Must define DS_MALLOC, DS_FREE and DS_EXIT when DS_NO_STDLIB is defined"
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef DS_MEMCPY
#define DS_MEMCPY(dst, src, sz)                                                \
    do {                                                                       \
        for (unsigned int i = 0; i < sz; i++) {                                \
            ((char *)dst)[i] = ((char *)src)[i];                               \
        }                                                                      \
    } while (0)
#endif

#ifndef DS_MEMCMP
#define DS_MEMCMP(ptr1, ptr2, sz)                                              \
    ({                                                                         \
        int result = 0;                                                        \
        for (unsigned int i = 0; i < sz; i++) {                                \
            if (((char *)ptr1)[i] != ((char *)ptr2)[i]) {                      \
                result = ((char *)ptr1)[i] - ((char *)ptr2)[i];                \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        result;                                                                \
    })
#endif

#ifndef DS_REALLOC
static void *ds_realloc(void *ptr, unsigned int old_sz, unsigned int new_sz) {
    void *new_ptr = DS_MALLOC(new_sz);
    if (new_ptr == NULL) {
        DS_FREE(ptr);
        return NULL;
    }
    DS_MEMCPY(new_ptr, ptr, old_sz < new_sz ? old_sz : new_sz);
    DS_FREE(ptr);
    return new_ptr;
}
#define DS_REALLOC(ptr, old_sz, new_sz) ds_realloc(ptr, old_sz, new_sz)
#endif
#endif // DS_NO_STDLIB

#ifndef DS_NO_STDIO
#include <stdio.h>
#endif

#ifdef DS_QUIET
#define DS_NO_LOG_ERROR
#define DS_NO_LOG_INFO
#endif

#ifdef DS_NO_TERMINAL_COLORS
#define DS_TERMINAL_RED ""
#define DS_TERMINAL_BLUE ""
#define DS_TERMINAL_RESET ""
#else
#define DS_TERMINAL_RED "\033[1;31m"
#define DS_TERMINAL_BLUE "\033[1;34m"
#define DS_TERMINAL_RESET "\033[0m"
#endif

#if defined(DS_NO_STDIO) || defined(DS_NO_LOG_ERROR)
#define DS_LOG_ERROR(format, ...)
#else
#define DS_LOG_ERROR(format, ...)                                              \
    fprintf(stderr,                                                            \
            DS_TERMINAL_RED "ERROR" DS_TERMINAL_RESET ": %s:%d: " format "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(DS_NO_STDIO) || defined(DS_NO_LOG_INFO)
#define DS_LOG_INFO(format, ...)
#else
#define DS_LOG_INFO(format, ...)                                               \
    fprintf(stdout,                                                            \
            DS_TERMINAL_BLUE "INFO" DS_TERMINAL_RESET ": %s:%d: " format "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define DS_PANIC(format, ...)                                                  \
    do {                                                                       \
        DS_LOG_ERROR(format, ##__VA_ARGS__);                                   \
        DS_EXIT(1);                                                            \
    } while (0)

// RETURN DEFER
//
// The return_defer macro is a simple way to return a value and jump to a label
// to execute cleanup code. It is similar to the defer statement in Go.

#ifndef return_defer
#define return_defer(code)                                                     \
    do {                                                                       \
        result = code;                                                         \
        goto defer;                                                            \
    } while (0)
#endif // return_defer

// DYNAMIC ARRAY
//
// The dynamic array is a simple array that grows as needed. To use the dynamic
// array append macro, you need to define a struct with the following fields:
//  - items: a pointer to the array of items
//  - count: the number of items in the array
//  - capacity: the number of items that can be stored in the array

#define DS_DA_INIT_CAPACITY 8192
#define ds_da_append(da, item)                                                 \
    do {                                                                       \
        if ((da)->count >= (da)->capacity) {                                   \
            unsigned int new_capacity = (da)->capacity * 2;                    \
            if (new_capacity == 0) {                                           \
                new_capacity = DS_DA_INIT_CAPACITY;                            \
            }                                                                  \
                                                                               \
            (da)->items =                                                      \
                DS_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items), \
                           new_capacity * sizeof(*(da)->items));               \
            if ((da)->items == NULL) {                                         \
                DS_PANIC("Failed to reallocate dynamic array");                \
            }                                                                  \
                                                                               \
            (da)->capacity = new_capacity;                                     \
        }                                                                      \
                                                                               \
        (da)->items[(da)->count++] = (item);                                   \
    } while (0)

#define ds_da_append_many(da, new_items, new_items_count)                      \
    do {                                                                       \
        if ((da)->count + new_items_count > (da)->capacity) {                  \
            if ((da)->capacity == 0) {                                         \
                (da)->capacity = DS_DA_INIT_CAPACITY;                          \
            }                                                                  \
            while ((da)->count + new_items_count > (da)->capacity) {           \
                (da)->capacity *= 2;                                           \
            }                                                                  \
                                                                               \
            (da)->items =                                                      \
                DS_REALLOC((da)->items, (da)->capacity * sizeof(*(da)->items), \
                           (da)->capacity * sizeof(*(da)->items));             \
            if ((da)->items == NULL) {                                         \
                DS_PANIC("Failed to reallocate dynamic array");                \
            }                                                                  \
        }                                                                      \
                                                                               \
        DS_MEMCPY((da)->items + (da)->count, new_items,                        \
                  new_items_count * sizeof(*(da)->items));                     \
        (da)->count += new_items_count;                                        \
    } while (0)

#ifndef DSHDEF
#ifdef DSH_STATIC
#define DSHDEF static
#else
#define DSHDEF extern
#endif
#endif

// PRIORITY QUEUE
//
// The priority queue is implemented as a heap, where you can define the
// comparison function to use when inserting items. The comparison function
// should return a positive value if the first item has higher priority than
// the second item, a negative value if the second item has higher priority than
// the first item, and 0 if the items have the same priority.
typedef struct ds_priority_queue {
        void **items;
        unsigned int count;
        unsigned int capacity;

        int (*compare)(const void *, const void *);
} ds_priority_queue;

DSHDEF void ds_priority_queue_init(ds_priority_queue *pq,
                                   int (*compare)(const void *, const void *));
DSHDEF int ds_priority_queue_insert(ds_priority_queue *pq, void *item);
DSHDEF int ds_priority_queue_pull(ds_priority_queue *pq, void **item);
DSHDEF int ds_priority_queue_peek(ds_priority_queue *pq, void **item);
DSHDEF int ds_priority_queue_empty(ds_priority_queue *pq);
DSHDEF void ds_priority_queue_free(ds_priority_queue *pq);

// STRING BUILDER
//
// The string builder is a simple utility to build strings. You can append
// formatted strings to the string builder, and then build the final string.
// The string builder will automatically grow as needed.
typedef struct ds_string_builder {
        char *items;
        unsigned int count;
        unsigned int capacity;
} ds_string_builder;

DSHDEF void ds_string_builder_init(ds_string_builder *sb);
DSHDEF int ds_string_builder_append(ds_string_builder *sb, const char *str);
DSHDEF int ds_string_builder_appendn(ds_string_builder *sb, const char *str,
                                     unsigned int len);
DSHDEF int ds_string_builder_appendc(ds_string_builder *sb, char chr);
DSHDEF int ds_string_builder_build(ds_string_builder *sb, char **str);
DSHDEF void ds_string_builder_free(ds_string_builder *sb);

// STRING SLICE
//
// The string slice is a simple utility to work with substrings. You can use the
// string slice to tokenize a string, and to convert a string slice to an owned
// string.
typedef struct ds_string_slice {
        char *str;
        unsigned int len;
} ds_string_slice;

DSHDEF void ds_string_slice_init(ds_string_slice *ss, char *str,
                                 unsigned int len);
DSHDEF int ds_string_slice_tokenize(ds_string_slice *ss, char delimiter,
                                    ds_string_slice *token);
DSHDEF int ds_string_slice_to_owned(ds_string_slice *ss, char **str);
DSHDEF void ds_string_slice_free(ds_string_slice *ss);

// DYNAMIC ARRAY
//
// The dynamic array is a simple array that grows as needed. This is the real
// implementation of dynamic arrays. The macros from the header file are just
// quick inline versions of these functions. This implementation is generic and
// can be used with any type of item, unlike the macros which require you to
// define the array structure with the items, count and capacity fields.
typedef struct ds_dynamic_array {
        void *items;
        unsigned int item_size;
        unsigned int count;
        unsigned int capacity;
} ds_dynamic_array;

DSHDEF void ds_dynamic_array_init(ds_dynamic_array *da, unsigned int item_size);
DSHDEF int ds_dynamic_array_append(ds_dynamic_array *da, const void *item);
DSHDEF int ds_dynamic_array_append_many(ds_dynamic_array *da, void **new_items,
                                        unsigned int new_items_count);
DSHDEF int ds_dynamic_array_get(ds_dynamic_array *da, unsigned int index,
                                void *item);
DSHDEF void ds_dynamic_array_get_ref(ds_dynamic_array *da, unsigned int index,
                                     void **item);
DSHDEF void ds_dynamic_array_free(ds_dynamic_array *da);

// (DOUBLY) LINKED LIST
//
// The linked list is a simple list that can be used to push and pop items from
// the front and back of the list.
typedef struct ds_linked_list_node ds_linked_list_node;

typedef struct ds_linked_list {
        unsigned int item_size;
        ds_linked_list_node *head;
        ds_linked_list_node *tail;
} ds_linked_list;

DSHDEF void ds_linked_list_init(ds_linked_list *ll, unsigned int item_size);
DSHDEF int ds_linked_list_push_back(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_push_front(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_pop_back(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_pop_front(ds_linked_list *ll, void *item);
DSHDEF void ds_linked_list_free(ds_linked_list *ll);

// HASH TABLE
typedef struct ds_hash_table {
        ds_dynamic_array *keys;
        ds_dynamic_array *values;
        unsigned int key_size;
        unsigned int value_size;
        unsigned int capacity;
        unsigned int (*hash)(const void *);
        int (*compare)(const void *, const void *);
} ds_hash_table;

DSHDEF int ds_hash_table_init(ds_hash_table *ht, unsigned int key_size,
                              unsigned int value_size, unsigned int capacity,
                              unsigned int (*hash)(const void *),
                              int (*compare)(const void *, const void *));
DSHDEF int ds_hash_table_insert(ds_hash_table *ht, const void *key,
                                void *value);
DSHDEF int ds_hash_table_has(ds_hash_table *ht, const void *key);
DSHDEF int ds_hash_table_get(ds_hash_table *ht, const void *key, void *value);
DSHDEF int ds_hash_table_get_ref(ds_hash_table *ht, const void *key,
                                 void **value);
DSHDEF unsigned int ds_hash_table_count(ds_hash_table *ht);
DSHDEF int ds_hash_table_remove(ds_hash_table *ht, const void *key);
DSHDEF void ds_hash_table_free(ds_hash_table *ht);

#endif // DS_H

#ifdef DS_IMPLEMENTATION
#define DS_PQ_IMPLEMENTATION
#define DS_SB_IMPLEMENTATION
#define DS_SS_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#define DS_LL_IMPLEMENTATION
#define DS_HT_IMPLEMENTATION
#endif // DS_IMPLEMENTATION

#ifdef DS_HT_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#endif // DS_HT_IMPLEMENTATION

#ifdef DS_PQ_IMPLEMENTATION

// Initialize the priority queue
DSHDEF void ds_priority_queue_init(ds_priority_queue *pq,
                                   int (*compare)(const void *, const void *)) {
    pq->items = NULL;
    pq->count = 0;
    pq->capacity = 0;
    pq->compare = compare;
}

// Insert an item into the priority queue
//
// Returns 0 if the item was inserted successfully.
DSHDEF int ds_priority_queue_insert(ds_priority_queue *pq, void *item) {
    ds_da_append(pq, item);

    int index = pq->count - 1;
    int parent = (index - 1) / 2;

    while (index != 0 && pq->compare(pq->items[index], pq->items[parent]) > 0) {
        void *temp = pq->items[index];
        pq->items[index] = pq->items[parent];
        pq->items[parent] = temp;

        index = parent;
        parent = (index - 1) / 2;
    }

    return 0;
}

// Pull the item with the highest priority from the priority queue
//
// Returns 0 if an item was pulled successfully, 1 if the priority queue is
// empty.
DSHDEF int ds_priority_queue_pull(ds_priority_queue *pq, void **item) {
    int result = 0;

    if (pq->count == 0) {
        DS_LOG_ERROR("Priority queue is empty");
        *item = NULL;
        return_defer(1);
    }

    *item = pq->items[0];
    pq->items[0] = pq->items[pq->count - 1];

    unsigned int index = 0;
    unsigned int swap = index;
    do {
        index = swap;

        unsigned int left = 2 * index + 1;
        if (left < pq->count &&
            pq->compare(pq->items[left], pq->items[swap]) > 0) {
            swap = left;
        }

        unsigned int right = 2 * index + 2;
        if (right < pq->count &&
            pq->compare(pq->items[right], pq->items[swap]) > 0) {
            swap = right;
        }

        void *temp = pq->items[index];
        pq->items[index] = pq->items[swap];
        pq->items[swap] = temp;
    } while (swap != index);

    pq->count--;
defer:
    return result;
}

// Peek at the item with the highest priority in the priority queue
//
// Returns 0 if an item was peeked successfully, 1 if the priority queue is
// empty.
DSHDEF int ds_priority_queue_peek(ds_priority_queue *pq, void **item) {
    int result = 0;

    if (pq->count == 0) {
        DS_LOG_ERROR("Priority queue is empty");
        *item = NULL;
        return_defer(1);
    }

    *item = pq->items[0];

defer:
    return result;
}

// Check if the priority queue is empty
DSHDEF int ds_priority_queue_empty(ds_priority_queue *pq) {
    return pq->count == 0;
}

// Free the priority queue
DSHDEF void ds_priority_queue_free(ds_priority_queue *pq) {
    DS_FREE(pq->items);
    pq->items = NULL;
    pq->count = 0;
    pq->capacity = 0;
    pq->compare = NULL;
}

#endif // DS_PQ_IMPLEMENTATION

#ifdef DS_SB_IMPLEMENTATION

// Initialize the string builder
DSHDEF void ds_string_builder_init(ds_string_builder *sb) {
    sb->items = NULL;
    sb->count = 0;
    sb->capacity = 0;
}

// Append a formatted string to the string builder
//
// Returns 0 if the string was appended successfully.
DSHDEF int ds_string_builder_appendn(ds_string_builder *sb, const char *str,
                                     unsigned int len) {
    ds_da_append_many(sb, str, len);
    return 0;
}

// Append a formatted string to the string builder
//
// Returns 0 if the string was appended successfully.
DSHDEF int ds_string_builder_append(ds_string_builder *sb, const char *str) {
    unsigned int str_len = strlen(str);
    ds_da_append_many(sb, str, str_len);
    return 0;
}

// Append a character to the string builder
//
// Returns 0 if the character was appended successfully.
DSHDEF int ds_string_builder_appendc(ds_string_builder *sb, char chr) {
    ds_da_append(sb, chr);
    return 0;
}

// Build the final string from the string builder
//
// Returns 0 if the string was built successfully, 1 if the string could not be
// allocated.
DSHDEF int ds_string_builder_build(ds_string_builder *sb, char **str) {
    int result = 0;

    *str = DS_MALLOC(sb->count + 1);
    if (*str == NULL) {
        DS_LOG_ERROR("Failed to allocate string");
        return_defer(1);
    }

    DS_MEMCPY(*str, sb->items, sb->count);
    (*str)[sb->count] = '\0';

defer:
    return result;
}

// Free the string builder
DSHDEF void ds_string_builder_free(ds_string_builder *sb) {
    if (sb->items != NULL) {
        DS_FREE(sb->items);
    }
    sb->items = NULL;
    sb->count = 0;
    sb->capacity = 0;
}

#endif // DS_SB_IMPLEMENTATION

#ifdef DS_SS_IMPLEMENTATION

// Initialize the string slice
DSHDEF void ds_string_slice_init(ds_string_slice *ss, char *str,
                                 unsigned int len) {
    ss->str = str;
    ss->len = len;
}

// Tokenize the string slice by a delimiter
//
// Returns 0 if a token was found, 1 if the string slice is empty.
DSHDEF int ds_string_slice_tokenize(ds_string_slice *ss, char delimiter,
                                    ds_string_slice *token) {
    int result = 0;

    if (ss->len == 0 || ss->str == NULL) {
        return_defer(1);
    }

    token->str = ss->str;
    token->len = 0;

    for (unsigned int i = 0; i < ss->len; i++) {
        if (ss->str[i] == delimiter) {
            token->len = i;
            ss->str += i + 1;
            ss->len -= i + 1;
            return_defer(0);
        }
    }

    token->len = ss->len;
    ss->str += ss->len;
    ss->len = 0;

defer:
    return result;
}

// Convert the string slice to an owned string
//
// Returns 0 if the string was converted successfully, 1 if the string could not
// be allocated.
DSHDEF int ds_string_slice_to_owned(ds_string_slice *ss, char **str) {
    int result = 0;

    *str = DS_MALLOC(ss->len + 1);
    if (*str == NULL) {
        DS_LOG_ERROR("Failed to allocate string");
        return_defer(1);
    }

    DS_MEMCPY(*str, ss->str, ss->len);
    (*str)[ss->len] = '\0';

defer:
    return result;
}

// Free the string slice
DSHDEF void ds_string_slice_free(ds_string_slice *ss) {
    ss->str = NULL;
    ss->len = 0;
}

#endif // DS_SS_IMPLEMENTATION

#ifdef DS_DA_IMPLEMENTATION

// Initialize the dynamic array
//
// The item_size parameter is the size of each item in the array.
DSHDEF void ds_dynamic_array_init(ds_dynamic_array *da,
                                  unsigned int item_size) {
    da->items = NULL;
    da->item_size = item_size;
    da->count = 0;
    da->capacity = 0;
}

// Append an item to the dynamic array
//
// Returns 0 if the item was appended successfully, 1 if the array could not be
// reallocated.
DSHDEF int ds_dynamic_array_append(ds_dynamic_array *da, const void *item) {
    int result = 0;

    if (da->count >= da->capacity) {
        unsigned int new_capacity = da->capacity * 2;
        if (new_capacity == 0) {
            new_capacity = DS_DA_INIT_CAPACITY;
        }

        da->items = DS_REALLOC(da->items, da->capacity * da->item_size,
                               new_capacity * da->item_size);

        if (da->items == NULL) {
            DS_LOG_ERROR("Failed to reallocate dynamic array");
            return_defer(1);
        }

        da->capacity = new_capacity;
    }

    DS_MEMCPY((char *)da->items + da->count * da->item_size, item,
              da->item_size);
    da->count++;

defer:
    return result;
}

// Append multiple items to the dynamic array
//
// Returns 0 if the items were appended successfully, 1 if the array could not
// be reallocated.
DSHDEF int ds_dynamic_array_append_many(ds_dynamic_array *da, void **new_items,
                                        unsigned int new_items_count) {
    int result = 0;

    if (da->count + new_items_count > da->capacity) {
        if (da->capacity == 0) {
            da->capacity = DS_DA_INIT_CAPACITY;
        }
        while (da->count + new_items_count > da->capacity) {
            da->capacity *= 2;
        }

        da->items = DS_REALLOC(da->items, da->capacity * da->item_size,
                               da->capacity * da->item_size);
        if (da->items == NULL) {
            DS_LOG_ERROR("Failed to reallocate dynamic array");
            return_defer(1);
        }
    }

    DS_MEMCPY((char *)da->items + da->count * da->item_size, new_items,
              new_items_count * da->item_size);
    da->count += new_items_count;

defer:
    return result;
}

// Get an item from the dynamic array
//
// Returns 0 if the item was retrieved successfully, 1 if the index is out of
// bounds.
DSHDEF int ds_dynamic_array_get(ds_dynamic_array *da, unsigned int index,
                                void *item) {
    int result = 0;

    if (index >= da->count) {
        DS_LOG_ERROR("Index out of bounds");
        return_defer(1);
    }

    DS_MEMCPY(item, (char *)da->items + index * da->item_size, da->item_size);

defer:
    return result;
}

DSHDEF void ds_dynamic_array_get_ref(ds_dynamic_array *da, unsigned int index,
                                     void **item) {
    *item = (char *)da->items + index * da->item_size;
}

DSHDEF void ds_dynamic_array_free(ds_dynamic_array *da) {
    if (da->items != NULL) {
        DS_FREE(da->items);
    }
    da->items = NULL;
    da->count = 0;
    da->capacity = 0;
}

#endif // DS_DA_IMPLEMENTATION

#ifdef DS_LL_IMPLEMENTATION

typedef struct ds_linked_list_node {
        void *item;
        struct ds_linked_list_node *prev;
        struct ds_linked_list_node *next;
} ds_linked_list_node;

// Initialize the linked list
//
// The item_size parameter is the size of each item in the list.
DSHDEF void ds_linked_list_init(ds_linked_list *ll, unsigned int item_size) {
    ll->item_size = item_size;
    ll->head = NULL;
    ll->tail = NULL;
}

// Push an item to the back of the linked list
//
// Returns 0 if the item was pushed successfully, 1 if the list could not be
// allocated.
DSHDEF int ds_linked_list_push_back(ds_linked_list *ll, void *item) {
    int result = 0;

    ds_linked_list_node *node = DS_MALLOC(sizeof(ds_linked_list_node));
    if (node == NULL) {
        DS_LOG_ERROR("Failed to allocate linked list node");
        return_defer(1);
    }

    node->item = DS_MALLOC(ll->item_size);
    if (node->item == NULL) {
        DS_LOG_ERROR("Failed to allocate linked list item");
        return_defer(1);
    }

    DS_MEMCPY(node->item, item, ll->item_size);
    node->prev = ll->tail;
    node->next = NULL;

    if (ll->tail != NULL) {
        ll->tail->next = node;
    }
    ll->tail = node;

    if (ll->head == NULL) {
        ll->head = node;
    }

defer:
    if (result != 0 && node != NULL) {
        if (node->item != NULL) {
            DS_FREE(node->item);
        }
        DS_FREE(node);
    }
    return result;
}

// Push an item to the front of the linked list
//
// Returns 0 if the item was pushed successfully, 1 if the list could not be
// allocated.
DSHDEF int ds_linked_list_push_front(ds_linked_list *ll, void *item) {
    int result = 0;

    ds_linked_list_node *node = DS_MALLOC(sizeof(ds_linked_list_node));
    if (node == NULL) {
        DS_LOG_ERROR("Failed to allocate linked list node");
        return_defer(1);
    }

    node->item = DS_MALLOC(ll->item_size);
    if (node->item == NULL) {
        DS_LOG_ERROR("Failed to allocate linked list item");
        return_defer(1);
    }

    DS_MEMCPY(node->item, item, ll->item_size);
    node->prev = NULL;
    node->next = ll->head;

    if (ll->head != NULL) {
        ll->head->prev = node;
    }
    ll->head = node;

    if (ll->tail == NULL) {
        ll->tail = node;
    }

defer:
    if (result != 0 && node != NULL) {
        if (node->item != NULL) {
            DS_FREE(node->item);
        }
        DS_FREE(node);
    }
    return result;
}

// Pop an item from the back of the linked list
//
// Returns 0 if the item was popped successfully, 1 if the list is empty.
// The item is stored in the item parameter.
DSHDEF int ds_linked_list_pop_back(ds_linked_list *ll, void *item) {
    int result = 0;

    if (ll->tail == NULL) {
        DS_LOG_ERROR("Linked list is empty");
        return_defer(1);
    }

    ds_linked_list_node *node = ll->tail;
    DS_MEMCPY(item, node->item, ll->item_size);

    ll->tail = node->prev;
    if (ll->tail != NULL) {
        ll->tail->next = NULL;
    }

    if (node == ll->head) {
        ll->head = NULL;
    }

defer:
    if (node != NULL) {
        if (node->item != NULL) {
            DS_FREE(node->item);
        }
        DS_FREE(node);
    }
    return result;
}

// Pop an item from the front of the linked list
//
// Returns 0 if the item was popped successfully, 1 if the list is empty.
// The item is stored in the item parameter.
DSHDEF int ds_linked_list_pop_front(ds_linked_list *ll, void *item) {
    int result = 0;

    if (ll->head == NULL) {
        DS_LOG_ERROR("Linked list is empty");
        return_defer(1);
    }

    ds_linked_list_node *node = ll->head;
    DS_MEMCPY(item, node->item, ll->item_size);

    ll->head = node->next;
    if (ll->head != NULL) {
        ll->head->prev = NULL;
    }

    if (node == ll->tail) {
        ll->tail = NULL;
    }

defer:
    if (node != NULL) {
        if (node->item != NULL) {
            DS_FREE(node->item);
        }
        DS_FREE(node);
    }
    return result;
}

DSHDEF void ds_linked_list_free(ds_linked_list *ll) {
    ds_linked_list_node *node = ll->head;
    while (node != NULL) {
        ds_linked_list_node *next = node->next;
        if (node->item != NULL) {
            DS_FREE(node->item);
        }
        DS_FREE(node);
        node = next;
    }
    ll->head = NULL;
    ll->tail = NULL;
}

#endif // DS_ST_IMPLEMENTATION

#ifdef DS_HT_IMPLEMENTATION

DSHDEF int ds_hash_table_init(ds_hash_table *ht, unsigned int key_size,
                              unsigned int value_size, unsigned int capacity,
                              unsigned int (*hash)(const void *),
                              int (*compare)(const void *, const void *)) {
    int result = 0;

    ht->keys = DS_MALLOC(capacity * sizeof(ds_dynamic_array));
    if (ht->keys == NULL) {
        DS_LOG_ERROR("Failed to allocate hash table keys");
        return_defer(1);
    }

    ht->values = DS_MALLOC(capacity * sizeof(ds_dynamic_array));
    if (ht->values == NULL) {
        DS_LOG_ERROR("Failed to allocate hash table values");
        return_defer(1);
    }

    for (unsigned int i = 0; i < capacity; i++) {
        ds_dynamic_array_init(ht->keys + i, key_size);
        ds_dynamic_array_init(ht->values + i, value_size);
    }

    ht->key_size = key_size;
    ht->value_size = value_size;
    ht->capacity = capacity;

    ht->hash = hash;
    ht->compare = compare;

defer:
    if (result != 0) {
        if (ht->keys != NULL) {
            DS_FREE(ht->keys);
        }
        if (ht->values != NULL) {
            DS_FREE(ht->values);
        }
    }
    return result;
}

DSHDEF int ds_hash_table_insert(ds_hash_table *ht, const void *key,
                                void *value) {
    int result = 0;

    unsigned int index = ht->hash(key) % ht->capacity;

    ds_dynamic_array *keys = ht->keys + index;
    ds_dynamic_array *values = ht->values + index;

    for (unsigned int i = 0; i < keys->count; i++) {
        void *k = NULL;
        ds_dynamic_array_get_ref(keys, i, &k);

        if (ht->compare(k, key) == 0) {
            void *v = NULL;
            ds_dynamic_array_get_ref(values, i, &v);

            DS_MEMCPY(v, value, ht->value_size);
            return_defer(0);
        }
    }

    if (ds_dynamic_array_append(keys, key) != 0) {
        DS_LOG_ERROR("Failed to append key to hash table");
        return_defer(1);
    }

    if (ds_dynamic_array_append(values, value) != 0) {
        DS_LOG_ERROR("Failed to append value to hash table");
        return_defer(1);
    }

defer:
    return result;
}

DSHDEF int ds_hash_table_has(ds_hash_table *ht, const void *key) {
    unsigned int index = ht->hash(key) % ht->capacity;

    ds_dynamic_array *keys = ht->keys + index;

    for (unsigned int i = 0; i < keys->count; i++) {
        void *k = NULL;
        ds_dynamic_array_get_ref(keys, i, &k);

        if (ht->compare(k, key) == 0) {
            return 1;
        }
    }

    return 0;
}

DSHDEF int ds_hash_table_get(ds_hash_table *ht, const void *key, void *value) {
    int result = 0;

    unsigned int index = ht->hash(key) % ht->capacity;

    ds_dynamic_array *keys = ht->keys + index;
    ds_dynamic_array *values = ht->values + index;

    for (unsigned int i = 0; i < keys->count; i++) {
        void *k = NULL;
        ds_dynamic_array_get_ref(keys, i, &k);

        if (ht->compare(k, key) == 0) {
            ds_dynamic_array_get(values, i, value);
            return_defer(0);
        }
    }

    return_defer(1);

defer:
    return result;
}

DSHDEF int ds_hash_table_get_ref(ds_hash_table *ht, const void *key,
                                 void **value) {
    int result = 0;

    unsigned int index = ht->hash(key) % ht->capacity;

    ds_dynamic_array *keys = ht->keys + index;
    ds_dynamic_array *values = ht->values + index;

    for (unsigned int i = 0; i < keys->count; i++) {
        void *k = NULL;
        ds_dynamic_array_get_ref(keys, i, &k);

        if (ht->compare(k, key) == 0) {
            ds_dynamic_array_get_ref(values, i, value);
            return_defer(0);
        }
    }

    return_defer(1);

defer:
    return result;
}

DSHDEF unsigned int ds_hash_table_count(ds_hash_table *ht) {
    unsigned int count = 0;
    for (unsigned int i = 0; i < ht->capacity; i++) {
        count += (ht->keys + i)->count;
    }
    return count;
}

DSHDEF int ds_hash_table_remove(ds_hash_table *ht, const void *key) {
    DS_LOG_ERROR("Not implemented");
    return 1;
}

DSHDEF void ds_hash_table_free(ds_hash_table *ht) {
    for (unsigned int i = 0; i < ht->capacity; i++) {
        ds_dynamic_array_free(ht->keys + i);
        ds_dynamic_array_free(ht->values + i);
    }
    DS_FREE(ht->keys);
    DS_FREE(ht->values);
}

#endif // DS_HT_IMPLEMENTATION
