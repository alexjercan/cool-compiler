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
// implementation of the priority queue data structure
// - DS_SB_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the string builder utility
// - DS_SS_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the string slice utility
// - DS_DA_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the dynamic array data structure
// - DS_LL_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the linked list data structure
// - DS_HT_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the hash table data structure
// - DS_AL_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the allocator utility and set the allocator to use
// - DS_AP_IMPLEMENTATION: Define this macro in one source file to include the
// implementation of the ds_argument parser utility
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
// - DS_LOG_LEVEL: Sets the logging level to one of the following values:
//  - DS_LOG_LEVEL_DEBUG: Print debug, info, warn and error messages
//  - DS_LOG_LEVEL_INFO: Print info, warn and error messages
//  - DS_LOG_LEVEL_WARN: Print warn and error messages
//  - DS_LOG_LEVEL_ERROR: Print error messages
//  - DS_LOG_LEVEL_NONE: Print no messages
// The default logging level is DS_LOG_LEVEL_DEBUG
// - DS_NO_TERMINAL_COLORS: Disables the use of terminal colors in the log
// messages

#ifndef DS_H
#define DS_H

#include <stdarg.h>
#include <stdint.h>

// TODO: rework the hash table to actually work

#ifndef DSHDEF
#ifdef DSH_STATIC
#define DSHDEF static
#else
#define DSHDEF extern
#endif
#endif

// ALLOCATOR
//
// The allocator is a simple utility to allocate and free memory. You can define
// the allocator to use when allocating and freeing memory. This can be used in
// all the other data structures and utilities to use a custom allocator.
typedef struct ds_allocator {
        uint8_t *start;
        uint8_t *prev;
        uint8_t *top;
        uint64_t size;
} ds_allocator;

DSHDEF void ds_allocator_init(ds_allocator *allocator, uint8_t *start,
                              uint64_t size);
DSHDEF void ds_allocator_dump(ds_allocator *allocator);
DSHDEF void *ds_allocator_alloc(ds_allocator *allocator, uint64_t size);
DSHDEF void ds_allocator_free(ds_allocator *allocator, void *ptr);

// DYNAMIC ARRAY
//
// The dynamic array is a simple array that grows as needed. This is the real
// implementation of dynamic arrays. The macros from the header file are just
// quick inline versions of these functions. This implementation is generic and
// can be used with any type of item, unlike the macros which require you to
// define the array structure with the items, count and capacity fields.
typedef struct ds_dynamic_array {
        struct ds_allocator *allocator;
        void *items;
        unsigned int item_size;
        unsigned int count;
        unsigned int capacity;
} ds_dynamic_array;

DSHDEF void ds_dynamic_array_init_allocator(ds_dynamic_array *da,
                                            unsigned int item_size,
                                            struct ds_allocator *allocator);
DSHDEF void ds_dynamic_array_init(ds_dynamic_array *da, unsigned int item_size);
DSHDEF int ds_dynamic_array_append(ds_dynamic_array *da, const void *item);
DSHDEF int ds_dynamic_array_pop(ds_dynamic_array *da, const void **item);
DSHDEF int ds_dynamic_array_append_many(ds_dynamic_array *da, void **new_items,
                                        unsigned int new_items_count);
DSHDEF int ds_dynamic_array_get(ds_dynamic_array *da, unsigned int index,
                                void *item);
DSHDEF void ds_dynamic_array_get_ref(ds_dynamic_array *da, unsigned int index,
                                     void **item);
DSHDEF int ds_dynamic_array_copy(ds_dynamic_array *da, ds_dynamic_array *copy);
DSHDEF void ds_dynamic_array_sort(ds_dynamic_array *da,
                                  int (*compare)(const void *, const void *));
DSHDEF int ds_dynamic_array_reverse(ds_dynamic_array *da);
DSHDEF int ds_dynamic_array_swap(ds_dynamic_array *da, unsigned int index1,
                                 unsigned int index2);
DSHDEF void ds_dynamic_array_free(ds_dynamic_array *da);

// PRIORITY QUEUE
//
// The priority queue is implemented as a heap, where you can define the
// comparison function to use when inserting items. The comparison function
// should return a positive value if the first item has higher priority than
// the second item, a negative value if the second item has higher priority than
// the first item, and 0 if the items have the same priority.
typedef struct ds_priority_queue {
        ds_dynamic_array items;

        int (*compare)(const void *, const void *);
} ds_priority_queue;

DSHDEF void ds_priority_queue_init_allocator(
    ds_priority_queue *pq, int (*compare)(const void *, const void *),
    unsigned int item_size, struct ds_allocator *allocator);
DSHDEF void ds_priority_queue_init(ds_priority_queue *pq,
                                   int (*compare)(const void *, const void *),
                                   unsigned int item_size);
DSHDEF int ds_priority_queue_insert(ds_priority_queue *pq, void *item);
DSHDEF int ds_priority_queue_pull(ds_priority_queue *pq, void *item);
DSHDEF int ds_priority_queue_peek(ds_priority_queue *pq, void *item);
DSHDEF int ds_priority_queue_empty(ds_priority_queue *pq);
DSHDEF void ds_priority_queue_free(ds_priority_queue *pq);

// STRING BUILDER
//
// The string builder is a simple utility to build strings. You can append
// formatted strings to the string builder, and then build the final string.
// The string builder will automatically grow as needed.
typedef struct ds_string_builder {
        ds_dynamic_array items;
} ds_string_builder;

DSHDEF void ds_string_builder_init_allocator(ds_string_builder *sb,
                                             struct ds_allocator *allocator);
DSHDEF void ds_string_builder_init(ds_string_builder *sb);
DSHDEF int ds_string_builder_append(ds_string_builder *sb, const char *format,
                                    ...);
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
        struct ds_allocator *allocator;
        char *str;
        unsigned int len;
} ds_string_slice;

DSHDEF void ds_string_slice_init_allocator(ds_string_slice *ss, char *str,
                                           unsigned int len,
                                           struct ds_allocator *allocator);
DSHDEF void ds_string_slice_init(ds_string_slice *ss, char *str,
                                 unsigned int len);
DSHDEF int ds_string_slice_tokenize(ds_string_slice *ss, char delimiter,
                                    ds_string_slice *token);
DSHDEF int ds_string_slice_trim_left(ds_string_slice *ss, char chr);
DSHDEF int ds_string_slice_trim_right(ds_string_slice *ss, char chr);
DSHDEF int ds_string_slice_trim(ds_string_slice *ss, char chr);
DSHDEF int ds_string_slice_to_owned(ds_string_slice *ss, char **str);
DSHDEF void ds_string_slice_free(ds_string_slice *ss);

// (DOUBLY) LINKED LIST
//
// The linked list is a simple list that can be used to push and pop items from
// the front and back of the list.
typedef struct ds_linked_list_node {
        void *item;
        struct ds_linked_list_node *prev;
        struct ds_linked_list_node *next;
} ds_linked_list_node;

typedef struct ds_linked_list {
        struct ds_allocator *allocator;
        unsigned int item_size;
        ds_linked_list_node *head;
        ds_linked_list_node *tail;
} ds_linked_list;

DSHDEF void ds_linked_list_init_allocator(ds_linked_list *ll,
                                          unsigned int item_size,
                                          struct ds_allocator *allocator);
