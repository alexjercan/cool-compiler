#include "parser.h"
#include <stdio.h>

static void expr_print(expr_node *expr, unsigned int indent);

static void local_print(let_init_node *init, unsigned int indent) {
    printf("%*slocal", indent, "");
    if (init->init != NULL && init->init->type != NULL) {
        printf(" : %s", init->init->type);
    }
    printf("\n");
    printf("%*s%s\n", indent + INDENT_SIZE, "", init->name.value);
    printf("%*s%s\n", indent + INDENT_SIZE, "", init->type.value);
    if (init->init != NULL) {
        expr_print(init->init, indent + INDENT_SIZE);
    }
}

static void branch_print(branch_node *branch, unsigned int indent) {
    printf("%*scase branch", indent, "");
    if (branch->body->type != NULL) {
        printf(" : %s", branch->body->type);
    }
    printf("\n");
    printf("%*s%s\n", indent + INDENT_SIZE, "", branch->name.value);
    printf("%*s%s\n", indent + INDENT_SIZE, "", branch->type.value);
    expr_print(branch->body, indent + INDENT_SIZE);
}

static void expr_print(expr_node *expr, unsigned int indent) {
    switch (expr->kind) {
    case EXPR_INT: {
        printf("%*s%s", indent, "", expr->integer.value);
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        break;
    }
    case EXPR_BOOL: {
        printf("%*s%s", indent, "", expr->boolean.value);
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        break;
    }
    case EXPR_STRING: {
        printf("%*s%s", indent, "", expr->string.value);
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        break;
    }
    case EXPR_IDENT: {
        printf("%*s%s", indent, "", expr->ident.value);
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        break;
    }
    case EXPR_ADD: {
        printf("%*s+", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->add.lhs, indent + INDENT_SIZE);
        expr_print(expr->add.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_SUB: {
        printf("%*s-", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->sub.lhs, indent + INDENT_SIZE);
        expr_print(expr->sub.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_MUL: {
        printf("%*s*", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->mul.lhs, indent + INDENT_SIZE);
        expr_print(expr->mul.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_DIV: {
        printf("%*s/", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->div.lhs, indent + INDENT_SIZE);
        expr_print(expr->div.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_NEG: {
        printf("%*s~", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->neg.expr, indent + INDENT_SIZE);
        break;
    }
    case EXPR_PAREN: {
        expr_print(expr->paren, indent);
        break;
    }
    case EXPR_LE: {
        printf("%*s<=", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->le.lhs, indent + INDENT_SIZE);
        expr_print(expr->le.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_LT: {
        printf("%*s<", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->lt.lhs, indent + INDENT_SIZE);
        expr_print(expr->lt.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_EQ: {
        printf("%*s=", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->eq.lhs, indent + INDENT_SIZE);
        expr_print(expr->eq.rhs, indent + INDENT_SIZE);
        break;
    }
    case EXPR_ASSIGN: {
        printf("%*s<-", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        printf("%*s%s\n", indent + INDENT_SIZE, "", expr->assign.name.value);
        expr_print(expr->assign.value, indent + INDENT_SIZE);
        break;
    }
    case EXPR_COND: {
        printf("%*sif", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->cond.predicate, indent + INDENT_SIZE);
        expr_print(expr->cond.then, indent + INDENT_SIZE);
        expr_print(expr->cond.else_, indent + INDENT_SIZE);
        break;
    }
    case EXPR_LOOP: {
        printf("%*swhile", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->loop.predicate, indent + INDENT_SIZE);
        expr_print(expr->loop.body, indent + INDENT_SIZE);
        break;
    }
    case EXPR_LET: {
        printf("%*slet", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        for (unsigned int i = 0; i < expr->let.inits.count; i++) {
            let_init_node init;
            ds_dynamic_array_get(&expr->let.inits, i, &init);
            local_print(&init, indent + INDENT_SIZE);
        }
        expr_print(expr->let.body, indent + INDENT_SIZE);
        break;
    }
    case EXPR_CASE: {
        printf("%*scase", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->case_.expr, indent + INDENT_SIZE);
        for (unsigned int i = 0; i < expr->case_.cases.count; i++) {
            branch_node branch;
            ds_dynamic_array_get(&expr->case_.cases, i, &branch);
            branch_print(&branch, indent + INDENT_SIZE);
        }
        break;
    }
    case EXPR_BLOCK: {
        printf("%*sblock", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        for (unsigned int i = 0; i < expr->block.exprs.count; i++) {
            expr_node subexpr;
            ds_dynamic_array_get(&expr->block.exprs, i, &subexpr);
            expr_print(&subexpr, indent + INDENT_SIZE);
        }
        break;
    }
    case EXPR_NEW: {
        printf("%*snew", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        printf("%*s%s\n", indent + INDENT_SIZE, "", expr->new.type.value);
        break;
    }
    case EXPR_ISVOID: {
        printf("%*sisvoid", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->isvoid.expr, indent + INDENT_SIZE);
        break;
    }
    case EXPR_NOT: {
        printf("%*snot", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->not_.expr, indent + INDENT_SIZE);
        break;
    }
    case EXPR_DISPATCH_FULL: {
        printf("%*s.", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        expr_print(expr->dispatch_full.expr, indent + INDENT_SIZE);
        if (expr->dispatch_full.type.value != NULL) {
            printf("%*s%s\n", indent + INDENT_SIZE, "",
                   expr->dispatch_full.type.value);
        }
        printf("%*s%s\n", indent + INDENT_SIZE, "",
               expr->dispatch_full.dispatch->method.value);
        for (unsigned int i = 0; i < expr->dispatch_full.dispatch->args.count;
             i++) {
            expr_node arg;
            ds_dynamic_array_get(&expr->dispatch_full.dispatch->args, i, &arg);
            expr_print(&arg, indent + INDENT_SIZE);
        }
        break;
    }
    case EXPR_DISPATCH: {
        printf("%*simplicit dispatch", indent, "");
        if (expr->type != NULL) {
            printf(" : %s", expr->type);
        }
        printf("\n");
        printf("%*s%s\n", indent + INDENT_SIZE, "",
               expr->dispatch.method.value);
        for (unsigned int i = 0; i < expr->dispatch.args.count; i++) {
            expr_node arg;
            ds_dynamic_array_get(&expr->dispatch.args, i, &arg);
            expr_print(&arg, indent + INDENT_SIZE);
        }
        break;
    }
    default: {
        break;
    }
    }
}

static void attribute_print(attribute_node *attribute, unsigned int indent) {
    printf("%*sattribute\n", indent, "");
    printf("%*s%s\n", indent + INDENT_SIZE, "", attribute->name.value);
    printf("%*s%s\n", indent + INDENT_SIZE, "", attribute->type.value);
    expr_print(&attribute->value, indent + INDENT_SIZE);
}

static void formal_print(formal_node *formal, unsigned int indent) {
    printf("%*sformal\n", indent, "");
    printf("%*s%s\n", indent + INDENT_SIZE, "", formal->name.value);
    printf("%*s%s\n", indent + INDENT_SIZE, "", formal->type.value);
}

static void method_print(method_node *method, unsigned int indent) {
    printf("%*smethod\n", indent, "");
    printf("%*s%s\n", indent + INDENT_SIZE, "", method->name.value);
    for (unsigned int i = 0; i < method->formals.count; i++) {
        formal_node formal;
        ds_dynamic_array_get(&method->formals, i, &formal);
        formal_print(&formal, indent + INDENT_SIZE);
    }
    printf("%*s%s\n", indent + INDENT_SIZE, "", method->type.value);
    expr_print(&method->body, indent + INDENT_SIZE);
}

static void class_print(class_node *class, unsigned int indent) {
    printf("%*sclass\n", indent, "");
    printf("%*s%s\n", indent + INDENT_SIZE, "", class->name.value);
    if (class->superclass.value != NULL) {
        printf("%*s%s\n", indent + INDENT_SIZE, "", class->superclass.value);
    }
    for (unsigned int i = 0; i < class->attributes.count; i++) {
        attribute_node attribute;
        ds_dynamic_array_get(&class->attributes, i, &attribute);
        attribute_print(&attribute, indent + INDENT_SIZE);
    }
    for (unsigned int i = 0; i < class->methods.count; i++) {
        method_node method;
        ds_dynamic_array_get(&class->methods, i, &method);
        method_print(&method, indent + INDENT_SIZE);
    }
}

static void program_print(program_node *program, unsigned int indent) {
    printf("%*sprogram\n", indent, "");
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);
        class_print(&class, indent + INDENT_SIZE);
    }
}

void parser_print_ast(program_node *program) { program_print(program, 0); }
