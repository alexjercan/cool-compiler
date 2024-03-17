#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ds.h"
#include "parser.h"

#define OBJECT_TYPE "Object"
#define INT_TYPE "Int"
#define STRING_TYPE "String"
#define BOOL_TYPE "Bool"

#define SELF_TYPE "SELF_TYPE"

enum semantic_result {
    SEMANTIC_OK = 0,
    SEMANTIC_ERROR,
};

typedef struct class_mapping_attribute {
        const char *name;
        const attribute_node *attribute;
} class_mapping_attribute;

typedef struct class_mapping_item {
        const char *class_name;
        ds_dynamic_array attributes; // class_mapping_attribute
} class_mapping_item;

typedef struct class_mapping {
        ds_dynamic_array items; // class_mapping_item
} class_mapping;

typedef struct implementation_mapping_item {
        const char *class_name;
        const char *parent_name;
        const char *method_name;
        const method_node *method;
} implementation_mapping_item;

typedef struct implementation_mapping {
        ds_dynamic_array items; // implementation_mapping_item
} implementation_mapping;

typedef struct parent_mapping_item {
        const char *name;
        struct parent_mapping_item *parent;
} parent_mapping_item;

typedef struct parent_mapping {
        ds_dynamic_array classes; // parent_mapping_item
} parent_mapping;

typedef struct semantic_mapping {
        class_mapping classes;
        implementation_mapping implementations;
        parent_mapping parents;
} semantic_mapping;

enum semantic_result semantic_check(const char *filename, program_node *program,
                                    semantic_mapping *mapping);
void semantic_print_mapping(semantic_mapping *mapping);

#endif // SEMANTIC_H