DSHDEF void ds_linked_list_init(ds_linked_list *ll, unsigned int item_size);
DSHDEF int ds_linked_list_push_back(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_push_front(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_pop_back(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_pop_front(ds_linked_list *ll, void *item);
DSHDEF int ds_linked_list_empty(ds_linked_list *ll);
DSHDEF void ds_linked_list_free(ds_linked_list *ll);

// HASH TABLE
//
// The hash table is a simple table that uses a hash function to store and
// retrieve items. The hash table uses separate chaining to handle collisions.
// You can define the hash and compare functions to use when inserting and
// retrieving items.
typedef struct ds_hash_table {
        struct ds_allocator *allocator;
        ds_dynamic_array *keys;
        ds_dynamic_array *values;
        unsigned int key_size;
        unsigned int value_size;
        unsigned int capacity;
        unsigned int (*hash)(const void *);
        int (*compare)(const void *, const void *);
} ds_hash_table;

DSHDEF int ds_hash_table_init_allocator(
    ds_hash_table *ht, unsigned int key_size, unsigned int value_size,
    unsigned int capacity, unsigned int (*hash)(const void *),
    int (*compare)(const void *, const void *), struct ds_allocator *allocator);
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

// ARGUMENT PARSER
//
// The ds_argument parser is a simple utility to parse command line arguments.
// You can define the options and arguments to parse, and then parse the command
// line arguments.
// Argument types
enum ds_argument_type {
    ARGUMENT_TYPE_VALUE,           // Argument with a value
    ARGUMENT_TYPE_FLAG,            // Flag ds_argument
    ARGUMENT_TYPE_POSITIONAL,      // Positional ds_argument
    ARGUMENT_TYPE_POSITIONAL_REST, // Positional ds_argument that consumes the
                                   // rest
    ARGUMENT_TYPE_VALUE_ARRAY,     // Argument with an array of values
};

// Argument options
typedef struct ds_argparse_options {
        char short_name;            // Short name of the ds_argument
        char *long_name;            // Long name of the ds_argument
        char *description;          // Description of the ds_argument
        enum ds_argument_type type; // Type of the ds_argument
        unsigned int required;      // Whether the ds_argument is required
} ds_argparse_options;

// Argument
typedef struct ds_argument {
        struct ds_argparse_options options;
        union {
                char *value;
                unsigned int flag;
                ds_dynamic_array values;
        };
} ds_argument;

typedef struct ds_argparse_parser {
        struct ds_allocator *allocator;
        char *name;
        char *description;
        char *version;
        ds_dynamic_array arguments; // ds_argument
} ds_argparse_parser;

DSHDEF void ds_argparse_parser_init_allocator(struct ds_argparse_parser *parser,
                                              char *name, char *description,
                                              char *version,
                                              struct ds_allocator *allocator);
DSHDEF void ds_argparse_parser_init(struct ds_argparse_parser *parser,
                                    char *name, char *description,
                                    char *version);
DSHDEF int ds_argparse_add_argument(struct ds_argparse_parser *parser,
                                    struct ds_argparse_options options);
DSHDEF int ds_argparse_parse(struct ds_argparse_parser *parser, int argc,
                             char **argv);
DSHDEF char *ds_argparse_get_value(struct ds_argparse_parser *parser,
                                   char *name);
DSHDEF unsigned int ds_argparse_get_flag(struct ds_argparse_parser *parser,
                                         char *name);
DSHDEF int ds_argparse_get_values(struct ds_argparse_parser *parser, char *name,
                                  ds_dynamic_array *values);
DSHDEF void ds_argparse_print_help(struct ds_argparse_parser *parser);
DSHDEF void ds_argparse_print_version(struct ds_argparse_parser *parser);
DSHDEF void ds_argparse_parser_free(struct ds_argparse_parser *parser);

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

#ifndef DS_NO_STDLIB
#include <stdlib.h>
#include <string.h>
#endif

#ifndef NULL
#define NULL 0
#endif

#if defined(DS_MEMCPY)
// ok
#elif !defined(DS_MEMCPY) && !defined(DS_NO_STDLIB)
#define DS_MEMCPY(dst, src, sz) memcpy(dst, src, sz)
#elif defined(DS_NO_STDLIB)
#define DS_MEMCPY(dst, src, sz)                                                \
    do {                                                                       \
        for (unsigned int i = 0; i < sz; i++) {                                \
            ((char *)dst)[i] = ((char *)src)[i];                               \
        }                                                                      \
    } while (0)
#endif

#if defined(DS_MEMCMP)
// ok
#elif !defined(DS_MEMCMP) && !defined(DS_NO_STDLIB)
#define DS_MEMCMP(ptr1, ptr2, sz) memcmp(ptr1, ptr2, sz)
#elif defined(DS_NO_STDLIB)
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

#if defined(DS_MALLOC) && defined(DS_FREE)
// ok
#elif !defined(DS_MALLOC) && !defined(DS_FREE) && !defined(DS_REALLOC) &&      \
    !defined(DS_NO_STDLIB) && !defined(DS_AL_IMPLEMENTATION)
#define DS_MALLOC(a, sz) malloc(sz)
#define DS_REALLOC(a, ptr, old_sz, new_sz) realloc(ptr, new_sz)
#define DS_FREE(a, ptr) free(ptr)
#elif !defined(DS_MALLOC) && !defined(DS_FREE) && !defined(DS_REALLOC) &&      \
    defined(DS_AL_IMPLEMENTATION)
#define DS_MALLOC(a, sz) ds_allocator_alloc(a, sz)
#define DS_FREE(a, ptr) ds_allocator_free(a, ptr)
#elif defined(DS_NO_STDLIB)
#error "Must define DS_MALLOC and DS_FREE when DS_NO_STDLIB is defined"
#elif defined(DS_REALLOC) && !defined(DS_MALLOC) && !defined(DS_FREE)
#error "Must define DS_MALLOC and DS_FREE when DS_REALLOC is defined"
#else
#error "Must define both DS_MALLOC and DS_FREE, or neither"
#endif

#ifndef DS_REALLOC
static inline void *ds_realloc(void *a, void *ptr, unsigned int old_sz,
                               unsigned int new_sz) {
    void *new_ptr = DS_MALLOC(a, new_sz);
    if (new_ptr == NULL) {
        DS_FREE(a, ptr);
        return NULL;
    }
    DS_MEMCPY(new_ptr, ptr, old_sz < new_sz ? old_sz : new_sz);
    DS_FREE(a, ptr);
    return new_ptr;
}
#define DS_REALLOC(a, ptr, old_sz, new_sz) ds_realloc(a, ptr, old_sz, new_sz)
#endif

#if defined(DS_EXIT)
// ok
#elif !defined(DS_EXIT) && !defined(DS_NO_STDLIB)
#define DS_EXIT(code) exit(code)
#elif defined(DS_NO_STDLIB)
#error "Must define DS_EXIT when DS_NO_STDLIB is defined"
#endif

#ifndef DS_NO_STDIO
#include <stdio.h>
#endif

// TODO: actually do something for fprintf
#if defined(DS_NO_STDIO) && !defined(fprintf)
#define fprintf(stream, format, ...) 0
#endif

// TODO: actually do something for vsnprintf
#if defined(DS_NO_STDIO) && !defined(vsnprintf)
#define vsnprintf(buffer, size, format, args) 0
#endif

#if defined(DS_NO_STDIO) && !defined(stderr)
#define stderr NULL
#endif

#if defined(DS_NO_STDIO) && !defined(stdout)
#define stdout NULL
#endif

#define DS_LOG_LEVEL_DEBUG 1
#define DS_LOG_LEVEL_INFO 10
#define DS_LOG_LEVEL_WARN 100
#define DS_LOG_LEVEL_ERROR 1000
#define DS_LOG_LEVEL_NONE 10000

#ifndef DS_LOG_LEVEL
#define DS_LOG_LEVEL DS_LOG_LEVEL_DEBUG
#endif

#ifdef DS_NO_TERMINAL_COLORS
#define DS_TERMINAL_RED ""
#define DS_TERMINAL_YELLOW ""
#define DS_TERMINAL_BLUE ""
#define DS_TERMINAL_RESET ""
#else
#define DS_TERMINAL_RED "\033[1;31m"
#define DS_TERMINAL_YELLOW "\033[1;33m"
#define DS_TERMINAL_BLUE "\033[1;34m"
#define DS_TERMINAL_RESET "\033[0m"
#endif

#if DS_LOG_LEVEL > DS_LOG_LEVEL_ERROR
#define DS_LOG_ERROR(format, ...)
#else
#define DS_LOG_ERROR(format, ...)                                              \
    fprintf(stderr,                                                            \
            DS_TERMINAL_RED "ERROR" DS_TERMINAL_RESET ": %s:%d: " format "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if DS_LOG_LEVEL > DS_LOG_LEVEL_WARN
#define DS_LOG_WARN(format, ...)
#else
#define DS_LOG_WARN(format, ...)                                               \
    fprintf(stdout,                                                            \
            DS_TERMINAL_YELLOW "WARN" DS_TERMINAL_RESET ": %s:%d: " format     \
                               "\n",                                           \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if DS_LOG_LEVEL > DS_LOG_LEVEL_INFO
#define DS_LOG_INFO(format, ...)
#else
#define DS_LOG_INFO(format, ...)                                               \
    fprintf(stderr,                                                            \
            DS_TERMINAL_BLUE "INFO" DS_TERMINAL_RESET ": %s:%d: " format "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if DS_LOG_LEVEL > DS_LOG_LEVEL_DEBUG
#define DS_LOG_DEBUG(format, ...)
#else
#define DS_LOG_DEBUG(format, ...)                                              \
    fprintf(stdout, "DEBUG: %s:%d: " format "\n", __FILE__, __LINE__,          \
            ##__VA_ARGS__)
#endif

#ifndef DS_PANIC
#define DS_PANIC(format, ...)                                                  \
    do {                                                                       \
        DS_LOG_ERROR(format, ##__VA_ARGS__);                                   \
        DS_EXIT(1);                                                            \
    } while (0)
#endif

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
            (da)->items = DS_REALLOC(NULL, (da)->items,                        \
                                     (da)->capacity * sizeof(*(da)->items),    \
                                     new_capacity * sizeof(*(da)->items));     \
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
            (da)->items = DS_REALLOC(NULL, (da)->items,                        \
                                     (da)->capacity * sizeof(*(da)->items),    \
                                     (da)->capacity * sizeof(*(da)->items));   \
            if ((da)->items == NULL) {                                         \
                DS_PANIC("Failed to reallocate dynamic array");                \
            }                                                                  \
        }                                                                      \
                                                                               \
        DS_MEMCPY((da)->items + (da)->count, new_items,                        \
                  new_items_count * sizeof(*(da)->items));                     \
        (da)->count += new_items_count;                                        \
    } while (0)

#endif // DS_H

#ifdef DS_IMPLEMENTATION
#define DS_PQ_IMPLEMENTATION
#define DS_SB_IMPLEMENTATION
#define DS_SS_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#define DS_LL_IMPLEMENTATION
#define DS_HT_IMPLEMENTATION
#define DS_AL_IMPLEMENTATION
#define DS_AP_IMPLEMENTATION
#endif // DS_IMPLEMENTATION

#ifdef DS_PQ_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#endif // DS_PQ_IMPLEMENTATION

#ifdef DS_SB_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#endif // DS_SB_IMPLEMENTATION

#ifdef DS_HT_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#endif // DS_HT_IMPLEMENTATION

#ifdef DS_AP_IMPLEMENTATION
#define DS_DA_IMPLEMENTATION
#endif // DS_AP_IMPLEMENTATION

#ifdef DS_PQ_IMPLEMENTATION

// Initialize the priority queue with a custom allocator
DSHDEF void ds_priority_queue_init_allocator(
    ds_priority_queue *pq, int (*compare)(const void *, const void *),
    unsigned int item_size, struct ds_allocator *allocator) {
    ds_dynamic_array_init_allocator(&pq->items, item_size, allocator);

    pq->compare = compare;
}

// Initialize the priority queue
DSHDEF void ds_priority_queue_init(ds_priority_queue *pq,
                                   int (*compare)(const void *, const void *),
                                   unsigned int item_size) {
    ds_priority_queue_init_allocator(pq, compare, item_size, NULL);
}

// Insert an item into the priority queue
//
// Returns 0 if the item was inserted successfully.
DSHDEF int ds_priority_queue_insert(ds_priority_queue *pq, void *item) {
    ds_dynamic_array_append(&pq->items, item);

    int index = pq->items.count - 1;
    int parent = (index - 1) / 2;

    void *index_item = NULL;
    ds_dynamic_array_get_ref(&pq->items, index, &index_item);

    void *parent_item = NULL;
    ds_dynamic_array_get_ref(&pq->items, parent, &parent_item);

    while (index != 0 && pq->compare(index_item, parent_item) > 0) {
        ds_dynamic_array_swap(&pq->items, index, parent);

        index = parent;
        parent = (index - 1) / 2;

        ds_dynamic_array_get_ref(&pq->items, index, &index_item);
        ds_dynamic_array_get_ref(&pq->items, parent, &parent_item);
    }

    return 0;
}

// Pull the item with the highest priority from the priority queue
//
// Returns 0 if an item was pulled successfully, 1 if the priority queue is
// empty.
DSHDEF int ds_priority_queue_pull(ds_priority_queue *pq, void *item) {
    int result = 0;

    if (pq->items.count == 0) {
        DS_LOG_ERROR("Priority queue is empty");
        return_defer(1);
    }

    ds_dynamic_array_get(&pq->items, 0, item);
    ds_dynamic_array_swap(&pq->items, 0, pq->items.count - 1);

    unsigned int index = 0;
    unsigned int swap = index;
    void *swap_item = NULL;
    do {
        index = swap;

        unsigned int left = 2 * index + 1;
        void *left_item = NULL;
        ds_dynamic_array_get_ref(&pq->items, swap, &swap_item);
        ds_dynamic_array_get_ref(&pq->items, left, &left_item);
        if (left < pq->items.count - 1 &&
            pq->compare(left_item, swap_item) > 0) {
            swap = left;
        }

        unsigned int right = 2 * index + 2;
        void *right_item = NULL;
        ds_dynamic_array_get_ref(&pq->items, swap, &swap_item);
        ds_dynamic_array_get_ref(&pq->items, right, &right_item);
        if (right < pq->items.count - 1 &&
            pq->compare(right_item, swap_item) > 0) {
            swap = right;
        }

        ds_dynamic_array_swap(&pq->items, index, swap);
    } while (swap != index);

    pq->items.count--;
defer:
    return result;
}

// Peek at the item with the highest priority in the priority queue
//
// Returns 0 if an item was peeked successfully, 1 if the priority queue is
// empty.
DSHDEF int ds_priority_queue_peek(ds_priority_queue *pq, void *item) {
    int result = 0;

    if (pq->items.count == 0) {
        DS_LOG_ERROR("Priority queue is empty");
        return_defer(1);
    }

    ds_dynamic_array_get(&pq->items, 0, item);

defer:
    return result;
}

// Check if the priority queue is empty
DSHDEF int ds_priority_queue_empty(ds_priority_queue *pq) {
    return pq->items.count == 0;
}

// Free the priority queue
DSHDEF void ds_priority_queue_free(ds_priority_queue *pq) {
    ds_dynamic_array_free(&pq->items);

    pq->compare = NULL;
}

#endif // DS_PQ_IMPLEMENTATION

#ifdef DS_SB_IMPLEMENTATION

DSHDEF void ds_string_builder_init_allocator(ds_string_builder *sb,
                                             struct ds_allocator *allocator) {
    ds_dynamic_array_init_allocator(&sb->items, sizeof(char), allocator);
}

// Initialize the string builder
DSHDEF void ds_string_builder_init(ds_string_builder *sb) {
    ds_string_builder_init_allocator(sb, NULL);
}

// Append a formatted string to the string builder
//
// Returns 0 if the string was appended successfully.
DSHDEF int ds_string_builder_appendn(ds_string_builder *sb, const char *str,
                                     unsigned int len) {
    return ds_dynamic_array_append_many(&sb->items, (void **)str, len);
}

// Append a formatted string to the string builder
//
// Returns 0 if the string was appended successfully.
DSHDEF int ds_string_builder_append(ds_string_builder *sb, const char *format,
                                    ...) {
    int result = 0;

    va_list args;
    va_start(args, format);
    int needed = vsnprintf(NULL, 0, format, args);
    va_end(args);

    char *buffer = DS_MALLOC(sb->items.allocator, needed + 1);
    if (buffer == NULL) {
        DS_LOG_ERROR("Failed to allocate string");
        return_defer(1);
    }

    va_start(args, format);
    vsnprintf(buffer, needed + 1, format, args);
    va_end(args);

    if (ds_dynamic_array_append_many(&sb->items, (void **)buffer, needed) !=
        0) {
        return_defer(1);
    }

defer:
    if (buffer != NULL) {
        DS_FREE(sb->items.allocator, buffer);
    }
    return result;
}

// Append a character to the string builder
//
// Returns 0 if the character was appended successfully.
DSHDEF int ds_string_builder_appendc(ds_string_builder *sb, char chr) {
    return ds_dynamic_array_append(&sb->items, &chr);
}

// Build the final string from the string builder
//
// Returns 0 if the string was built successfully, 1 if the string could not be
// allocated.
DSHDEF int ds_string_builder_build(ds_string_builder *sb, char **str) {
    int result = 0;

    *str = DS_MALLOC(NULL, sb->items.count + 1);
    if (*str == NULL) {
        DS_LOG_ERROR("Failed to allocate string");
        return_defer(1);
    }

    DS_MEMCPY(*str, (char *)sb->items.items, sb->items.count);
    (*str)[sb->items.count] = '\0';

defer:
    return result;
}

// Free the string builder
DSHDEF void ds_string_builder_free(ds_string_builder *sb) {
    ds_dynamic_array_free(&sb->items);
}

#endif // DS_SB_IMPLEMENTATION

#ifdef DS_SS_IMPLEMENTATION

DSHDEF void ds_string_slice_init_allocator(ds_string_slice *ss, char *str,
                                           unsigned int len,
                                           struct ds_allocator *allocator) {
    ss->allocator = allocator;
    ss->str = str;
    ss->len = len;
}

// Initialize the string slice
DSHDEF void ds_string_slice_init(ds_string_slice *ss, char *str,
                                 unsigned int len) {
    ds_string_slice_init_allocator(ss, str, len, NULL);
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

// Trim the left side of the string slice by a character
//
// Returns 0 if the string was trimmed successfully, 1 if the string slice is
DSHDEF int ds_string_slice_trim_left(ds_string_slice *ss, char chr) {
    int result = 0;

    while (ss->len > 0 && ss->str[0] == chr) {
        ss->str++;
        ss->len--;
    }

    return result;
}

// Trim the right side of the string slice by a character
//
// Returns 0 if the string was trimmed successfully, 1 if the string slice is
DSHDEF int ds_string_slice_trim_right(ds_string_slice *ss, char chr) {
    int result = 0;

    while (ss->len > 0 && ss->str[ss->len - 1] == chr) {
        ss->len--;
    }

    return result;
}

// Trim the string slice by a character
//
// Returns 0 if the string was trimmed successfully, 1 if the string slice is
DSHDEF int ds_string_slice_trim(ds_string_slice *ss, char chr) {
    int result = 0;

    if (ds_string_slice_trim_left(ss, chr) != 0) {
        return_defer(1);
    }

    if (ds_string_slice_trim_right(ss, chr) != 0) {
        return_defer(1);
    }

defer:
    return result;
}

// Convert the string slice to an owned string
//
// Returns 0 if the string was converted successfully, 1 if the string could not
// be allocated.
DSHDEF int ds_string_slice_to_owned(ds_string_slice *ss, char **str) {
    int result = 0;

    *str = DS_MALLOC(ss->allocator, ss->len + 1);
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

// Initialize the dynamic array with a custom allocator
//
// The item_size parameter is the size of each item in the array.
DSHDEF void ds_dynamic_array_init_allocator(ds_dynamic_array *da,
                                            unsigned int item_size,
                                            struct ds_allocator *allocator) {
    da->allocator = allocator;
    da->items = NULL;
    da->item_size = item_size;
    da->count = 0;
    da->capacity = 0;
}

// Initialize the dynamic array
//
// The item_size parameter is the size of each item in the array.
DSHDEF void ds_dynamic_array_init(ds_dynamic_array *da,
                                  unsigned int item_size) {
    ds_dynamic_array_init_allocator(da, item_size, NULL);
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

        da->items =
            DS_REALLOC(da->allocator, da->items, da->capacity * da->item_size,
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

// Pop an item from the dynamic array
//
// Returns 0 if the item was popped successfully, 1 if the array is empty.
// If the item is NULL, then we just pop the item without returning it.
DSHDEF int ds_dynamic_array_pop(ds_dynamic_array *da, const void **item) {
    int result = 0;

    if (da->count == 0) {
        DS_LOG_ERROR("Dynamic array is empty");
        *item = NULL;
        return_defer(1);
    }

    if (item != NULL) {
        *item = (char *)da->items + (da->count - 1) * da->item_size;
    }
    da->count--;

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

        da->items =
            DS_REALLOC(da->allocator, da->items, da->capacity * da->item_size,
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

// Get a reference to an item from the dynamic array
DSHDEF void ds_dynamic_array_get_ref(ds_dynamic_array *da, unsigned int index,
                                     void **item) {
    *item = (char *)da->items + index * da->item_size;
}

// Copy the dynamic array to another dynamic array
//
// Returns 0 if the array was copied successfully, 1 if the array could not be
// allocated.
DSHDEF int ds_dynamic_array_copy(ds_dynamic_array *da, ds_dynamic_array *copy) {
    int result = 0;

    copy->items = DS_MALLOC(da->allocator, da->capacity * da->item_size);
    if (copy->items == NULL) {
        DS_LOG_ERROR("Failed to allocate dynamic array items");
        return_defer(1);
    }

    copy->item_size = da->item_size;
    copy->count = da->count;
    copy->capacity = da->capacity;

    DS_MEMCPY(copy->items, da->items, da->count * da->item_size);

defer:
    return result;
}

DSHDEF void ds_dynamic_array_sort(ds_dynamic_array *da,
                                  int (*compare)(const void *, const void *)) {
    qsort(da->items, da->count, da->item_size, compare);
}

// Reverse the dynamic array
//
// Returns 0 if the array was reversed successfully, 1 if the array could not be
// allocated.
DSHDEF int ds_dynamic_array_reverse(ds_dynamic_array *da) {
    int result = 0;

    for (unsigned int i = 0; i < da->count / 2; i++) {
        unsigned int j = da->count - i - 1;

        void *temp = DS_MALLOC(da->allocator, da->item_size);
        if (temp == NULL) {
            DS_LOG_ERROR("Failed to allocate temporary item");
            return_defer(1);
        }

        DS_MEMCPY(temp, (char *)da->items + i * da->item_size, da->item_size);
        DS_MEMCPY((char *)da->items + i * da->item_size,
                  (char *)da->items + j * da->item_size, da->item_size);
        DS_MEMCPY((char *)da->items + j * da->item_size, temp, da->item_size);
        DS_FREE(da->allocator, temp);
    }

defer:
    return result;
}

// Swap two items in the dynamic array
//
// Returns 0 if the items were swapped successfully, 1 if the index is out of
// bounds or if the temporary item could not be allocated.
DSHDEF int ds_dynamic_array_swap(ds_dynamic_array *da, unsigned int index1,
                                 unsigned int index2) {
    int result = 0;

    if (index1 >= da->count || index2 >= da->count) {
        DS_LOG_ERROR("Index out of bounds");
        return_defer(1);
    }

    void *temp = DS_MALLOC(da->allocator, da->item_size);
    if (temp == NULL) {
        DS_LOG_ERROR("Failed to allocate temporary item");
        return_defer(1);
    }

    void *index1_item = NULL;
    ds_dynamic_array_get_ref(da, index1, &index1_item);

    void *index2_item = NULL;
    ds_dynamic_array_get_ref(da, index2, &index2_item);

    DS_MEMCPY(temp, index1_item, da->item_size);
    DS_MEMCPY(index1_item, index2_item, da->item_size);
    DS_MEMCPY(index2_item, temp, da->item_size);

    DS_FREE(da->allocator, temp);

defer:
    return result;
}

// Free the dynamic array
DSHDEF void ds_dynamic_array_free(ds_dynamic_array *da) {
    if (da->items != NULL) {
        DS_FREE(da->allocator, da->items);
    }
    da->items = NULL;
    da->count = 0;
    da->capacity = 0;
}

#endif // DS_DA_IMPLEMENTATION

#ifdef DS_LL_IMPLEMENTATION

// Initialize the linked list with a custom allocator
//
// The item_size parameter is the size of each item in the list.
DSHDEF void ds_linked_list_init_allocator(ds_linked_list *ll,
                                          unsigned int item_size,
                                          struct ds_allocator *allocator) {
    ll->allocator = allocator;
    ll->item_size = item_size;
    ll->head = NULL;
    ll->tail = NULL;
}

// Initialize the linked list
//
// The item_size parameter is the size of each item in the list.
DSHDEF void ds_linked_list_init(ds_linked_list *ll, unsigned int item_size) {
    ds_linked_list_init_allocator(ll, item_size, NULL);
}

// Push an item to the back of the linked list
//
// Returns 0 if the item was pushed successfully, 1 if the list could not be
// allocated.
DSHDEF int ds_linked_list_push_back(ds_linked_list *ll, void *item) {
    int result = 0;

    ds_linked_list_node *node =
        DS_MALLOC(ll->allocator, sizeof(ds_linked_list_node));
    if (node == NULL) {
        DS_LOG_ERROR("Failed to allocate linked list node");
        return_defer(1);
    }

    node->item = DS_MALLOC(ll->allocator, ll->item_size);
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
            DS_FREE(ll->allocator, node->item);
        }
        DS_FREE(ll->allocator, node);
    }
    return result;
}

// Push an item to the front of the linked list
//
// Returns 0 if the item was pushed successfully, 1 if the list could not be
// allocated.
DSHDEF int ds_linked_list_push_front(ds_linked_list *ll, void *item) {
    int result = 0;

    ds_linked_list_node *node =
        DS_MALLOC(ll->allocator, sizeof(ds_linked_list_node));
    if (node == NULL) {
        DS_LOG_ERROR("Failed to allocate linked list node");
        return_defer(1);
    }

    node->item = DS_MALLOC(ll->allocator, ll->item_size);
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
            DS_FREE(ll->allocator, node->item);
        }
        DS_FREE(ll->allocator, node);
    }
    return result;
}

// Pop an item from the back of the linked list
//
// Returns 0 if the item was popped successfully, 1 if the list is empty.
// The item is stored in the item parameter.
DSHDEF int ds_linked_list_pop_back(ds_linked_list *ll, void *item) {
    ds_linked_list_node *node = NULL;
    int result = 0;

    if (ll->tail == NULL) {
        DS_LOG_ERROR("Linked list is empty");
        return_defer(1);
    }

    node = ll->tail;
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
            DS_FREE(ll->allocator, node->item);
        }
        DS_FREE(ll->allocator, node);
    }
    return result;
}

// Pop an item from the front of the linked list
//
// Returns 0 if the item was popped successfully, 1 if the list is empty.
// The item is stored in the item parameter.
DSHDEF int ds_linked_list_pop_front(ds_linked_list *ll, void *item) {
    ds_linked_list_node *node = NULL;
    int result = 0;

    if (ll->head == NULL) {
        DS_LOG_ERROR("Linked list is empty");
        return_defer(1);
    }

    node = ll->head;
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
            DS_FREE(ll->allocator, node->item);
        }
        DS_FREE(ll->allocator, node);
    }
    return result;
}

// Check if the linked list is empty
//
// Returns 1 if the list is empty, 0 if the list is not empty.
DSHDEF int ds_linked_list_empty(ds_linked_list *ll) { return ll->head == NULL; }

DSHDEF void ds_linked_list_free(ds_linked_list *ll) {
    ds_linked_list_node *node = ll->head;
    while (node != NULL) {
        ds_linked_list_node *next = node->next;
        if (node->item != NULL) {
            DS_FREE(ll->allocator, node->item);
        }
        DS_FREE(ll->allocator, node);
        node = next;
    }
    ll->head = NULL;
    ll->tail = NULL;
}

#endif // DS_ST_IMPLEMENTATION

#ifdef DS_HT_IMPLEMENTATION

// Initialize the hash table with a custom allocator
DSHDEF int
ds_hash_table_init_allocator(ds_hash_table *ht, unsigned int key_size,
                             unsigned int value_size, unsigned int capacity,
                             unsigned int (*hash)(const void *),
                             int (*compare)(const void *, const void *),
                             struct ds_allocator *allocator) {
    int result = 0;

    ht->allocator = allocator;

    ht->keys = DS_MALLOC(ht->allocator, capacity * sizeof(ds_dynamic_array));
    if (ht->keys == NULL) {
        DS_LOG_ERROR("Failed to allocate hash table keys");
        return_defer(1);
    }

    ht->values = DS_MALLOC(ht->allocator, capacity * sizeof(ds_dynamic_array));
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
            DS_FREE(NULL, ht->keys);
        }
        if (ht->values != NULL) {
            DS_FREE(NULL, ht->values);
        }
    }
    return result;
}

// Initialize the hash table
//
// The key_size and value_size parameters are the size of each key and value in
// the table. The capacity parameter is the initial capacity of the table. The
// hash and compare parameters are the hash and compare functions to use when
// inserting and retrieving items.
DSHDEF int ds_hash_table_init(ds_hash_table *ht, unsigned int key_size,
                              unsigned int value_size, unsigned int capacity,
                              unsigned int (*hash)(const void *),
                              int (*compare)(const void *, const void *)) {
    return ds_hash_table_init_allocator(ht, key_size, value_size, capacity,
                                        hash, compare, NULL);
}

// Insert an item into the hash table
//
// Returns 0 if the item was inserted successfully, 1 if the item could not be
// inserted.
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

// Check if the hash table has an item with the given key
//
// Returns 1 if the item was found, 0 if the item was not found.
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

// Get an item from the hash table
//
// Returns 0 if the item was retrieved successfully, 1 if the item was not
// found.
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

// Get a reference to an item from the hash table
//
// Returns 0 if the item was retrieved successfully, 1 if the item was not
// found.
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

// Get the number of items in the hash table
//
// Returns the number of items in the hash table.
DSHDEF unsigned int ds_hash_table_count(ds_hash_table *ht) {
    unsigned int count = 0;
    for (unsigned int i = 0; i < ht->capacity; i++) {
        count += (ht->keys + i)->count;
    }
    return count;
}

// Remove an item from the hash table
//
// Returns 0 if the item was removed successfully, 1 if the item was not found.
DSHDEF int ds_hash_table_remove(ds_hash_table *ht, const void *key) {
    (void)ht;
    (void)key;
    DS_LOG_ERROR("Not implemented");
    return 1;
}

// Free the hash table
//
// This function frees all the memory used by the hash table.
DSHDEF void ds_hash_table_free(ds_hash_table *ht) {
    for (unsigned int i = 0; i < ht->capacity; i++) {
        ds_dynamic_array_free(ht->keys + i);
        ds_dynamic_array_free(ht->values + i);
    }
    DS_FREE(NULL, ht->keys);
    DS_FREE(NULL, ht->values);
}

#endif // DS_HT_IMPLEMENTATION

#ifdef DS_AL_IMPLEMENTATION

static void uint64_read_le(uint8_t *data, uint64_t *value) {
    *value = ((uint64_t)data[0] << 0) | ((uint64_t)data[1] << 8) |
             ((uint64_t)data[2] << 16) | ((uint64_t)data[3] << 24) |
             ((uint64_t)data[4] << 32) | ((uint64_t)data[5] << 40) |
             ((uint64_t)data[6] << 48) | ((uint64_t)data[7] << 56);
}

static void uint64_write_le(uint8_t *data, uint64_t value) {
    data[0] = (value >> 0) & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[3] = (value >> 24) & 0xff;
    data[4] = (value >> 32) & 0xff;
    data[5] = (value >> 40) & 0xff;
    data[6] = (value >> 48) & 0xff;
    data[7] = (value >> 56) & 0xff;
}

static void uint32_read_le(uint8_t *data, uint32_t *value) {
    *value = ((uint32_t)data[0] << 0) | ((uint32_t)data[1] << 8) |
             ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static void uint32_write_le(uint8_t *data, uint32_t value) {
    data[0] = (value >> 0) & 0xff;
    data[1] = (value >> 8) & 0xff;
    data[2] = (value >> 16) & 0xff;
    data[3] = (value >> 24) & 0xff;
}

#define BLOCK_METADATA_SIZE 28
#define BLOCK_INDEX_UNDEFINED -1

/*
 * | prev | next | size | free | ... size bytes of data ... |
 */
typedef struct block {
        int64_t prev;  // 8 bytes
        int64_t next;  // 8 bytes
        uint64_t size; // 8 bytes
        uint32_t free; // 4 bytes
        uint8_t *data; // 8 bytes
} block_t;

static void block_read(uint8_t *data, block_t *block) {
    uint64_read_le(data + 0, (uint64_t *)&block->prev);
    uint64_read_le(data + 8, (uint64_t *)&block->next);
    uint64_read_le(data + 16, &block->size);
    uint32_read_le(data + 24, &block->free);
    block->data = data + BLOCK_METADATA_SIZE;
}

static void block_write(uint8_t *data, block_t *block) {
    uint64_write_le(data + 0, block->prev);
    uint64_write_le(data + 8, block->next);
    uint64_write_le(data + 16, block->size);
    uint32_write_le(data + 24, block->free);
}

// Initialize the allocator
//
// The start parameter is the start of the memory block to allocate from, and
// the size parameter is the maximum size of the memory allocator.
DSHDEF void ds_allocator_init(ds_allocator *allocator, uint8_t *start,
                              uint64_t size) {
    allocator->start = start;
    allocator->prev = NULL;
    allocator->top = start;
    allocator->size = size;
}

// Dump the allocator to stdout
//
// This function prints the contents of the allocator to stdout.
DSHDEF void ds_allocator_dump(ds_allocator *allocator) {
    block_t block = {0};
    uint8_t *ptr = allocator->start;

    fprintf(stdout, "%*s %*s %*s %*s %*s\n", 14, "", 14, "prev", 14, "next", 14,
            "size", 14, "free");

    while (ptr < allocator->top) {
        block_read(ptr, &block);

        uint8_t *prev = (block.prev == BLOCK_INDEX_UNDEFINED)
                            ? NULL
                            : allocator->start + block.prev;
        uint8_t *next = (block.next == BLOCK_INDEX_UNDEFINED)
                            ? NULL
                            : allocator->start + block.next;

        fprintf(stdout, "%*p %*p %*p %*lu %*u\n", 14, ptr, 14, prev, 14, next,
                14, block.size, 14, block.free);

        ptr += (block.size + BLOCK_METADATA_SIZE);
    }
}

static int allocator_find_block(ds_allocator *allocator, uint64_t size,
                                block_t *block) {
    if (allocator->prev == NULL) {
        return 0;
    }

    block_t current = {0};
    uint8_t *ptr = allocator->start;

    while (ptr < allocator->top) {
        block_read(ptr, &current);

        if (current.free && current.size >= size + BLOCK_METADATA_SIZE * 2) {
            uint64_t old_size = current.size;
            int64_t old_next = current.next;

            block_t split = {0};
            split.prev = (uint64_t)(ptr - allocator->start);
            split.next = old_next;
            split.size = old_size - size - BLOCK_METADATA_SIZE;
            split.free = 1;
            split.data = ptr + BLOCK_METADATA_SIZE + size + BLOCK_METADATA_SIZE;

            block_write(ptr + BLOCK_METADATA_SIZE + size, &split);

            *block = current;
            block->next =
                (uint64_t)(ptr - allocator->start) + BLOCK_METADATA_SIZE + size;
            block->size = size;
            block->free = 0;
            block->data = ptr + BLOCK_METADATA_SIZE;

            block_write(ptr, block);

            block_t next = {0};
            block_read(allocator->start + old_next, &next);

            next.prev =
                (uint64_t)(ptr - allocator->start) + BLOCK_METADATA_SIZE + size;

            block_write(allocator->start + old_next, &next);

            return 1;
        }

        if (current.free && current.size >= size) {
            *block = current;
            block->free = 0;

            block_write(ptr, block);

            return 1;
        }

        ptr += (current.size + BLOCK_METADATA_SIZE);
    }

    return 0;
}

// Allocate memory from the allocator
//
// This function allocates memory from the allocator. If the allocator is unable
// to allocate the memory, it returns NULL.
DSHDEF void *ds_allocator_alloc(ds_allocator *allocator, uint64_t size) {
    block_t block = {0};
    if (allocator_find_block(allocator, size, &block) != 0) {
        return block.data;
    }

    if (allocator->top + size + BLOCK_METADATA_SIZE >
        allocator->start + allocator->size) {
        return NULL;
    }

    block.next = BLOCK_INDEX_UNDEFINED;
    block.size = size;
    block.free = 0;
    block.data = allocator->top + BLOCK_METADATA_SIZE;

    if (allocator->prev == NULL) {
        block.prev = BLOCK_INDEX_UNDEFINED;
    } else {
        block.prev = (uint64_t)(allocator->prev - allocator->start);

        block_t prev = {0};
        block_read(allocator->prev, &prev);
        prev.next = (uint64_t)(allocator->top - allocator->start);

        block_write(allocator->prev, &prev);
    }

    block_write(allocator->top, &block);

    allocator->prev = allocator->top;
    allocator->top += size + BLOCK_METADATA_SIZE;

    return block.data;
}

// Free memory from the allocator
//
// This function frees memory from the allocator. If the pointer is not within
// the bounds of the allocator, it does nothing.
DSHDEF void ds_allocator_free(ds_allocator *allocator, void *ptr) {
    if ((uint8_t *)ptr > allocator->top || (uint8_t *)ptr < allocator->start) {
        return;
    }

    block_t block = {0};
    block_read(ptr - BLOCK_METADATA_SIZE, &block);
    block.free = 1;

    if (block.prev != BLOCK_INDEX_UNDEFINED) {
        block_t prev = {0};
        block_read(allocator->start + block.prev, &prev);

        if (prev.free) {
            prev.next = block.next;
            prev.size += block.size + BLOCK_METADATA_SIZE;

            uint8_t *mptr = allocator->start + block.prev;

            block_t next = {0};
            block_read(allocator->start + block.next, &next);

            next.prev = (uint64_t)((uint8_t *)mptr - allocator->start);

            block_write(allocator->start + block.next, &next);
            block_write(allocator->start + block.prev, &prev);

            block = prev;
            ptr = mptr + BLOCK_METADATA_SIZE;
        }
    }

    if (block.next != BLOCK_INDEX_UNDEFINED) {
        block_t next = {0};
        block_read(allocator->start + block.next, &next);

        if (next.free) {
            block_t next_next = {0};
            block_read(allocator->start + next.next, &next_next);

            uint8_t *mptr = ptr - BLOCK_METADATA_SIZE;

            next_next.prev = (uint64_t)((uint8_t *)mptr - allocator->start);

            block_write(allocator->start + next.next, &next_next);

            block.next = next.next;
            block.size += next.size + BLOCK_METADATA_SIZE;
        }
    }

    block_write(ptr - BLOCK_METADATA_SIZE, &block);
}

#endif // DS_AL_IMPLEMENTATION

#ifdef DS_AP_IMPLEMENTATION

// Initialize the argparser with a custom allocator
DSHDEF void ds_argparse_parser_init_allocator(ds_argparse_parser *parser,
                                              char *name, char *description,
                                              char *version,
                                              ds_allocator *allocator) {
    parser->allocator = allocator;
    parser->name = name;
    parser->description = description;
    parser->version = version;
    ds_dynamic_array_init_allocator(&parser->arguments, sizeof(ds_argument),
                                    allocator);

    ds_argparse_add_argument(
        parser,
        (ds_argparse_options){.short_name = 'v',
                              .long_name = "version",
                              .description = "print the program version",
                              .type = ARGUMENT_TYPE_FLAG,
                              .required = 0});
    ds_argparse_add_argument(
        parser, (ds_argparse_options){.short_name = 'h',
                                      .long_name = "help",
                                      .description = "print this help message",
                                      .type = ARGUMENT_TYPE_FLAG,
                                      .required = 0});
}

// Initialize with the default malloc allocator
DSHDEF void ds_argparse_parser_init(ds_argparse_parser *parser, char *name,
                                    char *description, char *version) {
    ds_argparse_parser_init_allocator(parser, name, description, version, NULL);
}

// Add an argument to the parser
//
// Adds a new argument to the parser with the given options.
//
// Arguments:
// - parser: argument parser
// - options: argument options
DSHDEF int ds_argparse_add_argument(ds_argparse_parser *parser,
                                    ds_argparse_options options) {
    ds_argument arg = {
        .options = options,
    };

    switch (options.type) {
    case ARGUMENT_TYPE_VALUE:
        arg.value = NULL;
        break;
    case ARGUMENT_TYPE_FLAG:
        arg.flag = 0;
        break;
    case ARGUMENT_TYPE_POSITIONAL:
        arg.value = NULL;
        break;
    case ARGUMENT_TYPE_POSITIONAL_REST:
        ds_dynamic_array_init(&arg.values, sizeof(char *));
        break;
    case ARGUMENT_TYPE_VALUE_ARRAY:
        ds_dynamic_array_init(&arg.values, sizeof(char *));
        break;
    }

    return ds_dynamic_array_append(&parser->arguments, &arg);
}

static int argparse_validate_parser(ds_argparse_parser *parser) {
    int result = 0;
    int found_optional_positional = 0;
    int found_positional_rest = 0;

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        ds_argparse_options options = item->options;

        if (options.type == ARGUMENT_TYPE_POSITIONAL && found_positional_rest) {
            DS_LOG_ERROR("positional argument after positional rest: %s",
                         options.long_name);
            result = 1;
        }

        if (options.type == ARGUMENT_TYPE_POSITIONAL_REST &&
            found_positional_rest) {
            DS_LOG_ERROR("multiple positional rest arguments");
            result = 1;
        }

        if (options.type == ARGUMENT_TYPE_POSITIONAL_REST &&
            found_positional_rest == 0) {
            found_positional_rest = 1;
        }

        if (options.type == ARGUMENT_TYPE_POSITIONAL && options.required == 0) {
            found_optional_positional = 1;
        }

        if (options.short_name == '\0' && options.long_name == NULL) {
            DS_LOG_ERROR("no short_name and long_name for argument %zu", i);
            result = 1;
        }
        if (options.type == ARGUMENT_TYPE_FLAG && options.required == 1) {
            DS_LOG_ERROR("flag argument cannot be required: %s",
                         options.long_name);
            result = 1;
        }
        if (options.type == ARGUMENT_TYPE_POSITIONAL && options.required == 1 &&
            found_optional_positional == 1) {
            DS_LOG_ERROR("required positional argument after optional: %s",
                         options.long_name);
            result = 1;
        }
    }

    return result;
}

static int argparse_post_validate_parser(ds_argparse_parser *parser) {
    int result = 0;

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        ds_argparse_options options = item->options;

        if (options.type == ARGUMENT_TYPE_POSITIONAL && options.required == 1) {
            if (item->value == NULL) {
                DS_LOG_ERROR("missing required positional argument: %s",
                             options.long_name);
                result = 1;
            }
        }

        if (options.type == ARGUMENT_TYPE_VALUE && options.required == 1) {
            if (item->value == NULL) {
                DS_LOG_ERROR("missing required argument: --%s",
                             options.long_name);
                result = 1;
            }
        }

        if (options.type == ARGUMENT_TYPE_VALUE_ARRAY &&
            options.required == 1) {
            if (item->values.count == 0) {
                DS_LOG_ERROR("missing required argument: --%s",
                             options.long_name);
                result = 1;
            }
        }

        if (options.type == ARGUMENT_TYPE_POSITIONAL_REST &&
            options.required == 1) {
            if (item->values.count == 0) {
                DS_LOG_ERROR("missing required positional rest argument: %s",
                             options.long_name);
                result = 1;
            }
        }
    }

    return result;
}

static ds_argument *argparse_get_option_arg(ds_argparse_parser *parser,
                                            const char *name) {
    if (name[0] != '-') {
        DS_LOG_WARN("provided name is not an option: %s", name);
        return NULL;
    }

    ds_argument *arg = NULL;

    for (size_t j = 0; j < parser->arguments.count; j++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, j, (void **)&item);

        if ((name[1] == '-' && item->options.long_name != NULL &&
             strcmp(name + 2, item->options.long_name) == 0) ||
            (name[1] != '-' && item->options.short_name != '\0' &&
             name[1] == item->options.short_name)) {
            arg = item;
            break;
        }
    }

    if (arg == NULL) {
        DS_LOG_ERROR("invalid argument: %s", name);
        return NULL;
    }

    return arg;
}

