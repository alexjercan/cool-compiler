#include "codegen.h"
#include "parser.h"

static void print_tac_label(tac_label label) { printf("%s:\n", label.label); }

static void print_tac_jump(tac_jump jump) { printf("jump %s\n", jump.label); }

static void print_tac_jump_if_true(tac_jump_if_true jump_if_true) {
    printf("bt %s %s\n", jump_if_true.expr, jump_if_true.label);
}

static void print_tac_assign_isinstance(tac_isinstance isinstance) {
    printf("%s <- %s instanceof %s\n", isinstance.ident, isinstance.expr,
           isinstance.type);
}

static void print_tac_cast(tac_cast cast) {
    printf("%s <- %s as %s\n", cast.ident, cast.expr, cast.type);
}

static void print_tac_assign_value(tac_assign_value assign_value) {
    printf("%s <- %s\n", assign_value.ident, assign_value.expr);
}

static void print_tac_dispatch_call(tac_dispatch_call dispatch_call) {
    printf("%s <- ", dispatch_call.ident);

    if (dispatch_call.expr != NULL) {
        printf("%s", dispatch_call.expr);
        if (dispatch_call.type != NULL) {
            printf("@%s", dispatch_call.type);
        }
        printf(".");
    }

    printf("%s(", dispatch_call.method);

    for (unsigned int i = 0; i < dispatch_call.args.count; i++) {
        char *arg;
        ds_dynamic_array_get(&dispatch_call.args, i, &arg);
        printf("%s", arg);
        if (i < dispatch_call.args.count - 1) {
            printf(", ");
        }
    }
    printf(")\n");
}

static void print_tac_assign_new(tac_assign_new assign_new) {
    printf("%s <- new %s\n", assign_new.ident, assign_new.type);
}

static void print_tac_assign_default(tac_assign_new assign_new) {
    printf("%s <- default %s\n", assign_new.ident, assign_new.type);
}

static void print_tac_assign_binary(tac_assign_binary assign_binary,
                                    const char *op) {
    printf("%s <- %s %s %s\n", assign_binary.ident, assign_binary.lhs, op,
           assign_binary.rhs);
}

static void print_tac_assign_eq(tac_assign_eq assign_binary) {
    printf("%s <- %s = %s\n", assign_binary.ident, assign_binary.lhs,
           assign_binary.rhs);
}

static void print_tac_assign_unary(tac_assign_unary assign_unary,
                                   const char *op) {
    printf("%s <- %s %s\n", assign_unary.ident, op, assign_unary.expr);
}

static void print_tac_ident(tac_ident ident) { printf("%s\n", ident.name); }

static void print_tac_assign_int(tac_assign_int assign_int) {
    printf("%s <- int %d\n", assign_int.ident, assign_int.value);
}

static void print_tac_assign_string(tac_assign_string assign_string) {
    printf("%s <- string \"%s\"\n", assign_string.ident, assign_string.value);
}

static void print_tac_assign_bool(tac_assign_bool assign_bool) {
    printf("%s <- bool %s\n", assign_bool.ident,
           assign_bool.value ? "true" : "false");
}

static void print_tac(tac_instr tac) {
    switch (tac.kind) {
    case TAC_LABEL:
        return print_tac_label(tac.label);
    case TAC_JUMP:
        return print_tac_jump(tac.jump);
    case TAC_JUMP_IF_TRUE:
        return print_tac_jump_if_true(tac.jump_if_true);
    case TAC_ASSIGN_ISINSTANCE:
        return print_tac_assign_isinstance(tac.isinstance);
    case TAC_CAST:
        return print_tac_cast(tac.cast);
    case TAC_ASSIGN_VALUE:
        return print_tac_assign_value(tac.assign_value);
    case TAC_DISPATCH_CALL:
        return print_tac_dispatch_call(tac.dispatch_call);
    case TAC_ASSIGN_NEW:
        return print_tac_assign_new(tac.assign_new);
    case TAC_ASSIGN_DEFAULT:
        return print_tac_assign_default(tac.assign_default);
    case TAC_ASSIGN_ISVOID:
        return print_tac_assign_unary(tac.assign_unary, "isvoid");
    case TAC_ASSIGN_ADD:
        return print_tac_assign_binary(tac.assign_binary, "+");
    case TAC_ASSIGN_SUB:
        return print_tac_assign_binary(tac.assign_binary, "-");
    case TAC_ASSIGN_MUL:
        return print_tac_assign_binary(tac.assign_binary, "*");
    case TAC_ASSIGN_DIV:
        return print_tac_assign_binary(tac.assign_binary, "/");
    case TAC_ASSIGN_NEG:
        return print_tac_assign_unary(tac.assign_unary, "~");
    case TAC_ASSIGN_LT:
        return print_tac_assign_binary(tac.assign_binary, "<");
    case TAC_ASSIGN_LE:
        return print_tac_assign_binary(tac.assign_binary, "<=");
    case TAC_ASSIGN_EQ:
        return print_tac_assign_eq(tac.assign_eq);
    case TAC_ASSIGN_NOT:
        return print_tac_assign_unary(tac.assign_unary, "not");
    case TAC_IDENT:
        return print_tac_ident(tac.ident);
    case TAC_ASSIGN_INT:
        return print_tac_assign_int(tac.assign_int);
    case TAC_ASSIGN_STRING:
        return print_tac_assign_string(tac.assign_string);
    case TAC_ASSIGN_BOOL:
        return print_tac_assign_bool(tac.assign_bool);
    default:
        DS_PANIC("Unknown tac kind");
    }
}

void codegen_tac_print(semantic_mapping *mapping, program_node *program) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        for (unsigned int j = 0; j < class.methods.count; j++) {
            method_node method;
            ds_dynamic_array_get(&class.methods, j, &method);

            if (method.body.kind == EXPR_EXTERN) {
                continue;
            }

            tac_result tac;
            codegen_expr_to_tac(mapping, &method.body, &tac);

            printf("%s.%s\n", class.name.value, method.name.value);
            for (unsigned int k = 0; k < tac.instrs.count; k++) {
                tac_instr instr;
                ds_dynamic_array_get(&tac.instrs, k, &instr);

                print_tac(instr);
            }
        }
    }
}
