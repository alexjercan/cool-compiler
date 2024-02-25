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

enum expr_type {
    EXPR_NONE,
    EXPR_INT,
    EXPR_STRING,
    EXPR_BOOL,
    EXPR_IDENT,
};

typedef struct expr_node {
        enum expr_type type;
        union {
                node_info ident;
                node_info string;
                node_info integer;
                node_info boolean;
        };
} expr_node;

typedef struct attribute_node {
        node_info name;
        node_info type;
        expr_node value;
} attribute_node;

typedef struct formal_node {
        node_info name;
        node_info type;
} formal_node;

typedef struct method_node {
        node_info name;
        node_info type;
        ds_dynamic_array formals;
        expr_node body;
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
