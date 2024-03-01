#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

typedef struct class_context {
        node_info name;
        struct class_context *parent;
} class_context;

typedef struct program_context {
        const char *filename;
        ds_dynamic_array classes;
        int result;
} program_context;

int semantic_check(program_node *program, program_context *context);

#endif // SEMANTIC_H
