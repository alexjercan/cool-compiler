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

typedef struct method_environment_item {
        const char *class_name;
        const char *method_name;
        ds_dynamic_array names;   // const char *
        ds_dynamic_array formals; // const char *
        const char *type;
} method_environment_item;

typedef struct method_environment {
        ds_dynamic_array items; // method_environment_item
} method_environment;

typedef struct object_environment_item {
        const char *class_name;
        ds_dynamic_array objects; // object_context
} object_environment_item;

typedef struct object_environment {
        ds_dynamic_array items; // object_environment_item
} object_environment;

int semantic_check(program_node *program, semantic_context *context);

void method_env_show(method_environment method_env);
void object_env_show(object_environment object_env);

#endif // SEMANTIC_H
