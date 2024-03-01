#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

typedef struct typed_symbol {
        node_info name;
        node_info type;
        struct class_context *class;
} typed_symbol;

typedef struct method_context {
        node_info name;
        node_info type;
        struct class_context *return_type;
        ds_dynamic_array formals;
} method_context;

typedef struct class_context {
        node_info name;
        struct class_context *parent;
        ds_dynamic_array attributes;
        ds_dynamic_array methods;
} class_context;

typedef struct program_context {
        const char *filename;
        ds_dynamic_array classes;
        int result;
} program_context;

int semantic_check(program_node *program, program_context *context);

#endif // SEMANTIC_H