static ds_argument *argparse_get_positional_arg(ds_argparse_parser *parser,
                                                const char *name) {
    if (name[0] == '-') {
        DS_LOG_WARN("provided name is not a positional argument: %s", name);
        return NULL;
    }

    ds_argument *arg = NULL;
    for (size_t j = 0; j < parser->arguments.count; j++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, j, (void **)&item);

        if (item->options.type == ARGUMENT_TYPE_POSITIONAL &&
            item->value == NULL) {
            arg = item;
            break;
        }

        if (item->options.type == ARGUMENT_TYPE_POSITIONAL_REST) {
            arg = item;
            break;
        }
    }

    return arg;
}

// Parse the command line arguments
//
// Parses the command line arguments and sets the values of the arguments in the
// parser.
//
// Arguments:
// - parser: argument parser
// - argc: number of command line arguments
// - argv: command line arguments
//
// Returns 0 if the parsing was successful, 1 otherwise.
int ds_argparse_parse(ds_argparse_parser *parser, int argc, char *argv[]) {
    int result = 0;

    if (argparse_validate_parser(parser) != 0) {
        return_defer(1);
    }

    for (int i = 1; i < argc; i++) {
        char *name = argv[i];

        if (strcmp(name, "-h") == 0 || strcmp(name, "--help") == 0) {
            ds_argparse_print_help(parser);
            exit(0);
        }

        if (strcmp(name, "-v") == 0 || strcmp(name, "--version") == 0) {
            ds_argparse_print_version(parser);
            exit(0);
        }

        if (name[0] == '-') {
            ds_argument *arg = argparse_get_option_arg(parser, name);

            if (arg == NULL) {
                return_defer(1);
            }

            switch (arg->options.type) {
            case ARGUMENT_TYPE_FLAG: {
                arg->flag = 1;
                break;
            }
            case ARGUMENT_TYPE_VALUE: {
                if (i + 1 >= argc) {
                    DS_LOG_ERROR("missing value for argument: %s", name);
                    ds_argparse_print_help(parser);
                    return_defer(1);
                }

                arg->value = argv[++i];
                break;
            }
            case ARGUMENT_TYPE_VALUE_ARRAY: {
                if (i + 1 >= argc) {
                    DS_LOG_ERROR("missing value for argument: %s", name);
                    ds_argparse_print_help(parser);
                    return_defer(1);
                }

                if (ds_dynamic_array_append(&arg->values, &argv[++i]) != 0) {
                    DS_LOG_ERROR("failed to append value to argument: %s",
                                 name);
                    return_defer(1);
                }
                break;
            }
            default: {
                DS_LOG_ERROR("type not supported for argument: %s", name);
                ds_argparse_print_help(parser);
                return_defer(1);
            }
            }
        } else {
            ds_argument *arg = argparse_get_positional_arg(parser, name);

            if (arg == NULL) {
                DS_LOG_ERROR("unexpected positional argument: %s", name);
                ds_argparse_print_help(parser);
                return_defer(1);
            }

            switch (arg->options.type) {
            case ARGUMENT_TYPE_POSITIONAL: {
                arg->value = name;
                break;
            }
            case ARGUMENT_TYPE_POSITIONAL_REST: {
                if (ds_dynamic_array_append(&arg->values, &name) != 0) {
                    DS_LOG_ERROR("failed to append value to positional rest");
                    return_defer(1);
                }
                break;
            }
            default: {
                DS_LOG_ERROR("type not supported for argument: %s", name);
                ds_argparse_print_help(parser);
                return_defer(1);
            }
            }

            arg->value = name;
        }
    }

    if (argparse_post_validate_parser(parser) != 0) {
        ds_argparse_print_help(parser);
        return_defer(1);
    }

defer:
    return result;
}

