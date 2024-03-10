#include "codegen.h"
#include "ds.h"
#include "parser.h"

typedef struct tac_context {
        int result;
        int temp_count;
        int label_count;
} tac_context;

static void tac_new_var(tac_context *context, char **ident) {
    *ident = malloc(32);
    snprintf(*ident, 32, "t%d", context->temp_count++);
}

static void tac_new_label(tac_context *context, char **label) {
    *label = malloc(32);
    snprintf(*label, 32, "L%d", context->label_count++);
}

static void tac_expr(tac_context *context, expr_node *expr,
                     ds_dynamic_array *instrs, tac_instr *result);

static void tac_assign(tac_context *context, assign_node *assign,
                       ds_dynamic_array *instrs, tac_instr *result) {
    tac_instr expr;
    tac_expr(context, assign->value, instrs, &expr);

    tac_assign_value assign_ident = {
        .ident = assign->name.value,
        .expr = expr.ident,
    };
    tac_instr instr = {
        .kind = TAC_ASSIGN_VALUE,
        .assign_value = assign_ident,
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = assign->name.value;
}

static void tac_dispatch_args(tac_context *context, dispatch_node *dispatch,
                              ds_dynamic_array *instrs,
                              ds_dynamic_array *args) {
    ds_dynamic_array_init(args, sizeof(char *));

    for (unsigned int i = 0; i < dispatch->args.count; i++) {
        expr_node expr;
        ds_dynamic_array_get(&dispatch->args, i, &expr);

        tac_instr instr;
        tac_expr(context, &expr, instrs, &instr);

        ds_dynamic_array_append(args, &instr.ident);
    }
}

static void tac_dispatch_full(tac_context *context,
                              dispatch_full_node *dispatch_full,
                              ds_dynamic_array *instrs, tac_instr *result) {
    ds_dynamic_array args;
    tac_dispatch_args(context, dispatch_full->dispatch, instrs, &args);

    tac_instr expr;
    tac_expr(context, dispatch_full->expr, instrs, &expr);

    char *type;
    if (dispatch_full->type.value != NULL) {
        type = dispatch_full->type.value;
    } else {
        type = (char *)dispatch_full->expr->type;
    }

    char *ident;
    tac_new_var(context, &ident);
    tac_dispatch_call dispatch_call = {
        .ident = ident,
        .type = type,
        .expr = expr.ident,
        .method = dispatch_full->dispatch->method.value,
        .args = args,
    };
    tac_instr instr = {
        .kind = TAC_DISPATCH_CALL,
        .dispatch_call = dispatch_call,
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_dispatch(tac_context *context, dispatch_node *dispatch,
                         ds_dynamic_array *instrs, tac_instr *result) {
    ds_dynamic_array args;
    tac_dispatch_args(context, dispatch, instrs, &args);

    char *ident;
    tac_new_var(context, &ident);
    tac_dispatch_call dispatch_call = {
        .ident = ident,
        .type = "SELF_TYPE",
        .expr = "self",
        .method = dispatch->method.value,
        .args = args,
    };
    tac_instr instr = {
        .kind = TAC_DISPATCH_CALL,
        .dispatch_call = dispatch_call,
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_cond(tac_context *context, cond_node *cond,
                     ds_dynamic_array *instrs, tac_instr *result) {

    char *ident;
    tac_new_var(context, &ident);

    char *then_label;
    tac_new_label(context, &then_label);

    char *done_label;
    tac_new_label(context, &done_label);

    // ... PREDICATE ...
    tac_instr predicate;
    tac_expr(context, cond->predicate, instrs, &predicate);

    // bt PREDICATE THEN_LABEL
    tac_instr jump_then_instr = {
        .kind = TAC_JUMP_IF_TRUE,
        .jump_if_true =
            {
                .expr = predicate.ident,
                .label = then_label,
            },
    };
    ds_dynamic_array_append(instrs, &jump_then_instr);

    // ... ELSE ...
    tac_instr else_instr;
    tac_expr(context, cond->else_, instrs, &else_instr);
    tac_instr instr_else = {
        .kind = TAC_ASSIGN_VALUE,
        .assign_value =
            {
                .ident = ident,
                .expr = else_instr.ident,
            },
    };
    ds_dynamic_array_append(instrs, &instr_else);

    // jump DONE_LABEL
    tac_instr jump_done_instr = {
        .kind = TAC_JUMP,
        .jump =
            {
                .label = done_label,
            },
    };
    ds_dynamic_array_append(instrs, &jump_done_instr);

    // THEN_LABEL:
    tac_instr label_then_instr = {
        .kind = TAC_LABEL,
        .label =
            {
                .label = then_label,
            },
    };
    ds_dynamic_array_append(instrs, &label_then_instr);

    // ... THEN ...
    tac_instr then_instr;
    tac_expr(context, cond->then, instrs, &then_instr);
    tac_instr instr_then = {
        .kind = TAC_ASSIGN_VALUE,
        .assign_value =
            {
                .ident = ident,
                .expr = then_instr.ident,
            },
    };
    ds_dynamic_array_append(instrs, &instr_then);

    // DONE_LABEL:
    tac_instr label_done_instr = {
        .kind = TAC_LABEL,
        .label =
            {
                .label = done_label,
            },
    };
    ds_dynamic_array_append(instrs, &label_done_instr);

    // result = IDENT
    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_block(tac_context *context, block_node *block,
                      ds_dynamic_array *instrs, tac_instr *result) {
    for (unsigned int i = 0; i < block->exprs.count; i++) {
        expr_node expr;
        ds_dynamic_array_get(&block->exprs, i, &expr);

        tac_instr instr;
        tac_expr(context, &expr, instrs, &instr);

        *result = instr;
    }
}

static void tac_let(tac_context *context, let_node *let,
                    ds_dynamic_array *instrs, tac_instr *result) {
    for (unsigned int i = 0; i < let->inits.count; i++) {
        let_init_node let_init;
        ds_dynamic_array_get(&let->inits, i, &let_init);

        tac_instr expr;
        if (let_init.init != NULL) {
            tac_expr(context, let_init.init, instrs, &expr);
        } else {
            char *ident;
            tac_new_var(context, &ident);
            tac_assign_new assign = {
                .ident = ident,
                .type = let_init.type.value,
            };
            expr.kind = TAC_ASSIGN_NEW;
            expr.assign_new = assign;
            ds_dynamic_array_append(instrs, &expr);
        }

        tac_assign_value assign_ident = {
            .ident = let_init.name.value,
            .expr = expr.ident,
        };
        tac_instr instr = {
            .kind = TAC_ASSIGN_VALUE,
            .assign_value = assign_ident,
        };
        ds_dynamic_array_append(instrs, &instr);
    }

    tac_expr(context, let->body, instrs, result);
}

static void tac_new(tac_context *context, new_node *new,
                    ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);
    tac_assign_new assign = {
        .ident = ident,
        .type = new->type.value,
    };
    tac_instr instr = {
        .kind = TAC_ASSIGN_NEW,
        .assign_new = assign,
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_binary(tac_context *context, expr_binary_node *binary,
                       enum tac_kind kind, ds_dynamic_array *instrs,
                       tac_instr *result) {

    tac_instr lhs;
    tac_expr(context, binary->lhs, instrs, &lhs);

    tac_instr rhs;
    tac_expr(context, binary->rhs, instrs, &rhs);

    char *ident;
    tac_new_var(context, &ident);
    tac_assign_binary assign = {
        .ident = ident,
        .lhs = lhs.ident,
        .rhs = rhs.ident,
    };
    tac_instr instr = {
        .kind = kind,
        .assign_binary = assign,
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_unary(tac_context *context, expr_unary_node *unary,
                      enum tac_kind kind, ds_dynamic_array *instrs,
                      tac_instr *result) {
    tac_instr expr;
    tac_expr(context, unary->expr, instrs, &expr);

    char *ident;
    tac_new_var(context, &ident);
    tac_assign_unary assign = {
        .ident = ident,
        .expr = expr.ident,
    };
    tac_instr instr = {
        .kind = kind,
        .assign_unary = assign,
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_paren(tac_context *context, expr_node *paren,
                      ds_dynamic_array *instrs, tac_instr *result) {
    tac_expr(context, paren, instrs, result);
}

static void tac_ident(tac_context *context, node_info *ident,
                      ds_dynamic_array *instrs, tac_instr *result) {
    result->kind = TAC_IDENT;
    result->ident = ident->value;
}

static void tac_int(tac_context *context, node_info *int_node,
                    ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);
    tac_assign_int assign = {
        .ident = ident,
        .value = atoi(int_node->value),
    };
    tac_instr instr = {
        .kind = TAC_ASSIGN_INT,
        .assign_int = assign,
    };

    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident = ident;
}

static void tac_string(tac_context *context, node_info *string,
                       ds_dynamic_array *instrs, tac_instr *result) {
    tac_instr assign;

    assign.kind = TAC_ASSIGN_STRING;
    tac_new_var(context, &assign.assign_string.ident);
    assign.assign_string.value = string->value;

    ds_dynamic_array_append(instrs, &assign);

    result->kind = TAC_IDENT;
    result->ident = assign.assign_string.ident;
}

static void tac_bool(tac_context *context, node_info *boolean,
                     ds_dynamic_array *instrs, tac_instr *result) {
    tac_instr assign;

    assign.kind = TAC_ASSIGN_BOOL;
    tac_new_var(context, &assign.assign_bool.ident);
    assign.assign_bool.value = strcmp(boolean->value, "true") == 0 ? 1 : 0;

    ds_dynamic_array_append(instrs, &assign);

    result->kind = TAC_IDENT;
    result->ident = assign.assign_bool.ident;
}

static void tac_expr(tac_context *context, expr_node *expr,
                     ds_dynamic_array *instrs, tac_instr *result) {
    switch (expr->kind) {
    case EXPR_NONE:
        break;
    case EXPR_ASSIGN:
        return tac_assign(context, &expr->assign, instrs, result);
    case EXPR_DISPATCH_FULL:
        return tac_dispatch_full(context, &expr->dispatch_full, instrs, result);
    case EXPR_DISPATCH:
        return tac_dispatch(context, &expr->dispatch, instrs, result);
    case EXPR_COND:
        return tac_cond(context, &expr->cond, instrs, result);
    case EXPR_LOOP:
    case EXPR_BLOCK:
        return tac_block(context, &expr->block, instrs, result);
    case EXPR_LET:
        return tac_let(context, &expr->let, instrs, result);
    case EXPR_CASE:
    case EXPR_NEW:
        return tac_new(context, &expr->new, instrs, result);
    case EXPR_ISVOID:
        return tac_unary(context, &expr->isvoid, TAC_ASSIGN_ISVOID, instrs,
                         result);
    case EXPR_ADD:
        return tac_binary(context, &expr->add, TAC_ASSIGN_ADD, instrs, result);
    case EXPR_SUB:
        return tac_binary(context, &expr->sub, TAC_ASSIGN_SUB, instrs, result);
    case EXPR_MUL:
        return tac_binary(context, &expr->mul, TAC_ASSIGN_MUL, instrs, result);
    case EXPR_DIV:
        return tac_binary(context, &expr->div, TAC_ASSIGN_DIV, instrs, result);
    case EXPR_NEG:
        return tac_unary(context, &expr->neg, TAC_ASSIGN_NEG, instrs, result);
    case EXPR_LT:
        return tac_binary(context, &expr->lt, TAC_ASSIGN_LT, instrs, result);
    case EXPR_LE:
        return tac_binary(context, &expr->le, TAC_ASSIGN_LE, instrs, result);
    case EXPR_EQ:
        return tac_binary(context, &expr->eq, TAC_ASSIGN_EQ, instrs, result);
    case EXPR_NOT:
        return tac_unary(context, &expr->not_, TAC_ASSIGN_NOT, instrs, result);
    case EXPR_PAREN:
        return tac_paren(context, expr->paren, instrs, result);
    case EXPR_IDENT:
        return tac_ident(context, &expr->ident, instrs, result);
    case EXPR_INT:
        return tac_int(context, &expr->integer, instrs, result);
    case EXPR_STRING:
        return tac_string(context, &expr->string, instrs, result);
    case EXPR_BOOL:
        return tac_bool(context, &expr->boolean, instrs, result);
    }
}

int codegen_expr_to_tac(expr_node *expr, ds_dynamic_array *tac) {
    tac_context context = {.result = 0, .temp_count = 0, .label_count = 0};

    ds_dynamic_array_init(tac, sizeof(tac_instr));
    tac_instr result;

    tac_expr(&context, expr, tac, &result);

    ds_dynamic_array_append(tac, &result);

    return context.result;
}
