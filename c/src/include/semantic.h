#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ds.h"
#include "parser.h"

typedef struct semantic_context {
        const char *filename;
        int result;
        ds_dynamic_array classes; // class_context
} semantic_context;

typedef struct class_context {
        const char *name;
        struct class_context *parent;
        ds_dynamic_array objects; // object_context
        ds_dynamic_array methods; // method_context
} class_context;

typedef struct method_context {
        const char *name;
        const char *type;
        ds_dynamic_array formals; // object_context
} method_context;

typedef struct object_context {
        const char *name;
        const char *type;
} object_context;

int semantic_check(program_node *program, semantic_context *context);

#endif // SEMANTIC_H
