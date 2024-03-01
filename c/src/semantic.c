#include "semantic.h"
#include "ds.h"
#include <stdarg.h>
#include <stdio.h>

static void context_show_errorf(program_context *context, unsigned int line,
                                unsigned int col, const char *format, ...) {
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

// Error messages

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

#define attribute_illegal_error(context, class, attribute)                     \
    context_show_errorf(context, attribute.line, attribute.col,                \
                        "Class %s has attribute with illegal name %s",         \
                        class.value, attribute.value)

#define attribute_redefined_error(context, class, attribute)                   \
    context_show_errorf(context, attribute.line, attribute.col,                \
                        "Class %s redefines attribute %s", class.value,        \
                        attribute.value)

#define attribute_undefined_type_error(context, class, attr, type)             \
    context_show_errorf(context, type.line, type.col,                          \
                        "Class %s has attribute %s with undefined type %s",    \
                        class.value, attr.value, type.value)

#define attribute_redefined_parent_error(context, class, attribute)            \
    context_show_errorf(context, attribute.line, attribute.col,                \
                        "Class %s redefines inherited attribute %s",           \
                        class.value, attribute.value)

#define method_redefined_error(context, class, method)                         \
    context_show_errorf(context, method.line, method.col,                      \
                        "Class %s redefines method %s", class.value,           \
                        method.value)

#define formal_illegal_error(context, class, method, formal)                   \
    context_show_errorf(context, formal.line, formal.col,                      \
                        "Method %s of class %s has formal parameter with "     \
                        "illegal name %s",                                     \
                        method.value, class.value, formal.value)

#define formal_undefined_type_error(context, class, method, name, type)        \
    context_show_errorf(context, type.line, type.col,                          \
                        "Method %s of class %s has formal parameter %s with "  \
                        "undefined type %s",                                   \
                        method.value, class.value, name.value, type.value)

#define formal_type_illegal_error(context, class, method, name, type)          \
    context_show_errorf(context, type.line, type.col,                          \
                        "Method %s of class %s has formal parameter %s with "  \
                        "illegal type %s",                                     \
                        method.value, class.value, name.value, type.value)

#define formal_redefined_error(context, class, method, formal)                 \
    context_show_errorf(context, formal.line, formal.col,                      \
                        "Method %s of class %s redefines formal parameter "    \
                        "%s",                                                  \
                        method.value, class.value, formal.value)

#define formals_count_error(context, class, method)                            \
    context_show_errorf(context, method.line, method.col,                      \
                        "Class %s overrides method %s with different number "  \
                        "of formal parameters",                                \
                        class.value, method.value)

#define formal_type_mismatch_error(context, class, method, name, type1, type2) \
    context_show_errorf(context, type1.line, type1.col,                        \
                        "Class %s overrides method %s but changes type of "    \
                        "formal parameter %s from %s to %s",                   \
                        class.value, method.value, name.value, type2.value,    \
                        type1.value)

#define return_type_missmatch_error(context, class, method, type1, type2)      \
    context_show_errorf(context, type1.line, type1.col,                        \
                        "Class %s overrides method %s but changes return "     \
                        "type from %s to %s",                                  \
                        class.value, method.value, type2.value, type1.value)

#define return_type_undefined_error(context, class, method, type)              \
    context_show_errorf(context, type.line, type.col,                          \
                        "Class %s has method %s with undefined return type "   \
                        "%s",                                                  \
                        class.value, method.value, type.value)

// Error checking

static int is_class_redefined(program_context *context, const char *name) {
    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context class;
        ds_dynamic_array_get(&context->classes, i, &class);

        if (strcmp(class.name.value, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int is_class_invalid(const char *name) {
    if (strcmp(name, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

static int is_parent_invalid(const char *name) {
    if (strcmp(name, "Int") == 0 || strcmp(name, "String") == 0 ||
        strcmp(name, "Bool") == 0 || strcmp(name, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

static int is_attribute_illegal(const char *name) {
    if (strcmp(name, "self") == 0) {
        return 1;
    }

    return 0;
}

static int is_attribute_redefined(class_context *class, const char *name) {
    for (unsigned int i = 0; i < class->attributes.count; i++) {
        typed_symbol attr;
        ds_dynamic_array_get(&class->attributes, i, &attr);

        if (strcmp(attr.name.value, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int is_method_redefined(class_context *class, const char *name) {
    for (unsigned int i = 0; i < class->methods.count; i++) {
        method_context method;
        ds_dynamic_array_get(&class->methods, i, &method);

        if (strcmp(method.name.value, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int is_formal_illegal(const char *name) {
    if (strcmp(name, "self") == 0) {
        return 1;
    }

    return 0;
}

static int is_formal_type_illegal(const char *name) {
    if (strcmp(name, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

static int is_formal_redefined(ds_dynamic_array formals, const char *name) {
    for (unsigned int i = 0; i < formals.count; i++) {
        typed_symbol formal;
        ds_dynamic_array_get(&formals, i, &formal);

        if (strcmp(formal.name.value, name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int is_formals_different_count(ds_dynamic_array formals1,
                                      ds_dynamic_array formals2) {
    return formals1.count != formals2.count;
}

static int is_formals_different_types(typed_symbol formal1,
                                      typed_symbol formal2) {
    return strcmp(formal1.type.value, formal2.type.value) != 0;
}

// Implementation

static void context_init(program_context *context) {
    ds_dynamic_array_init(&context->classes, sizeof(class_context));

    class_context object_class = {.name = {.value = "Object"}};
    ds_dynamic_array_append(&context->classes, &object_class);

    class_context *object_context = NULL;
    ds_dynamic_array_get_ref(&context->classes, 0, (void **)&object_context);

    class_context int_class = {.name = {.value = "Int"},
                               .parent = object_context};
    ds_dynamic_array_append(&context->classes, &int_class);

    class_context string_class = {.name = {.value = "String"},
                                  .parent = object_context};
    ds_dynamic_array_append(&context->classes, &string_class);

    class_context bool_class = {.name = {.value = "Bool"},
                                .parent = object_context};
    ds_dynamic_array_append(&context->classes, &bool_class);

    class_context io_class = {.name = {.value = "IO"},
                              .parent = object_context};
    ds_dynamic_array_append(&context->classes, &io_class);
}

static class_context *class_to_context(program_context *context, char *name) {
    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context *c = NULL;
        ds_dynamic_array_get_ref(&context->classes, i, (void **)&c);

        if (strcmp(c->name.value, name) == 0) {
            return c;
        }
    }

    return NULL;
}

static method_context *method_to_context(class_context *class, char *name) {
    for (unsigned int i = 0; i < class->methods.count; i++) {
        method_context *method = NULL;
        ds_dynamic_array_get_ref(&class->methods, i, (void **)&method);

        if (strcmp(method->name.value, name) == 0) {
            return method;
        }
    }

    return NULL;
}

static typed_symbol *attribute_to_info(class_context *class, char *name) {
    for (unsigned int i = 0; i < class->attributes.count; i++) {
        typed_symbol *attr = NULL;
        ds_dynamic_array_get_ref(&class->attributes, i, (void **)&attr);

        if (strcmp(attr->name.value, name) == 0) {
            return attr;
        }
    }

    return NULL;
}

static void build_class_hierarchy(program_node *program,
                                  program_context *context) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        if (is_class_redefined(context, node.name.value)) {
            class_redefined_error(context, node.name);
        } else if (is_class_invalid(node.name.value)) {
            class_invalid_error(context, node.name);
        } else {
            class_context class = {.name = node.name, .parent = NULL};
            ds_dynamic_array_init(&class.attributes, sizeof(typed_symbol));
            ds_dynamic_array_init(&class.methods, sizeof(method_context));
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
        class_context *class = class_to_context(context, name);

        if (class == NULL) {
            continue;
        }

        char *parent_name = node.superclass.value;
        if (parent_name == NULL) {
            parent_name = "Object";
        }

        if (is_parent_invalid(parent_name)) {
            class_illegal_parent_error(context, node.name, node.superclass);
            continue;
        }

        class_context *parent_class = class_to_context(context, parent_name);

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

static void build_class_attributes(program_node *program,
                                   program_context *context) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        char *name = node.name.value;
        class_context *class = class_to_context(context, name);

        if (class == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < node.attributes.count; j++) {
            attribute_node attr;
            ds_dynamic_array_get(&node.attributes, j, &attr);

            if (is_attribute_illegal(attr.name.value)) {
                attribute_illegal_error(context, node.name, attr.name);
                continue;
            }

            if (is_attribute_redefined(class, attr.name.value)) {
                attribute_redefined_error(context, node.name, attr.name);
                continue;
            }

            class_context *type = class_to_context(context, attr.type.value);
            if (type == NULL) {
                attribute_undefined_type_error(context, node.name, attr.name,
                                               attr.type);
                continue;
            }

            typed_symbol info = {
                .name = attr.name, .type = attr.type, .class = type};
            ds_dynamic_array_append(&class->attributes, &info);
        }
    }
}

static void check_parent_class_attributes(program_node *program,
                                          program_context *context) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        char *name = node.name.value;
        class_context *class = class_to_context(context, name);

        if (class == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class->attributes.count; j++) {
            typed_symbol attr;
            ds_dynamic_array_get(&class->attributes, j, &attr);

            class_context *parent = class->parent;
            while (parent != NULL) {
                typed_symbol *parent_attr =
                    attribute_to_info(parent, attr.name.value);
                if (parent_attr != NULL) {
                    attribute_redefined_parent_error(context, class->name,
                                                     attr.name);
                    break;
                }

                parent = parent->parent;
            }
        }
    }
}

static void build_class_methods(program_node *program,
                                program_context *context) {

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        char *name = node.name.value;
        class_context *class = class_to_context(context, name);

        if (class == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < node.methods.count; j++) {
            method_node method;
            ds_dynamic_array_get(&node.methods, j, &method);

            if (is_method_redefined(class, method.name.value)) {
                method_redefined_error(context, node.name, method.name);
                continue;
            }

            method_context info = {.name = method.name};
            ds_dynamic_array_init(&info.formals, sizeof(typed_symbol));

            for (unsigned int k = 0; k < method.formals.count; k++) {
                formal_node formal;
                ds_dynamic_array_get(&method.formals, k, &formal);

                if (is_formal_illegal(formal.name.value)) {
                    formal_illegal_error(context, class->name, method.name,
                                         formal.name);
                    continue;
                }

                if (is_formal_type_illegal(formal.type.value)) {
                    formal_type_illegal_error(context, class->name, method.name,
                                              formal.name, formal.type);
                    continue;
                }

                if (is_formal_redefined(info.formals, formal.name.value)) {
                    formal_redefined_error(context, class->name, method.name,
                                           formal.name);
                    continue;
                }

                class_context *type =
                    class_to_context(context, formal.type.value);
                if (type == NULL) {
                    formal_undefined_type_error(context, class->name,
                                                method.name, formal.name,
                                                formal.type);
                }

                typed_symbol formal_info = {
                    .name = formal.name, .type = formal.type, .class = type};
                ds_dynamic_array_append(&info.formals, &formal_info);
            }

            class_context *type = class_to_context(context, method.type.value);
            if (type == NULL) {
                return_type_undefined_error(context, class->name, method.name,
                                            method.type);
            }

            info.type = method.type;
            info.return_type = type;

            ds_dynamic_array_append(&class->methods, &info);
        }
    }
}

static void check_parent_class_methods(program_node *program,
                                       program_context *context) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node node;
        ds_dynamic_array_get(&program->classes, i, &node);

        char *name = node.name.value;
        class_context *class = class_to_context(context, name);

        if (class == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class->methods.count; j++) {
            method_context method;
            ds_dynamic_array_get(&class->methods, j, &method);

            class_context *parent = class->parent;
            while (parent != NULL) {
                method_context *parent_method =
                    method_to_context(parent, method.name.value);

                if (parent_method != NULL) {
                    if (is_formals_different_count(method.formals,
                                                   parent_method->formals)) {
                        formals_count_error(context, class->name, method.name);
                        break;
                    }

                    for (unsigned int k = 0; k < method.formals.count; k++) {
                        typed_symbol formal;
                        ds_dynamic_array_get(&method.formals, k, &formal);

                        typed_symbol parent_formal;
                        ds_dynamic_array_get(&parent_method->formals, k,
                                             &parent_formal);

                        if (is_formals_different_types(formal, parent_formal)) {
                            formal_type_mismatch_error(
                                context, class->name, method.name, formal.name,
                                formal.type, parent_formal.type);
                            break;
                        }
                    }

                    if (strcmp(method.type.value, parent_method->type.value) !=
                        0) {
                        return_type_missmatch_error(context, class->name,
                                                    method.name, method.type,
                                                    parent_method->type);
                    }

                    break;
                }

                parent = parent->parent;
            }
        }
    }
}

int semantic_check(program_node *program, program_context *context) {
    context->result = 0;

    context_init(context);

    build_class_hierarchy(program, context);
    build_parent_hierarchy(program, context);
    check_inheritance_cycle(context);

    build_class_attributes(program, context);
    check_parent_class_attributes(program, context);

    build_class_methods(program, context);
    check_parent_class_methods(program, context);

    return context->result;
}
