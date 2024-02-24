#ifndef PARSER_H
#define PARSER_H

#include "ds.h"

typedef struct node_info {
        char *value;
        unsigned int pos;
} node_info;

typedef struct attribute_node {
        node_info name;
        node_info type;
} attribute_node;

typedef struct class_node {
        node_info name;
        node_info superclass;
        ds_dynamic_array attributes;
} class_node;

typedef struct program_node {
        ds_dynamic_array classes;
} program_node;

int parser_run(const char *filename, ds_dynamic_array *tokens,
               program_node *program);
void parser_print(program_node *program);

#endif // PARSER_H
