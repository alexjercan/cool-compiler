#ifndef CODEGEN_H
#define CODEGEN_H

#include "ds.h"
#include "parser.h"

enum tac_kind {
    TAC_ASSIGN_VALUE,
    TAC_DISPATCH_CALL,
    TAC_ASSIGN_NEW,
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
        char *expr;
        char *type;
        char *method;
        ds_dynamic_array args; // char *
} tac_dispatch_call;

typedef struct tac_instr {
        enum tac_kind kind;
        union {
                tac_assign_value assign_value;
                tac_dispatch_call dispatch_call;
                tac_assign_new assign_new;
                tac_assign_binary assign_binary;
                tac_assign_unary assign_unary;
                struct tac_instr *assign_paren;
                char *ident;
                tac_assign_int assign_int;
                tac_assign_string assign_string;
                tac_assign_bool assign_bool;
        };
} tac_instr;

int codegen_expr_to_tac(expr_node *expr, ds_dynamic_array *tac);

void codegen_tac_print(program_node *program);

#endif // CODEGEN_H
