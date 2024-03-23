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
        const char *attribute_name;
        const attribute_node *attribute;
} class_mapping_attribute;

typedef struct implementation_mapping_item {
        const char *from_class;
        const char *method_name;
        const method_node *method;
} implementation_mapping_item;

typedef struct semantic_mapping_item {
        const char *class_name;
        struct semantic_mapping_item *parent;
        ds_dynamic_array attributes; // class_mapping_attribute
        ds_dynamic_array methods; // implementation_mapping_item
} semantic_mapping_item;

typedef struct semantic_mapping {
        ds_dynamic_array classes; // semantic_mapping_item
} semantic_mapping;

enum semantic_result semantic_check(program_node *program, semantic_mapping *mapping);
void semantic_print_mapping(semantic_mapping *mapping);

#endif // SEMANTIC_H
