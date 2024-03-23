#ifndef PARSER_H
#define PARSER_H

#include "ds.h"

enum parser_result {
    PARSER_OK = 0,
    PARSER_ERROR,
};

typedef struct node_info {
        char *value;
        unsigned int line;
        unsigned int col;
} node_info;

enum expr_kind {
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
    EXPR_EXTERN,
    EXPR_NULL,
};

typedef struct expr_unary_node {
        node_info op;
        struct expr_node *expr;
} expr_unary_node;

typedef struct expr_binary_node {
        node_info op;
        struct expr_node *lhs;
        struct expr_node *rhs;
} expr_binary_node;

typedef struct let_init_node {
        node_info name;
        node_info type;
        struct expr_node *init;
} let_init_node;

typedef struct branch_node {
        node_info name;
        node_info type;
        struct expr_node *body;
} branch_node;

typedef struct assign_node {
        node_info name;
        struct expr_node *value;
} assign_node;

typedef struct dispatch_node {
        node_info method;
        ds_dynamic_array args; // expr_node
} dispatch_node;

typedef struct dispatch_full_node {
        struct expr_node *expr;
        node_info type;
        dispatch_node *dispatch;
} dispatch_full_node;

typedef struct cond_node {
        node_info node;
        struct expr_node *predicate;
        struct expr_node *then;
        struct expr_node *else_;
} cond_node;

typedef struct loop_node {
        node_info node;
        struct expr_node *predicate;
        struct expr_node *body;
} loop_node;

typedef struct block_node {
        node_info node;
        ds_dynamic_array exprs; // expr_node
} block_node;

typedef struct let_node {
        node_info node;
        ds_dynamic_array inits; // let_init_node
        struct expr_node *body;
} let_node;

typedef struct case_node {
        node_info node;
        struct expr_node *expr;
        ds_dynamic_array cases; // branch_node
} case_node;

typedef struct new_node {
        node_info node;
        node_info type;
} new_node;

typedef struct expr_null {
        node_info type;
} expr_null;

typedef struct expr_node {
        const char *type;
        enum expr_kind kind;
        union {
                assign_node assign;
                dispatch_full_node dispatch_full;
                dispatch_node dispatch;
                cond_node cond;
                loop_node loop;
                block_node block;
                let_node let;
                case_node case_;
                new_node new;
                expr_unary_node isvoid;
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
                expr_null null;
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
        ds_dynamic_array formals; // formal_node
        expr_node body;
} method_node;

typedef struct class_node {
        const char *filename;
        node_info name;
        node_info superclass;
        ds_dynamic_array attributes; // attribute_node
        ds_dynamic_array methods; // method_node
} class_node;

typedef struct program_node {
        const char *filename;
        ds_dynamic_array classes; // class_node
} program_node;

enum parser_result parser_run(const char *filename, ds_dynamic_array *tokens,
                              program_node *program);

void parser_merge(ds_dynamic_array programs, program_node *program,
                  unsigned int index);

#ifndef INDENT_SIZE
#define INDENT_SIZE 2
#endif

void parser_print_ast(program_node *program);

#endif // PARSER_H
