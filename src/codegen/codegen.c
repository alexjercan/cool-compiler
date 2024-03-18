#include "codegen.h"
#include "ds.h"
#include "parser.h"

typedef struct tac_context {
        int result;
        int temp_count;
        int label_count;
        ds_dynamic_array locals; // char *
} tac_context;

static void tac_new_var(tac_context *context, char **ident) {
    int needed = snprintf(NULL, 0, "t%d", context->temp_count) + 1;

    *ident = malloc(needed);
    snprintf(*ident, needed, "t%d", context->temp_count++);

    ds_dynamic_array_append(&context->locals, *ident);
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

    tac_instr instr = {
        .kind = TAC_ASSIGN_VALUE,
        .assign_value =
            {
                .ident = assign->name.value,
                .expr = expr.ident.name,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = assign->name.value;
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
    tac_instr instr = {
        .kind = TAC_DISPATCH_CALL,
        .dispatch_call =
            {
                .ident = ident,
                .type = type,
                .expr = expr.ident.name,
                .method = dispatch_full->dispatch->method.value,
                .args = args,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
}

static void tac_dispatch(tac_context *context, dispatch_node *dispatch,
                         ds_dynamic_array *instrs, tac_instr *result) {
    ds_dynamic_array args;
    tac_dispatch_args(context, dispatch, instrs, &args);

    char *ident;
    tac_new_var(context, &ident);
    tac_instr instr = {
        .kind = TAC_DISPATCH_CALL,
        .dispatch_call =
            {
                .ident = ident,
                .type = "SELF_TYPE",
                .expr = "self",
                .method = dispatch->method.value,
                .args = args,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
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
                .expr = predicate.ident.name,
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
                .expr = else_instr.ident.name,
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
                .expr = then_instr.ident.name,
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
    result->ident.name = ident;
}

static void tac_loop(tac_context *context, loop_node *loop,
                     ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);

    char *loop_label;
    tac_new_label(context, &loop_label);

    char *done_label;
    tac_new_label(context, &done_label);

    // LOOP_LABEL:
    tac_instr label_loop_instr = {
        .kind = TAC_LABEL,
        .label =
            {
                .label = loop_label,
            },
    };
    ds_dynamic_array_append(instrs, &label_loop_instr);

    // ... PREDICATE ...
    tac_instr predicate;
    tac_expr(context, loop->predicate, instrs, &predicate);

    char *not_predicate_ident;
    tac_new_var(context, &not_predicate_ident);
    tac_instr not_predicate = {
        .kind = TAC_ASSIGN_NOT,
        .assign_unary =
            {
                .ident = not_predicate_ident,
                .expr = predicate.ident.name,
            },
    };
    ds_dynamic_array_append(instrs, &not_predicate);

    // bt not PREDICATE DONE_LABEL
    tac_instr jump_done_instr = {
        .kind = TAC_JUMP_IF_TRUE,
        .jump_if_true =
            {
                .expr = not_predicate_ident,
                .label = done_label,
            },
    };
    ds_dynamic_array_append(instrs, &jump_done_instr);

    // ... BODY ...
    tac_instr body;
    tac_expr(context, loop->body, instrs, &body);
    tac_instr body_instr = {
        .kind = TAC_ASSIGN_VALUE,
        .assign_value =
            {
                .ident = ident,
                .expr = body.ident.name,
            },
    };
    ds_dynamic_array_append(instrs, &body_instr);

    // jump LOOP_LABEL
    tac_instr jump_loop_instr = {
        .kind = TAC_JUMP,
        .jump =
            {
                .label = loop_label,
            },
    };
    ds_dynamic_array_append(instrs, &jump_loop_instr);

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
    result->ident.name = ident;
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

        tac_instr instr = {
            .kind = TAC_ASSIGN_VALUE,
            .assign_value =
                {
                    .ident = let_init.name.value,
                    .expr = expr.ident.name,
                },
        };
        ds_dynamic_array_append(instrs, &instr);
    }

    tac_expr(context, let->body, instrs, result);
}

static void tac_case(tac_context *context, case_node *case_,
                     ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);

    char *done_label;
    tac_new_label(context, &done_label);

    tac_instr expr;
    tac_expr(context, case_->expr, instrs, &expr);

    ds_dynamic_array case_labels;
    ds_dynamic_array_init(&case_labels, sizeof(char *));

    for (unsigned int i = 0; i < case_->cases.count; i++) {
        char *ident;
        tac_new_var(context, &ident);

        char *case_label;
        tac_new_label(context, &case_label);

        ds_dynamic_array_append(&case_labels, &case_label);

        branch_node branch;
        ds_dynamic_array_get(&case_->cases, i, &branch);

        tac_instr isinatance_instr = {
            .kind = TAC_ASSIGN_ISINSTANCE,
            .isinstance =
                {
                    .ident = ident,
                    .expr = expr.ident.name,
                    .type = branch.type.value,
                },
        };
        ds_dynamic_array_append(instrs, &isinatance_instr);

        // bt ISINSTANCE CASE_LABEL
        tac_instr jump_case_instr = {
            .kind = TAC_JUMP_IF_TRUE,
            .jump_if_true =
                {
                    .expr = ident,
                    .label = case_label,
                },
        };
        ds_dynamic_array_append(instrs, &jump_case_instr);
    }

    for (unsigned int i = 0; i < case_->cases.count; i++) {
        char *case_label;
        ds_dynamic_array_get(&case_labels, i, &case_label);

        // CASE_LABEL:
        tac_instr label_case_instr = {
            .kind = TAC_LABEL,
            .label =
                {
                    .label = case_label,
                },
        };
        ds_dynamic_array_append(instrs, &label_case_instr);

        // ... BODY ...
        branch_node branch;
        ds_dynamic_array_get(&case_->cases, i, &branch);

        tac_instr cast_instr = {
            .kind = TAC_CAST,
            .cast =
                {
                    .ident = branch.name.value,
                    .expr = expr.ident.name,
                    .type = branch.type.value,
                },
        };
        ds_dynamic_array_append(instrs, &cast_instr);

        tac_instr body;
        tac_expr(context, branch.body, instrs, &body);

        tac_instr body_instr = {
            .kind = TAC_ASSIGN_VALUE,
            .assign_value =
                {
                    .ident = ident,
                    .expr = body.ident.name,
                },
        };
        ds_dynamic_array_append(instrs, &body_instr);

        // jump DONE_LABEL
        tac_instr jump_done_instr = {
            .kind = TAC_JUMP,
            .jump =
                {
                    .label = done_label,
                },
        };
        ds_dynamic_array_append(instrs, &jump_done_instr);
    }

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
    result->ident.name = ident;
}

static void tac_new(tac_context *context, new_node *new,
                    ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);
    tac_instr instr = {
        .kind = TAC_ASSIGN_NEW,
        .assign_new =
            {
                .ident = ident,
                .type = new->type.value,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
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
    tac_instr instr = {
        .kind = kind,
        .assign_binary =
            {
                .ident = ident,
                .lhs = lhs.ident.name,
                .rhs = rhs.ident.name,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
}

static void tac_unary(tac_context *context, expr_unary_node *unary,
                      enum tac_kind kind, ds_dynamic_array *instrs,
                      tac_instr *result) {
    tac_instr expr;
    tac_expr(context, unary->expr, instrs, &expr);

    char *ident;
    tac_new_var(context, &ident);
    tac_instr instr = {
        .kind = kind,
        .assign_unary =
            {
                .ident = ident,
                .expr = expr.ident.name,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
}

static void tac_paren(tac_context *context, expr_node *paren,
                      ds_dynamic_array *instrs, tac_instr *result) {
    tac_expr(context, paren, instrs, result);
}

static void tac_identifier(tac_context *context, node_info *ident,
                           ds_dynamic_array *instrs, tac_instr *result) {
    result->kind = TAC_IDENT;
    result->ident.name = ident->value;
}

static void tac_int(tac_context *context, node_info *int_node,
                    ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);
    tac_instr instr = {
        .kind = TAC_ASSIGN_INT,
        .assign_int =
            {
                .ident = ident,
                .value = atoi(int_node->value),
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
}

static void tac_string(tac_context *context, node_info *string,
                       ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);
    tac_instr instr = {
        .kind = TAC_ASSIGN_STRING,
        .assign_string =
            {
                .ident = ident,
                .value = string->value,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
}

static void tac_bool(tac_context *context, node_info *boolean,
                     ds_dynamic_array *instrs, tac_instr *result) {
    char *ident;
    tac_new_var(context, &ident);
    tac_instr instr = {
        .kind = TAC_ASSIGN_BOOL,
        .assign_bool =
            {
                .ident = ident,
                .value = strcmp(boolean->value, "true") == 0 ? 1 : 0,
            },
    };
    ds_dynamic_array_append(instrs, &instr);

    result->kind = TAC_IDENT;
    result->ident.name = ident;
}

static void tac_expr(tac_context *context, expr_node *expr,
                     ds_dynamic_array *instrs, tac_instr *result) {
    switch (expr->kind) {
    case EXPR_ASSIGN:
        return tac_assign(context, &expr->assign, instrs, result);
    case EXPR_DISPATCH_FULL:
        return tac_dispatch_full(context, &expr->dispatch_full, instrs, result);
    case EXPR_DISPATCH:
        return tac_dispatch(context, &expr->dispatch, instrs, result);
    case EXPR_COND:
        return tac_cond(context, &expr->cond, instrs, result);
    case EXPR_LOOP:
        return tac_loop(context, &expr->loop, instrs, result);
    case EXPR_BLOCK:
        return tac_block(context, &expr->block, instrs, result);
    case EXPR_LET:
        return tac_let(context, &expr->let, instrs, result);
    case EXPR_CASE:
        return tac_case(context, &expr->case_, instrs, result);
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
        return tac_identifier(context, &expr->ident, instrs, result);
    case EXPR_INT:
        return tac_int(context, &expr->integer, instrs, result);
    case EXPR_STRING:
        return tac_string(context, &expr->string, instrs, result);
    case EXPR_BOOL:
        return tac_bool(context, &expr->boolean, instrs, result);
    default:
        break;
    }
}

int codegen_expr_to_tac(const expr_node *expr, tac_result *tac) {
    tac_context context = {.result = 0, .temp_count = 0, .label_count = 0};
    ds_dynamic_array_init(&context.locals, sizeof(char *));

    ds_dynamic_array_init(&tac->instrs, sizeof(tac_instr));
    tac_instr result;

    tac_expr(&context, (expr_node *)expr, &tac->instrs, &result);

    ds_dynamic_array_append(&tac->instrs, &result);
    ds_dynamic_array_copy(&context.locals, &tac->locals);

    return context.result;
}
