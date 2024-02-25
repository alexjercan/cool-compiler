#include "print_ast.h"
#include <stdio.h>

static void expr_print(expr_node *expr, unsigned int indent) {
    switch (expr->type) {
        case EXPR_INT: {
            printf("%*s%s\n", indent, "", expr->integer.value);
            break;
        }
        case EXPR_BOOL: {
            printf("%*s%s\n", indent, "", expr->boolean.value);
            break;
        }
        case EXPR_STRING: {
            printf("%*s%s\n", indent, "", expr->string.value);
            break;
        }
        case EXPR_IDENT: {
            printf("%*s%s\n", indent, "", expr->ident.value);
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

void print_ast(program_node *program) { program_print(program, 0); }
