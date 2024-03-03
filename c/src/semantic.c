#include "semantic.h"
#include <stdarg.h>
#include <stdio.h>

static void context_show_errorf(semantic_context *context, unsigned int line,
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

static void find_class_ctx(semantic_context *context, const char *class_name,
                           class_context **class_ctx) {
    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context *ctx;
        ds_dynamic_array_get_ref(&context->classes, i, (void **)&ctx);

        if (strcmp(ctx->name, class_name) == 0) {
            *class_ctx = ctx;
            return;
        }
    }
}

static void find_method_ctx(class_context *class_ctx, const char *method_name,
                            method_context **method_ctx) {
    for (unsigned int i = 0; i < class_ctx->methods.count; i++) {
        method_context *ctx;
        ds_dynamic_array_get_ref(&class_ctx->methods, i, (void **)&ctx);

        if (strcmp(ctx->name, method_name) == 0) {
            *method_ctx = ctx;
            return;
        }
    }
}

static void find_object_ctx(class_context *class_ctx, const char *object_name,
                            object_context **object_ctx) {
    for (unsigned int i = 0; i < class_ctx->objects.count; i++) {
        object_context *ctx;
        ds_dynamic_array_get_ref(&class_ctx->objects, i, (void **)&ctx);

        if (strcmp(ctx->name, object_name) == 0) {
            *object_ctx = ctx;
            return;
        }
    }
}

static int is_class_redefined(semantic_context *context, class_node class) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, class.name.value, &class_ctx);
    return class_ctx != NULL;
}

#define context_show_error_class_redefined(context, class)                     \
    context_show_errorf(context, class.name.line, class.name.col,              \
                        "Class %s is redefined", class.name.value)

