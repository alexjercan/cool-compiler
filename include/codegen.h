#ifndef CODEGEN_H
#define CODEGEN_H

#include "ds.h"
#include "parser.h"
#include "semantic.h"

enum tac_kind {
    TAC_LABEL,
    TAC_JUMP,
    TAC_JUMP_IF_TRUE,
    TAC_ASSIGN_ISINSTANCE,
    TAC_CAST,
    TAC_ASSIGN_VALUE,
    TAC_DISPATCH_CALL,
    TAC_ASSIGN_NEW,
    TAC_ASSIGN_DEFAULT,
    TAC_ASSIGN_ISVOID,
    TAC_ASSIGN_ADD,
    TAC_ASSIGN_SUB,
    TAC_ASSIGN_MUL,
    TAC_ASSIGN_DIV,
    TAC_ASSIGN_NEG,
    TAC_ASSIGN_LT,
    TAC_ASSIGN_LE,
    TAC_ASSIGN_EQ,
    TAC_ASSIGN_NOT,
    TAC_IDENT,
    TAC_ASSIGN_INT,
    TAC_ASSIGN_STRING,
    TAC_ASSIGN_BOOL
};

typedef struct tac_ident {
        char *name;
        int index;
} tac_ident;

typedef struct tac_assign_int {
        char *ident;
        int value;
} tac_assign_int;

typedef struct tac_assign_string {
        char *ident;
        char *value;
} tac_assign_string;

typedef struct tac_assign_bool {
        char *ident;
        int value;
} tac_assign_bool;

typedef struct tac_assign_binary {
        char *ident;
        char *lhs;
        char *rhs;
} tac_assign_binary;

typedef struct tac_assign_eq {
        char *type;
        char *ident;
        char *lhs;
        char *rhs;
} tac_assign_eq;

typedef struct tac_assign_unary {
        char *ident;
        char *expr;
} tac_assign_unary;

typedef struct tac_assign_new {
        char *ident;
        char *type;
} tac_assign_new;

typedef struct tac_assign_value {
        char *ident;
        char *expr;
} tac_assign_value;

typedef struct tac_dispatch_call {
        char *ident;
        char *expr_type;
        char *expr;
        char *type;
        char *method;
        ds_dynamic_array args; // char *
} tac_dispatch_call;

typedef struct tac_label {
        char *label;
} tac_label;

typedef struct tac_jump {
        char *label;
} tac_jump;

typedef struct tac_jump_if_true {
        char *expr;
        char *label;
} tac_jump_if_true;

typedef struct tac_isinstance {
        char *ident;
        char *expr;
        char *type;
} tac_isinstance;

typedef struct tac_cast {
        char *ident;
        char *expr;
        char *type;
} tac_cast;

typedef struct tac_instr {
        enum tac_kind kind;
        union {
                tac_label label;
                tac_jump jump;
                tac_jump_if_true jump_if_true;
                tac_isinstance isinstance;
                tac_cast cast;
                tac_assign_value assign_value;
                tac_dispatch_call dispatch_call;
                tac_assign_new assign_new;
                tac_assign_new assign_default;
                tac_assign_binary assign_binary;
                tac_assign_eq assign_eq;
                tac_assign_unary assign_unary;
                struct tac_instr *assign_paren;
                tac_ident ident;
                tac_assign_int assign_int;
                tac_assign_string assign_string;
                tac_assign_bool assign_bool;
        };
} tac_instr;

typedef struct tac_result {
        ds_dynamic_array locals; // char *
        ds_dynamic_array instrs; // tac_instr
} tac_result;

int codegen_expr_to_tac(semantic_mapping *mapping, const expr_node *expr, tac_result *result);

void codegen_tac_print(semantic_mapping *mapping, program_node *program);

#endif // CODEGEN_H