// Get the value of an argument
//
// Returns the value of the argument with the given long name.
//
// Arguments:
// - parser: argument parser
// - long_name: long name of the argument
//
// Returns:
// - value of the argument
char *ds_argparse_get_value(ds_argparse_parser *parser, char *long_name) {
    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        if (item->options.long_name != NULL &&
            strcmp(long_name, item->options.long_name) == 0) {
            if (item->options.type != ARGUMENT_TYPE_VALUE &&
                item->options.type != ARGUMENT_TYPE_POSITIONAL) {
                DS_LOG_WARN("argument is not a value: %s", long_name);
            }
            return item->value;
        }
    }

    return NULL;
}

// Get the value of a positional argument
//
// Returns the value of the positional argument with the given long name.
//
// Arguments:
// - parser: argument parser
// - long_name: long name of the argument
//
// Returns:
// - value of the flag argument
unsigned int ds_argparse_get_flag(ds_argparse_parser *parser, char *long_name) {
    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        if (item->options.long_name != NULL &&
            strcmp(long_name, item->options.long_name) == 0) {
            if (item->options.type != ARGUMENT_TYPE_FLAG) {
                DS_LOG_WARN("argument is not a flag: %s", long_name);
            }
            return item->flag;
        }
    }

    return 0;
}

DSHDEF int ds_argparse_get_values(struct ds_argparse_parser *parser,
                                  char *long_name, ds_dynamic_array *values) {
    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        if (item->options.long_name != NULL &&
            strcmp(long_name, item->options.long_name) == 0) {
            if (item->options.type != ARGUMENT_TYPE_POSITIONAL_REST &&
                item->options.type != ARGUMENT_TYPE_VALUE_ARRAY) {
                DS_LOG_WARN("argument is not a array: %s", long_name);
            }
            *values = item->values;
            return item->values.count;
        }
    }

    return 0;
}

