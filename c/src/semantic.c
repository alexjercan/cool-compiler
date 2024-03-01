#include "semantic.h"
#include "ds.h"
#include <stdarg.h>
#include <stdio.h>

static void context_show_errorf(struct program_context *context,
                                unsigned int line, unsigned int col,
                                const char *format, ...) {
    context->result = 1;

    const char *filename = context->filename;

    if (filename != NULL) {
        printf("\"%s\", ", filename);
    }

    printf("line %d:%d, Semantic error: ", line, col);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}

#define class_redefined_error(context, class)                                  \
    context_show_errorf(context, class.line, class.col,                        \
                        "Class %s is redefined", class.value)

#define class_invalid_error(context, class)                                    \
    context_show_errorf(context, class.line, class.col,                        \
                        "Class has illegal name %s", class.value)

#define class_illegal_parent_error(context, class, parent)                     \
    context_show_errorf(context, parent.line, parent.col,                      \
                        "Class %s has illegal parent %s", class.value, parent)

#define class_undefined_parent_error(context, class, parent)                   \
    context_show_errorf(context, parent.line, parent.col,                      \
                        "Class %s has undefined parent %s", class.value,       \
                        parent)

#define class_inheritance_cycle_error(context, class)                          \
    context_show_errorf(context, class.line, class.col,                        \
                        "Inheritance cycle for class %s", class.value)

static void context_init(program_context *context) {
    ds_dynamic_array_init(&context->classes, sizeof(struct class_context));

    struct class_context object_class = {.name = {.value = "Object"}};
    ds_dynamic_array_append(&context->classes, &object_class);

    struct class_context *object_context = NULL;
    ds_dynamic_array_get_ref(&context->classes, 0, (void **)&object_context);

    struct class_context int_class = {.name = {.value = "Int"},
                                      .parent = object_context};
    ds_dynamic_array_append(&context->classes, &int_class);

    struct class_context string_class = {.name = {.value = "String"},
                                         .parent = object_context};
    ds_dynamic_array_append(&context->classes, &string_class);

    struct class_context bool_class = {.name = {.value = "Bool"},
                                       .parent = object_context};
    ds_dynamic_array_append(&context->classes, &bool_class);

    struct class_context io_class = {.name = {.value = "IO"},
                                     .parent = object_context};
    ds_dynamic_array_append(&context->classes, &io_class);
}

static int is_class_redefined(program_context *context, const char *name) {
    for (unsigned int i = 0; i < context->classes.count; i++) {
        struct class_context class;
        ds_dynamic_array_get(&context->classes, i, &class);

        if (strcmp(class.name.value, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int is_class_invalid(program_context *context, const char *name) {
    if (strcmp(name, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

static int is_parent_invalid(program_context *context, const char *name) {
    if (strcmp(name, "Int") == 0 || strcmp(name, "String") == 0 ||
        strcmp(name, "Bool") == 0 || strcmp(name, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

static void build_class_hierarchy(program_node *program,
                                  program_context *context) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        if (is_class_redefined(context, node.name.value)) {
            class_redefined_error(context, node.name);
        } else if (is_class_invalid(context, node.name.value)) {
            class_invalid_error(context, node.name);
        } else {
            struct class_context class = {.name = node.name, .parent = NULL};
            ds_dynamic_array_append(&context->classes, &class);
        }
    }
}

static void build_parent_hierarchy(program_node *program,
                                   program_context *context) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        char *name = node.name.value;
        class_context *class = NULL;
        for (unsigned int j = 0; j < context->classes.count; j++) {
            class_context *c = NULL;
            ds_dynamic_array_get_ref(&context->classes, j, (void **)&c);

            if (strcmp(c->name.value, name) == 0) {
                class = c;
                break;
            }
        }

        if (class == NULL) {
            continue;
        }

        char *parent_name = node.superclass.value;
        if (parent_name == NULL) {
            parent_name = "Object";
        }

        if (is_parent_invalid(context, parent_name)) {
            class_illegal_parent_error(context, node.name, node.superclass);
            continue;
        }

        class_context *parent_class = NULL;
        for (unsigned int j = 0; j < context->classes.count; j++) {
            class_context *c = NULL;
            ds_dynamic_array_get_ref(&context->classes, j, (void **)&c);

            if (strcmp(c->name.value, parent_name) == 0) {
                parent_class = c;
                break;
            }
        }

        if (parent_class == NULL) {
            class_undefined_parent_error(context, node.name, node.superclass);
            continue;
        }

        class->parent = parent_class;
    }
}

static void check_inheritance_cycle(program_context *context) {
    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context *class = NULL;
        ds_dynamic_array_get_ref(&context->classes, i, (void **)&class);

        class_context *parent = class->parent;
        while (parent != NULL) {
            if (parent == class) {
                class_inheritance_cycle_error(context, class->name);
                break;
            }

            parent = parent->parent;
        }
    }
}

int semantic_check(program_node *program, program_context *context) {
    context->result = 0;

    context_init(context);

    build_class_hierarchy(program, context);
    build_parent_hierarchy(program, context);
    check_inheritance_cycle(context);

    return context->result;
}
