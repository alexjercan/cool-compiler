#include "print_ast.h"
#include <stdio.h>

static void attribute_print(attribute_node *attribute, unsigned int indent) {
    printf("%*sattribute\n", indent, "");
    printf("%*s%s\n", indent + INDENT_SIZE, "", attribute->name.value);
    printf("%*s%s\n", indent + INDENT_SIZE, "", attribute->type.value);
    if (attribute->value.value != NULL) {
        printf("%*s%s\n", indent + INDENT_SIZE, "", attribute->value.value);
    }
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