// Show the help message
//
// Prints the help message for the argument parser.
//
// Arguments:
// - parser: argument parser
void ds_argparse_print_help(ds_argparse_parser *parser) {
    fprintf(stdout, "usage: %s [options]", parser->name);

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        ds_argparse_options options = item->options;

        if (options.type == ARGUMENT_TYPE_VALUE && options.required == 1) {
            fprintf(stdout, " -%c <%s>", options.short_name, options.long_name);
        }
    }

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        ds_argparse_options options = item->options;

        if (options.type == ARGUMENT_TYPE_POSITIONAL) {
            if (options.required == 1) {
                fprintf(stdout, " <%s>", options.long_name);
            } else {
                fprintf(stdout, " [%s]", options.long_name);
            }
        }
    }

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        ds_argparse_options options = item->options;

        if (options.type == ARGUMENT_TYPE_VALUE_ARRAY) {
            if (options.required == 1) {
                fprintf(stdout, " -%c <%s>...", options.short_name,
                        options.long_name);
            } else {
                fprintf(stdout, " -%c [%s]...", options.short_name,
                        options.long_name);
            }
        }
    }

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        ds_argparse_options options = item->options;

        if (options.type == ARGUMENT_TYPE_POSITIONAL_REST) {
            if (options.required == 1) {
                fprintf(stdout, " <%s>...", options.long_name);
            } else {
                fprintf(stdout, " [%s]...", options.long_name);
            }
        }
    }

    fprintf(stdout, "\n");
    fprintf(stdout, "%s\n", parser->description);
    fprintf(stdout, "\n");
    fprintf(stdout, "options:\n");

    for (size_t i = 0; i < parser->arguments.count; i++) {
        ds_argument *item = NULL;
        ds_dynamic_array_get_ref(&parser->arguments, i, (void **)&item);

        switch (item->options.type) {
        case ARGUMENT_TYPE_POSITIONAL: {
            fprintf(stdout, "  %c, %s\n", item->options.short_name,
                    item->options.long_name);
            fprintf(stdout, "      %s\n", item->options.description);
            fprintf(stdout, "\n");
            break;
        }
        case ARGUMENT_TYPE_POSITIONAL_REST: {
            fprintf(stdout, "  %c, %s\n", item->options.short_name,
                    item->options.long_name);
            fprintf(stdout, "      %s\n", item->options.description);
            fprintf(stdout, "\n");
            break;
        }
        case ARGUMENT_TYPE_FLAG: {
            fprintf(stdout, "  -%c, --%s\n", item->options.short_name,
                    item->options.long_name);
            fprintf(stdout, "      %s\n", item->options.description);
            fprintf(stdout, "\n");
            break;
        }
        case ARGUMENT_TYPE_VALUE: {
            fprintf(stdout, "  -%c, --%s <value>\n", item->options.short_name,
                    item->options.long_name);
            fprintf(stdout, "      %s\n", item->options.description);
            fprintf(stdout, "\n");
            break;
        }
        case ARGUMENT_TYPE_VALUE_ARRAY: {
            fprintf(stdout, "  -%c, --%s <value>...\n",
                    item->options.short_name, item->options.long_name);
            fprintf(stdout, "      %s\n", item->options.description);
            fprintf(stdout, "\n");
            break;
        }
        default: {
            DS_PANIC("invalid argument type");
        }
        }
    }
}

// Show the version
//
// Prints the version of the program.
//
// Arguments:
// - parser: argument parser
void ds_argparse_print_version(ds_argparse_parser *parser) {
    fprintf(stdout, "%s %s\n", parser->name, parser->version);
}

// Free the argument parser
//
// Frees the memory allocated for the argument parser.
//
// Arguments:
// - parser: argument parser
void ds_argparse_free(ds_argparse_parser *parser) {
    ds_dynamic_array_free(&parser->arguments);
}

#endif // DS_AP_IMPLEMENTATION
