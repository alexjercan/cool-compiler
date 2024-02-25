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
    EXPR_ASSIGN,
    EXPR_DISPATCH_FULL,
    EXPR_DISPATCH,
    EXPR_COND,
    EXPR_LOOP,
    EXPR_BLOCK,
    EXPR_LET,
    EXPR_CASE,
    EXPR_NEW,
    EXPR_ISVOID,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_NEG,
    EXPR_LT,
    EXPR_LE,
    EXPR_EQ,
    EXPR_NOT,
    EXPR_PAREN,
    EXPR_IDENT,
    EXPR_INT,
    EXPR_STRING,
    EXPR_BOOL,
};

typedef struct expr_unary_node {
        struct expr_node *expr;
} expr_unary_node;

typedef struct expr_binary_node {
        struct expr_node *lhs;
        struct expr_node *rhs;
} expr_binary_node;

typedef struct let_init_node {
        node_info name;
        node_info type;
        struct expr_node *init;
} let_init_node;

typedef struct case_node {
        node_info name;
        node_info type;
        struct expr_node *body;
} branch_node;

typedef struct expr_node {
        enum expr_type type;
        union {
                struct {
                        node_info name;
                        struct expr_node *value;
                } assign;
                struct {
                        struct expr_node *expr;
                        node_info type;
                        node_info method;
                        ds_dynamic_array args; // expr_node
                } dispatch_full;
                struct {
                        struct expr_node *expr;
                        node_info method;
                        ds_dynamic_array args; // expr_node
                } dispatch;
                struct {
                        struct expr_node *predicate;
                        struct expr_node *then;
                        struct expr_node *else_;
                } cond;
                struct {
                        struct expr_node *predicate;
                        struct expr_node *body;
                } loop;
                ds_dynamic_array block;
                struct {
                        ds_dynamic_array inits; // let_init_node
                        struct expr_node *body;
                } let;
                struct {
                        struct expr_node *expr;
                        ds_dynamic_array cases; // branch_node
                } case_;
                node_info new;
                struct expr_node *isvoid;
                expr_binary_node add;
                expr_binary_node sub;
                expr_binary_node mul;
                expr_binary_node div;
                expr_unary_node neg;
                expr_binary_node lt;
                expr_binary_node le;
                expr_binary_node eq;
                expr_unary_node not_;
                struct expr_node *paren;
                node_info ident;
                node_info integer;
                node_info string;
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
