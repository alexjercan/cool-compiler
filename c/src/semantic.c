#include "semantic.h"
#include "ds.h"
#include <stdarg.h>
#include <stdio.h>

static unsigned int hash_string(const void *key) {
    const char *str = *(char **)key;
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

static int compare_string(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

typedef struct class_context {
        const char *name;
        struct class_context *parent;
        ds_hash_table objects;
        ds_hash_table methods;
} class_context;

typedef struct object_kv {
        const char *name;
        const char *type;
} object_kv;

typedef struct method_context {
        const char *name;
        const char *type;
        ds_dynamic_array formals;
} method_context;

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

static int is_class_redefined(semantic_context *context, class_node class) {
    return ds_hash_table_has(&context->classes, &class.name.value);
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
    return !ds_hash_table_has(&context->classes, &class.superclass.value);
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

static void semantic_check_class_nodes(semantic_context *context,
                                       program_node *program) {
    ds_hash_table_init(&context->classes, sizeof(char *), sizeof(class_context),
                       100, hash_string, compare_string);

    class_context object_instance = {.name = ("Object"), .parent = NULL};
    ds_hash_table_init(&object_instance.objects, sizeof(char *),
                       sizeof(object_kv), 100, hash_string, compare_string);
    ds_hash_table_init(&object_instance.methods, sizeof(char *),
                       sizeof(object_kv), 100, hash_string, compare_string);
    ds_hash_table_insert(&context->classes, &object_instance.name,
                         &object_instance);

    class_context *object = NULL;
    ds_hash_table_get_ref(&context->classes, &object_instance.name,
                          (void **)&object);

    class_context string = {.name = ("String"), .parent = object};
    ds_hash_table_init(&string.objects, sizeof(char *), sizeof(object_kv), 100,
                       hash_string, compare_string);
    ds_hash_table_init(&string.methods, sizeof(char *), sizeof(object_kv), 100,
                       hash_string, compare_string);
    ds_hash_table_insert(&context->classes, &string.name, &string);

    class_context int_class = {.name = ("Int"), .parent = object};
    ds_hash_table_init(&int_class.objects, sizeof(char *), sizeof(object_kv),
                       100, hash_string, compare_string);
    ds_hash_table_init(&int_class.methods, sizeof(char *), sizeof(object_kv),
                       100, hash_string, compare_string);
    ds_hash_table_insert(&context->classes, &int_class.name, &int_class);

    class_context bool_class = {.name = ("Bool"), .parent = object};
    ds_hash_table_init(&bool_class.objects, sizeof(char *), sizeof(object_kv),
                       100, hash_string, compare_string);
    ds_hash_table_init(&bool_class.methods, sizeof(char *), sizeof(object_kv),
                       100, hash_string, compare_string);
    ds_hash_table_insert(&context->classes, &bool_class.name, &bool_class);

    class_context io = {.name = ("IO"), .parent = object};
    ds_hash_table_init(&io.objects, sizeof(char *), sizeof(object_kv), 100,
                       hash_string, compare_string);
    ds_hash_table_init(&io.methods, sizeof(char *), sizeof(object_kv), 100,
                       hash_string, compare_string);
    ds_hash_table_insert(&context->classes, &io.name, &io);

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
        ds_hash_table_init(&class_ctx.objects, sizeof(char *),
                           sizeof(object_kv), 100, hash_string, compare_string);
        ds_hash_table_init(&class_ctx.methods, sizeof(char *),
                           sizeof(method_context), 100, hash_string,
                           compare_string);
        ds_hash_table_insert(&context->classes, &class_ctx.name, &class_ctx);
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        ds_hash_table_get_ref(&context->classes, &class.name.value,
                              (void **)&class_ctx);

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
        ds_hash_table_get_ref(&context->classes, &class.superclass.value,
                              (void **)&parent_ctx);

        class_ctx->parent = parent_ctx;
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        ds_hash_table_get_ref(&context->classes, &class.name.value,
                              (void **)&class_ctx);

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
    return ds_hash_table_has(&class_ctx->objects, &attribute.name.value);
}

#define context_show_error_attribute_redefined(context, class, attribute)      \
    context_show_errorf(context, attribute.name.line, attribute.name.col,      \
                        "Class %s redefines attribute %s", class.name.value,   \
                        attribute.name.value)

static int is_attribute_type_undefiend(semantic_context *context,
                                       attribute_node attribute) {
    return !ds_hash_table_has(&context->classes, &attribute.type.value);
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
        if (ds_hash_table_has(&parent_ctx->objects, &attribute.name.value)) {
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
        ds_hash_table_get_ref(&context->classes, &class.name.value,
                              (void **)&class_ctx);

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

            object_kv object = {.name = attribute.name.value,
                                .type = attribute.type.value};
            ds_hash_table_insert(&class_ctx->objects, &object.name, &object);
        }
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        ds_hash_table_get_ref(&context->classes, &class.name.value,
                              (void **)&class_ctx);

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
    return ds_hash_table_has(&class_ctx->methods, &method.name.value);
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
        object_kv object;
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
    return !ds_hash_table_has(&context->classes, &formal.type.value);
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
    return !ds_hash_table_has(&context->classes, &method.type.value);
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

static int is_formals_different_types(object_kv parent_formal,
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
        ds_hash_table_get_ref(&context->classes, &class.name.value,
                              (void **)&class_ctx);

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
            ds_dynamic_array_init(&method_ctx.formals, sizeof(object_kv));

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

                object_kv object = {.name = formal.name.value,
                                    .type = formal.type.value};
                ds_dynamic_array_append(&method_ctx.formals, &object);
            }

            if (is_return_type_undefiend(context, method)) {
                context_show_error_return_type_undefined(context, class,
                                                         method);
                continue;
            }

            method_ctx.type = method.type.value;

            ds_hash_table_insert(&class_ctx->methods, &method_ctx.name,
                                 &method_ctx);
        }
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);

        class_context *class_ctx = NULL;
        ds_hash_table_get_ref(&context->classes, &class.name.value,
                              (void **)&class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        for (unsigned int j = 0; j < class.methods.count; j++) {
            method_node method;
            ds_dynamic_array_get(&class.methods, j, &method);

            method_context *method_ctx = NULL;
            ds_hash_table_get_ref(&class_ctx->methods, &method.name.value,
                                  (void **)&method_ctx);

            if (method_ctx == NULL) {
                continue;
            }

            class_context *parent_ctx = class_ctx->parent;
            while (parent_ctx != NULL) {
                method_context *parent_method_ctx = NULL;
                ds_hash_table_get_ref(&parent_ctx->methods, &method.name.value,
                                      (void **)&parent_method_ctx);

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

                        object_kv parent_formal;
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

int semantic_check(program_node *program, semantic_context *context) {
    context->result = 0;

    semantic_check_class_nodes(context, program);
    semantic_check_attributes(context, program);
    semantic_check_methods(context, program);

    for (unsigned int i = 0; i < context->classes.capacity; i++) {
        ds_dynamic_array *keys = &context->classes.keys[i];
        ds_dynamic_array *values = &context->classes.values[i];

        for (unsigned int j = 0; j < keys->count; j++) {
            char *key;
            ds_dynamic_array_get(keys, j, &key);

            class_context value;
            ds_dynamic_array_get(values, j, &value);

            printf("Class: %s\n", key);
            if (value.parent != NULL) {
                printf("Parent: %s\n", value.parent->name);
            }
            printf("Objects:\n");
            for (unsigned int k = 0; k < value.objects.capacity; k++) {
                ds_dynamic_array *keys = &value.objects.keys[k];
                ds_dynamic_array *values = &value.objects.values[k];

                for (unsigned int l = 0; l < keys->count; l++) {
                    char *key;
                    ds_dynamic_array_get(keys, l, &key);

                    object_kv value;
                    ds_dynamic_array_get(values, l, &value);

                    printf("  %s: %s\n", key, value.type);
                }
            }
            printf("Methods:\n");
            for (unsigned int k = 0; k < value.methods.capacity; k++) {
                ds_dynamic_array *keys = &value.methods.keys[k];
                ds_dynamic_array *values = &value.methods.values[k];

                for (unsigned int l = 0; l < keys->count; l++) {
                    char *key;
                    ds_dynamic_array_get(keys, l, &key);

                    method_context value;
                    ds_dynamic_array_get(values, l, &value);

                    printf("  %s: %s\n", key, value.type);
                    printf("  Formals:\n");
                    for (unsigned int m = 0; m < value.formals.count; m++) {
                        object_kv formal;
                        ds_dynamic_array_get(&value.formals, m, &formal);

                        printf("    %s: %s\n", formal.name, formal.type);
                    }
                }
            }
        }
    }

    return context->result;
}
