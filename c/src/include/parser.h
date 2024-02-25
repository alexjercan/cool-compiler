#ifndef PARSER_H
#define PARSER_H

#include "ds.h"

enum parser_result {
        PARSER_OK,
        PARSER_ERROR,
};

typedef struct node_info {
        char *value;
        unsigned int line;
        unsigned int col;
} node_info;

typedef struct attribute_node {
        node_info name;
        node_info type;
        node_info value; // we assume it is gonna be a string
} attribute_node;

typedef struct formal_node {
        node_info name;
        node_info type;
} formal_node;

typedef struct method_node {
        node_info name;
        node_info type;
        ds_dynamic_array formals;
        node_info body; // we assume it is gonna be a string
} method_node;

typedef struct class_node {
        node_info name;
        node_info superclass;
        ds_dynamic_array attributes;
        ds_dynamic_array methods;
} class_node;

typedef struct program_node {
        ds_dynamic_array classes;
} program_node;

int parser_run(const char *filename, ds_dynamic_array *tokens,
               program_node *program);

#endif // PARSER_H