static int is_class_name_illegal(semantic_context *context, class_node class) {
    if (strcmp(class.name.value, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_class_name_illegal(context, class)                  \
    context_show_errorf(context, class.name.line, class.name.col,              \
                        "Class has illegal name %s", class.name.value)

static int is_class_parent_undefined(semantic_context *context,
                                     class_node class) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, class.superclass.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_class_parent_undefined(context, class)              \
    context_show_errorf(context, class.superclass.line, class.superclass.col,  \
                        "Class %s has undefined parent %s", class.name.value,  \
                        class.superclass.value)

static int is_class_parent_illegal(semantic_context *context,
                                   class_node class) {
    if (strcmp(class.superclass.value, "Int") == 0 ||
        strcmp(class.superclass.value, "String") == 0 ||
        strcmp(class.superclass.value, "Bool") == 0 ||
        strcmp(class.superclass.value, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_class_parent_illegal(context, class)                \
    context_show_errorf(context, class.superclass.line, class.superclass.col,  \
                        "Class %s has illegal parent %s", class.name.value,    \
                        class.superclass.value)

static int is_class_inheritance_cycle(semantic_context *context,
                                      class_context *class_ctx) {

    class_context *parent_ctx = class_ctx->parent;
    while (parent_ctx != NULL) {
        if (strcmp(parent_ctx->name, class_ctx->name) == 0) {
            return 1;
        }

        parent_ctx = parent_ctx->parent;
    }

    return 0;
}

#define context_show_error_class_inheritance(context, class)                   \
    context_show_errorf(context, class.name.line, class.name.col,              \
                        "Inheritance cycle for class %s", class.name.value)

static void semantic_check_classes(semantic_context *context,
                                   program_node *program) {
    ds_dynamic_array_init(&context->classes, sizeof(class_context));

    class_context object_instance = {.name = "Object", .parent = NULL};
    ds_dynamic_array_init(&object_instance.objects, sizeof(object_context));
    ds_dynamic_array_init(&object_instance.methods, sizeof(method_context));
    ds_dynamic_array_append(&context->classes, &object_instance);

    class_context *object = NULL;
    find_class_ctx(context, "Object", &object);

    method_context abort = {.name = "abort", .type = "Object"};
    ds_dynamic_array_init(&abort.formals, sizeof(object_context));
    ds_dynamic_array_append(&object->methods, &abort);

    method_context type_name = {.name = "type_name", .type = "String"};
    ds_dynamic_array_init(&type_name.formals, sizeof(object_context));
    ds_dynamic_array_append(&object->methods, &type_name);

    method_context copy = {.name = "copy", .type = "SELF_TYPE"};
    ds_dynamic_array_init(&copy.formals, sizeof(object_context));
    ds_dynamic_array_append(&object->methods, &copy);

    class_context string = {.name = "String", .parent = object};
    ds_dynamic_array_init(&string.objects, sizeof(object_context));
    ds_dynamic_array_init(&string.methods, sizeof(method_context));
    ds_dynamic_array_append(&context->classes, &string);

    method_context length = {.name = "length", .type = "Int"};
    ds_dynamic_array_init(&length.formals, sizeof(object_context));
    ds_dynamic_array_append(&string.methods, &length);

    method_context concat = {.name = "concat", .type = "String"};
    ds_dynamic_array_init(&concat.formals, sizeof(object_context));
    object_context concat_formal = {.name = "s", .type = "String"};
    ds_dynamic_array_append(&concat.formals, &concat_formal);
    ds_dynamic_array_append(&string.methods, &concat);

    method_context substr = {.name = "substr", .type = "String"};
    ds_dynamic_array_init(&substr.formals, sizeof(object_context));
    object_context substr_formal1 = {.name = "i", .type = "Int"};
    object_context substr_formal2 = {.name = "l", .type = "Int"};
    ds_dynamic_array_append(&substr.formals, &substr_formal1);
    ds_dynamic_array_append(&substr.formals, &substr_formal2);
    ds_dynamic_array_append(&string.methods, &substr);

    class_context int_class = {.name = "Int", .parent = object};
    ds_dynamic_array_init(&int_class.objects, sizeof(object_context));
    ds_dynamic_array_init(&int_class.methods, sizeof(method_context));
    ds_dynamic_array_append(&context->classes, &int_class);

    class_context bool_class = {.name = "Bool", .parent = object};
    ds_dynamic_array_init(&bool_class.objects, sizeof(object_context));
    ds_dynamic_array_init(&bool_class.methods, sizeof(method_context));
    ds_dynamic_array_append(&context->classes, &bool_class);

    class_context io = {.name = "IO", .parent = object};
    ds_dynamic_array_init(&io.objects, sizeof(object_context));
    ds_dynamic_array_init(&io.methods, sizeof(method_context));
    ds_dynamic_array_append(&context->classes, &io);

    method_context out_string = {.name = "out_string", .type = "SELF_TYPE"};
    ds_dynamic_array_init(&out_string.formals, sizeof(object_context));
    object_context out_string_formal = {.name = "x", .type = "String"};
    ds_dynamic_array_append(&out_string.formals, &out_string_formal);
    ds_dynamic_array_append(&io.methods, &out_string);

    method_context out_int = {.name = "out_int", .type = "SELF_TYPE"};
    ds_dynamic_array_init(&out_int.formals, sizeof(object_context));
    object_context out_int_formal = {.name = "x", .type = "Int"};
    ds_dynamic_array_append(&out_int.formals, &out_int_formal);
    ds_dynamic_array_append(&io.methods, &out_int);

    method_context in_string = {.name = "in_string", .type = "String"};
    ds_dynamic_array_init(&in_string.formals, sizeof(object_context));
    ds_dynamic_array_append(&io.methods, &in_string);

    method_context in_int = {.name = "in_int", .type = "Int"};
    ds_dynamic_array_init(&in_int.formals, sizeof(object_context));
    ds_dynamic_array_append(&io.methods, &in_int);

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        if (is_class_name_illegal(context, class)) {
            context_show_error_class_name_illegal(context, class);
            continue;
        }

        if (is_class_redefined(context, class)) {
            context_show_error_class_redefined(context, class);
            continue;
        }

        class_context class_ctx = {.name = class.name.value, .parent = NULL};
        ds_dynamic_array_init(&class_ctx.objects, sizeof(object_context));
        ds_dynamic_array_init(&class_ctx.methods, sizeof(method_context));
        ds_dynamic_array_append(&context->classes, &class_ctx);
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        if (class.superclass.value == NULL) {
            class_ctx->parent = object;
            continue;
        }

        if (is_class_parent_illegal(context, class)) {
            context_show_error_class_parent_illegal(context, class);
            continue;
        }

        if (is_class_parent_undefined(context, class)) {
            context_show_error_class_parent_undefined(context, class);
            continue;
        }

        class_context *parent_ctx = NULL;
        find_class_ctx(context, class.superclass.value, &parent_ctx);
        class_ctx->parent = parent_ctx;
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        if (is_class_inheritance_cycle(context, class_ctx)) {
            context_show_error_class_inheritance(context, class);
            continue;
        }
    }
}

static int is_attribute_name_illegal(semantic_context *context,
                                     attribute_node attribute) {
    if (strcmp(attribute.name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_attribute_name_illegal(context, class, attribute)   \
    context_show_errorf(context, attribute.name.line, attribute.name.col,      \
                        "Class %s has attribute with illegal name %s",         \
                        class.name.value, attribute.name.value)

static int is_attribute_redefined(semantic_context *context,
                                  class_context *class_ctx,
                                  attribute_node attribute) {
    object_context *object_ctx = NULL;
    find_object_ctx(class_ctx, attribute.name.value, &object_ctx);
    return object_ctx != NULL;
}

#define context_show_error_attribute_redefined(context, class, attribute)      \
    context_show_errorf(context, attribute.name.line, attribute.name.col,      \
                        "Class %s redefines attribute %s", class.name.value,   \
                        attribute.name.value)

static int is_attribute_type_undefiend(semantic_context *context,
                                       attribute_node attribute) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, attribute.type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_attribute_type_undefined(context, class, attribute) \
    context_show_errorf(context, attribute.type.line, attribute.type.col,      \
                        "Class %s has attribute %s with undefined type %s",    \
                        class.name.value, attribute.name.value,                \
                        attribute.type.value)

static int is_attribute_parent_redefined(semantic_context *context,
                                         class_context *class_ctx,
                                         attribute_node attribute) {
    class_context *parent_ctx = class_ctx->parent;
    while (parent_ctx != NULL) {
        object_context *object_ctx = NULL;
        find_object_ctx(parent_ctx, attribute.name.value, &object_ctx);
        if (object_ctx != NULL) {
            return 1;
        }

        parent_ctx = parent_ctx->parent;
    }

    return 0;
}

#define context_show_error_attribute_parent_redefined(context, class,          \
                                                      attribute)               \
    context_show_errorf(context, attribute.name.line, attribute.name.col,      \
                        "Class %s redefines inherited attribute %s",           \
                        class.name.value, attribute.name.value)

static void semantic_check_attributes(semantic_context *context,
                                      program_node *program) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class.attributes.count; j++) {
            attribute_node attribute;
            ds_dynamic_array_get(&class.attributes, j, &attribute);

            if (is_attribute_name_illegal(context, attribute)) {
                context_show_error_attribute_name_illegal(context, class,
                                                          attribute);
                continue;
            }

            if (is_attribute_redefined(context, class_ctx, attribute)) {
                context_show_error_attribute_redefined(context, class,
                                                       attribute);
                continue;
            }

            if (is_attribute_type_undefiend(context, attribute)) {
                context_show_error_attribute_type_undefined(context, class,
                                                            attribute);
                continue;
            }

            object_context object = {.name = attribute.name.value,
                                     .type = attribute.type.value};
            ds_dynamic_array_append(&class_ctx->objects, &object);
        }
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class.attributes.count; j++) {
            attribute_node attribute;
            ds_dynamic_array_get(&class.attributes, j, &attribute);

            if (is_attribute_parent_redefined(context, class_ctx, attribute)) {
                context_show_error_attribute_parent_redefined(context, class,
                                                              attribute);
                continue;
            }
        }
    }
}

static int is_method_redefined(semantic_context *context,
                               class_context *class_ctx, method_node method) {
    method_context *method_ctx = NULL;
    find_method_ctx(class_ctx, method.name.value, &method_ctx);
    return method_ctx != NULL;
}

#define context_show_error_method_redefined(context, class, method)            \
    context_show_errorf(context, method.name.line, method.name.col,            \
                        "Class %s redefines method %s", class.name.value,      \
                        method.name.value)

static int is_formal_name_illegal(semantic_context *context,
                                  formal_node formal) {
    if (strcmp(formal.name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_formal_name_illegal(context, class, method, formal) \
    context_show_errorf(context, formal.name.line, formal.name.col,            \
                        "Method %s of class %s has formal parameter with "     \
                        "illegal name %s",                                     \
                        method.name.value, class.name.value,                   \
                        formal.name.value)

static int is_formal_type_illegal(semantic_context *context,
                                  formal_node formal) {
    if (strcmp(formal.type.value, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_formal_type_illegal(context, class, method, formal) \
    context_show_errorf(context, formal.type.line, formal.type.col,            \
                        "Method %s of class %s has formal parameter %s with "  \
                        "illegal type %s",                                     \
                        method.name.value, class.name.value,                   \
                        formal.name.value, formal.type.value)

static int is_formal_redefined(semantic_context *context, method_context method,
                               formal_node formal) {
    for (unsigned int i = 0; i < method.formals.count; i++) {
        object_context object;
        ds_dynamic_array_get(&method.formals, i, &object);

        if (strcmp(object.name, formal.name.value) == 0) {
            return 1;
        }
    }

    return 0;
}

#define context_show_error_formal_redefined(context, class, method, formal)    \
    context_show_errorf(context, formal.name.line, formal.name.col,            \
                        "Method %s of class %s redefines formal parameter %s", \
                        method.name.value, class.name.value,                   \
                        formal.name.value)

static int is_formal_type_undefiend(semantic_context *context,
                                    formal_node formal) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, formal.type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_formal_type_undefined(context, class, method,       \
                                                 formal)                       \
    context_show_errorf(context, formal.type.line, formal.type.col,            \
                        "Method %s of class %s has formal parameter %s with "  \
                        "undefined type %s",                                   \
                        method.name.value, class.name.value,                   \
                        formal.name.value, formal.type.value)

static int is_return_type_undefiend(semantic_context *context,
                                    method_node method) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, method.type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_return_type_undefined(context, class, method)       \
    context_show_errorf(context, method.type.line, method.type.col,            \
                        "Method %s of class %s has undefined return type %s",  \
                        method.name.value, class.name.value,                   \
                        method.type.value)

static int is_formals_different_count(method_context *method_ctx,
                                      unsigned int formals_count) {
    return method_ctx->formals.count != formals_count;
}

#define context_show_error_formals_different_count(context, class, method)     \
    context_show_errorf(context, method.name.line, method.name.col,            \
                        "Class %s overrides method %s with different number "  \
                        "of formal parameters",                                \
                        class.name.value, method.name.value)

static int is_formals_different_types(object_context parent_formal,
                                      formal_node formal) {
    return strcmp(parent_formal.type, formal.type.value) != 0;
}

#define context_show_error_formals_different_types(context, class, method,     \
                                                   parent_formal, formal)      \
    context_show_errorf(context, formal.type.line, formal.type.col,            \
                        "Class %s overrides method %s but changes type of "    \
                        "formal parameter %s from %s to %s",                   \
                        class.name.value, method.name.value,                   \
                        formal.name.value, parent_formal.type,                 \
                        formal.type.value)

static int is_return_type_different(method_context *parent_method_ctx,
                                    method_node method) {
    return strcmp(parent_method_ctx->type, method.type.value) != 0;
}

#define context_show_error_return_type_different(context, class, method)       \
    context_show_errorf(context, method.type.line, method.type.col,            \
                        "Class %s overrides method %s but changes return "     \
                        "type from %s to %s",                                  \
                        class.name.value, method.name.value,                   \
                        parent_method_ctx->type, method.type.value)

static void semantic_check_methods(semantic_context *context,
                                   program_node *program) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class.methods.count; j++) {
            method_node method;
            ds_dynamic_array_get(&class.methods, j, &method);

            if (is_method_redefined(context, class_ctx, method)) {
                context_show_error_method_redefined(context, class, method);
                continue;
            }

            method_context method_ctx = {.name = method.name.value};
            ds_dynamic_array_init(&method_ctx.formals, sizeof(object_context));

            for (unsigned int k = 0; k < method.formals.count; k++) {
                formal_node formal;
                ds_dynamic_array_get(&method.formals, k, &formal);

                if (is_formal_name_illegal(context, formal)) {
                    context_show_error_formal_name_illegal(context, class,
                                                           method, formal);
                    continue;
                }

                if (is_formal_type_illegal(context, formal)) {
                    context_show_error_formal_type_illegal(context, class,
                                                           method, formal);
                    continue;
                }

                if (is_formal_redefined(context, method_ctx, formal)) {
                    context_show_error_formal_redefined(context, class, method,
                                                        formal);
                    continue;
                }

                if (is_formal_type_undefiend(context, formal)) {
                    context_show_error_formal_type_undefined(context, class,
                                                             method, formal);
                    continue;
                }

                object_context object = {.name = formal.name.value,
                                         .type = formal.type.value};
                ds_dynamic_array_append(&method_ctx.formals, &object);
            }

            if (is_return_type_undefiend(context, method)) {
                context_show_error_return_type_undefined(context, class,
                                                         method);
                continue;
            }

            method_ctx.type = method.type.value;

            ds_dynamic_array_append(&class_ctx->methods, &method_ctx);
        }
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class.methods.count; j++) {
            method_node method;
            ds_dynamic_array_get(&class.methods, j, &method);

            method_context *method_ctx = NULL;
            ds_dynamic_array_get_ref(&class_ctx->methods, i,
                                     (void **)&method_ctx);

            if (method_ctx == NULL) {
                continue;
            }

            class_context *parent_ctx = class_ctx->parent;
            while (parent_ctx != NULL) {
                method_context *parent_method_ctx = NULL;
                find_method_ctx(parent_ctx, method.name.value,
                                &parent_method_ctx);

                if (parent_method_ctx != NULL) {
                    if (is_formals_different_count(parent_method_ctx,
                                                   method.formals.count)) {
                        context_show_error_formals_different_count(
                            context, class, method);
                        break;
                    }

                    for (unsigned int k = 0; k < method.formals.count; k++) {
                        formal_node formal;
                        ds_dynamic_array_get(&method.formals, k, &formal);

                        object_context parent_formal;
                        ds_dynamic_array_get(&parent_method_ctx->formals, k,
                                             &parent_formal);

                        if (is_formals_different_types(parent_formal, formal)) {
                            context_show_error_formals_different_types(
                                context, class, method, parent_formal, formal);
                            break;
                        }
                    }

                    if (is_return_type_different(parent_method_ctx, method)) {
                        context_show_error_return_type_different(context, class,
                                                                 method);
                        break;
                    }
                }

                parent_ctx = parent_ctx->parent;
            }
        }
    }
}

typedef struct method_environment_item {
        const char *class_name;
        const char *method_name;
        ds_dynamic_array formals; // const char *
} method_environment_item;

typedef struct method_environment {
        ds_dynamic_array items; // method_environment_item
} method_environment;

static void method_env_show(method_environment method_env) {
    for (unsigned int i = 0; i < method_env.items.count; i++) {
        method_environment_item item;
        ds_dynamic_array_get(&method_env.items, i, &item);

        printf("M(%s, %s) = (", item.class_name, item.method_name);
        for (unsigned int m = 0; m < item.formals.count; m++) {
            const char *formal_type;
            ds_dynamic_array_get(&item.formals, m, &formal_type);

            printf("%s", formal_type);
            if (m < item.formals.count - 1) {
                printf(", ");
            }
        }
        printf(")\n");
    }
}

static void build_method_environment(semantic_context *context,
                                     program_node *program,
                                     method_environment *env) {
    ds_dynamic_array_init(&env->items, sizeof(method_environment_item));

    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context *class_ctx = NULL;
        ds_dynamic_array_get_ref(&context->classes, i, (void **)&class_ctx);

        const char *class_name = class_ctx->name;

        for (unsigned int k = 0; k < class_ctx->methods.count; k++) {
            method_context *method_ctx;
            ds_dynamic_array_get_ref(&class_ctx->methods, k,
                                     (void **)&method_ctx);

            const char *method_name = method_ctx->name;

            method_environment_item item = {.class_name = class_name,
                                            .method_name = method_name};
            ds_dynamic_array_init(&item.formals, sizeof(const char *));

            for (unsigned int m = 0; m < method_ctx->formals.count; m++) {
                object_context formal_ctx;
                ds_dynamic_array_get(&method_ctx->formals, m, &formal_ctx);

                ds_dynamic_array_append(&item.formals, &formal_ctx.type);
            }

            ds_dynamic_array_append(&item.formals, &method_ctx->type);
            ds_dynamic_array_append(&env->items, &item);
        }
    }
}

typedef struct object_environment_item {
        const char *class_name;
        ds_dynamic_array objects; // object_context
} object_environment_item;

typedef struct object_environment {
        ds_dynamic_array items; // object_environment_item
} object_environment;

static void object_env_show(object_environment_item item) {
    printf("class %s\n", item.class_name);
    for (unsigned int m = 0; m < item.objects.count; m++) {
        object_context object;
        ds_dynamic_array_get(&item.objects, m, &object);

        printf("O(%s) = %s\n", object.name, object.type);
    }
}

static void build_object_environment(semantic_context *context,
                                     program_node *program,
                                     object_environment *env) {
    ds_dynamic_array_init(&env->items, sizeof(object_environment_item));

    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context *class_ctx = NULL;
        ds_dynamic_array_get_ref(&context->classes, i, (void **)&class_ctx);

        const char *class_name = class_ctx->name;

        object_environment_item item = {.class_name = class_name};
        ds_dynamic_array_init(&item.objects, sizeof(object_context));

        class_context *current_ctx = class_ctx;
        do {
            for (unsigned int j = 0; j < current_ctx->objects.count; j++) {
                object_context attribute_ctx;
                ds_dynamic_array_get(&current_ctx->objects, j, &attribute_ctx);

                ds_dynamic_array_append(&item.objects, &attribute_ctx);
            }

            current_ctx = current_ctx->parent;
        } while (current_ctx != NULL && class_ctx != current_ctx);

        ds_dynamic_array_append(&env->items, &item);
    }
}

static void get_object_environment(object_environment *env,
                                   const char *class_name,
                                   object_environment_item *item) {
    for (unsigned int i = 0; i < env->items.count; i++) {
        object_environment_item *env_item;
        ds_dynamic_array_get_ref(&env->items, i, (void **)&env_item);

        if (strcmp(env_item->class_name, class_name) == 0) {
            (*item).class_name = env_item->class_name;
            ds_dynamic_array_copy(&env_item->objects, &(*item).objects);
            return;
        }
    }
}

static void semantic_check_expression(semantic_context *context,
                                      expr_node *expr,
                                      method_environment *method_env,
                                      object_environment_item *object_env);

static int is_let_init_name_illegal(semantic_context *context,
                                    let_init_node init) {
    if (strcmp(init.name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_let_init_name_illegal(context, init)                \
    context_show_errorf(context, init.name.line, init.name.col,                \
                        "Let variable has illegal name %s", init.name.value)

static int is_let_init_type_undefined(semantic_context *context,
                                      let_init_node init) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, init.type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_let_init_type_undefined(context, init)              \
    context_show_errorf(context, init.type.line, init.type.col,                \
                        "Let variable %s has undefined type %s",               \
                        init.name.value, init.type.value)

static void semantic_check_let_expression(semantic_context *context,
                                          let_node *expr,
                                          method_environment *method_env,
                                          object_environment_item *object_env) {

    unsigned int depth = 0;
    for (unsigned int i = 0; i < expr->inits.count; i++) {
        let_init_node init;
        ds_dynamic_array_get(&expr->inits, i, &init);

        if (is_let_init_name_illegal(context, init)) {
            context_show_error_let_init_name_illegal(context, init);
            continue;
        }

        if (is_let_init_type_undefined(context, init)) {
            context_show_error_let_init_type_undefined(context, init);
            continue;
        }

        object_context object = {.name = init.name.value,
                                 .type = init.type.value};
        ds_dynamic_array_append(&object_env->objects, &object);

        depth++;
    }

    semantic_check_expression(context, expr->body, method_env, object_env);

    for (unsigned int i = 0; i < depth; i++) {
        ds_dynamic_array_pop(&object_env->objects, NULL);
    }
}

static int is_case_variable_name_illegal(semantic_context *context,
                                         branch_node branch) {
    if (strcmp(branch.name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_case_variable_name_illegal(context, branch)         \
    context_show_errorf(context, branch.name.line, branch.name.col,            \
                        "Case variable has illegal name %s",                   \
                        branch.name.value)

static int is_case_variable_type_illegal(semantic_context *context,
                                         branch_node branch) {
    if (strcmp(branch.type.value, "SELF_TYPE") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_case_variable_type_illegal(context, branch)         \
    context_show_errorf(context, branch.type.line, branch.type.col,            \
                        "Case variable %s has illegal type %s",                \
                        branch.name.value, branch.type.value)

static int is_case_variable_type_undefined(semantic_context *context,
                                           branch_node branch) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, branch.type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_case_variable_type_undefined(context, branch)       \
    context_show_errorf(context, branch.type.line, branch.type.col,            \
                        "Case variable %s has undefined type %s",              \
                        branch.name.value, branch.type.value)

static void
semantic_check_case_expression(semantic_context *context, case_node *expr,
                               method_environment *method_env,
                               object_environment_item *object_env) {
    for (unsigned int i = 0; i < expr->cases.count; i++) {
        branch_node branch;
        ds_dynamic_array_get(&expr->cases, i, &branch);

        if (is_case_variable_name_illegal(context, branch)) {
            context_show_error_case_variable_name_illegal(context, branch);
            continue;
        }

        if (is_case_variable_type_illegal(context, branch)) {
            context_show_error_case_variable_type_illegal(context, branch);
            continue;
        }

        if (is_case_variable_type_undefined(context, branch)) {
            context_show_error_case_variable_type_undefined(context, branch);
            continue;
        }

        object_context object = {.name = branch.name.value,
                                 .type = branch.type.value};
        ds_dynamic_array_append(&object_env->objects, &object);

        semantic_check_expression(context, branch.body, method_env, object_env);

        ds_dynamic_array_pop(&object_env->objects, NULL);
    }
}

static void semantic_check_expression(semantic_context *context,
                                      expr_node *expr,
                                      method_environment *method_env,
                                      object_environment_item *object_env) {
    switch (expr->type) {
    case EXPR_NONE:
    case EXPR_ASSIGN:
        break;
    case EXPR_DISPATCH_FULL:
        break;
    case EXPR_DISPATCH:
        break;
    case EXPR_COND:
        break;
    case EXPR_LOOP:
        break;
    case EXPR_BLOCK:
        break;
    case EXPR_LET:
        return semantic_check_let_expression(context, &expr->let, method_env,
                                             object_env);
    case EXPR_CASE:
        return semantic_check_case_expression(context, &expr->case_, method_env,
                                              object_env);
    case EXPR_NEW:
        break;
    case EXPR_ISVOID:
        break;
    case EXPR_ADD:
        break;
    case EXPR_SUB:
        break;
    case EXPR_MUL:
        break;
    case EXPR_DIV:
        break;
    case EXPR_NEG:
        break;
    case EXPR_LT:
        break;
    case EXPR_LE:
        break;
    case EXPR_EQ:
        break;
    case EXPR_NOT:
        break;
    case EXPR_PAREN:
        break;
    case EXPR_IDENT:
        break;
    case EXPR_INT:
        break;
    case EXPR_STRING:
        break;
    case EXPR_BOOL:
        break;
    default:
        break;
    }

    // object_env_show(*object_env);
}

static void semantic_check_method_body(semantic_context *context,
                                       program_node *program,
                                       method_environment *method_env,
                                       object_environment *object_envs) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        object_environment_item object_env = {0};
        get_object_environment(object_envs, class.name.value, &object_env);

        for (unsigned int j = 0; j < class.methods.count; j++) {
            method_node method;
            ds_dynamic_array_get(&class.methods, j, &method);

            method_context *method_ctx = NULL;
            find_method_ctx(class_ctx, method.name.value, &method_ctx);

            if (method_ctx == NULL) {
                continue;
            }

            unsigned int depth = 0;
            for (unsigned int k = 0; k < method_ctx->formals.count; k++) {
                object_context formal_ctx;
                ds_dynamic_array_get(&method_ctx->formals, k, &formal_ctx);

                ds_dynamic_array_append(&object_env.objects, &formal_ctx);

                depth++;
            }

            expr_node body = method.body;
            semantic_check_expression(context, &body, method_env, &object_env);

            for (unsigned int k = 0; k < depth; k++) {
                ds_dynamic_array_pop(&object_env.objects, NULL);
            }
        }
    }
}

int semantic_check(program_node *program, semantic_context *context) {
    context->result = 0;

    semantic_check_classes(context, program);
    semantic_check_attributes(context, program);
    semantic_check_methods(context, program);

    object_environment object_env;
    build_object_environment(context, program, &object_env);

    method_environment method_env;
    build_method_environment(context, program, &method_env);

    semantic_check_method_body(context, program, &method_env, &object_env);

    /*for (unsigned int i = 0; i < object_env.items.count; i++) {
        object_environment_item item;
        ds_dynamic_array_get(&object_env.items, i, &item);

        object_env_show(item);
    }

    method_env_show(method_env);
    */

    return context->result;
}
