#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

typedef struct semantic_context {
    const char *filename;
    int result;
    ds_hash_table classes;
} semantic_context;

int semantic_check(program_node *program, semantic_context *context);

#endif // SEMANTIC_H
