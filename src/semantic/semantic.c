#include "semantic.h"
#include "ds.h"
#include "parser.h"
#include <stdarg.h>
#include <stdio.h>

typedef struct semantic_context {
        const char *filename;
        enum semantic_result result;
        ds_dynamic_array classes; // class_context
        FILE *error_fd;
} semantic_context;

typedef struct class_context {
        const char *name;
        struct class_context *parent;
        ds_dynamic_array objects; // object_context
        ds_dynamic_array methods; // method_context
} class_context;

typedef struct method_context {
        const char *name;
        const char *type;
        ds_dynamic_array formals; // object_context
} method_context;

typedef struct object_context {
        const char *name;
        const char *type;
        int external;
} object_context;

typedef struct method_environment_item {
        const char *class_name;
        const char *method_name;
        ds_dynamic_array names;   // const char *
        ds_dynamic_array formals; // const char *
        const char *type;
} method_environment_item;

typedef struct method_environment {
        ds_dynamic_array items; // method_environment_item
} method_environment;

typedef struct object_environment_item {
        const char *class_name;
        ds_dynamic_array objects; // object_context
} object_environment_item;

typedef struct object_environment {
        ds_dynamic_array items; // object_environment_item
} object_environment;

static node_info *token_get_node_info(expr_node *node) {
    switch (node->kind) {
    case EXPR_ASSIGN:
        return &node->assign.name;
    case EXPR_DISPATCH_FULL:
        return token_get_node_info(node->dispatch_full.expr);
    case EXPR_DISPATCH:
        return &node->dispatch.method;
    case EXPR_COND:
        return &node->cond.node;
    case EXPR_LOOP:
        return &node->loop.node;
    case EXPR_BLOCK:
        return &node->block.node;
    case EXPR_LET:
        return &node->let.node;
    case EXPR_CASE:
        return &node->case_.node;
    case EXPR_NEW:
        return &node->new.node;
    case EXPR_ISVOID:
        return &node->isvoid.op;
    case EXPR_ADD:
        return token_get_node_info(node->add.lhs);
    case EXPR_SUB:
        return token_get_node_info(node->sub.lhs);
    case EXPR_MUL:
        return token_get_node_info(node->mul.lhs);
    case EXPR_DIV:
        return token_get_node_info(node->div.lhs);
    case EXPR_NEG:
        return token_get_node_info(node->neg.expr);
    case EXPR_LT:
        return token_get_node_info(node->lt.lhs);
    case EXPR_LE:
        return token_get_node_info(node->le.lhs);
    case EXPR_EQ:
        return token_get_node_info(node->eq.lhs);
    case EXPR_NOT:
        return token_get_node_info(node->not_.expr);
    case EXPR_PAREN:
        return token_get_node_info(node->paren);
    case EXPR_IDENT:
        return &node->ident;
    case EXPR_INT:
        return &node->integer;
    case EXPR_STRING:
        return &node->string;
    case EXPR_BOOL:
        return &node->boolean;
    default:
        return NULL;
    }
}

static void context_show_errorf(semantic_context *context, unsigned int line,
                                unsigned int col, const char *format, ...) {
    context->result = SEMANTIC_ERROR;

    const char *filename = context->filename;

    if (filename != NULL) {
        fprintf(context->error_fd, "\"%s\", ", filename);
    }

    fprintf(context->error_fd, "line %d:%d, Semantic error: ", line, col);

    va_list args;
    va_start(args, format);
    vfprintf(context->error_fd, format, args);
    va_end(args);

    fprintf(context->error_fd, "\n");
}

static void find_class_ctx(semantic_context *context, const char *class_name,
                           class_context **class_ctx) {
    for (unsigned int i = 0; i < context->classes.count; i++) {
        class_context *ctx = NULL;
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
        method_context *ctx = NULL;
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
        object_context *ctx = NULL;
        ds_dynamic_array_get_ref(&class_ctx->objects, i, (void **)&ctx);

        if (strcmp(ctx->name, object_name) == 0) {
            *object_ctx = ctx;
            return;
        }
    }
}

// Check if lhs_type <= rhs_type
static int is_type_ancestor(semantic_context *context, const char *class_type,
                            const char *lhs_type, const char *rhs_type) {

    if (strcmp(lhs_type, SELF_TYPE) == 0 && strcmp(rhs_type, SELF_TYPE) == 0) {
        return 1;
    }

    if (strcmp(lhs_type, SELF_TYPE) == 0) {
        lhs_type = class_type;
    }

    class_context *lhs_ctx = NULL;
    find_class_ctx(context, lhs_type, &lhs_ctx);

    class_context *rhs_ctx = NULL;
    find_class_ctx(context, rhs_type, &rhs_ctx);

    if (lhs_ctx == NULL || rhs_ctx == NULL) {
        return 0;
    }

    class_context *current_ctx = lhs_ctx;
    do {
        if (strcmp(current_ctx->name, rhs_type) == 0) {
            return 1;
        }

        current_ctx = current_ctx->parent;
    } while (current_ctx != NULL && lhs_ctx != current_ctx);

    return 0;
}

// Find the least common ancestor of two types
static const char *least_common_ancestor(semantic_context *context,
                                         const char *class_type,
                                         const char *type1, const char *type2) {
    if (strcmp(type1, type2) == 0) {
        return type1;
    }

    if (strcmp(type1, SELF_TYPE) == 0) {
        const char *tmp = type1;
        type1 = type2;
        type2 = tmp;
    }

    class_context *class_ctx1 = NULL;
    find_class_ctx(context, type1, &class_ctx1);

    class_context *class_ctx2 = NULL;
    find_class_ctx(context, type2, &class_ctx2);

    if (class_ctx1 == NULL ||
        (class_ctx2 == NULL && strcmp(type2, SELF_TYPE) != 0)) {
        return NULL;
    }

    class_context *current_ctx = class_ctx1;
    while (current_ctx != NULL) {
        if (is_type_ancestor(context, class_type, type2, current_ctx->name)) {
            return current_ctx->name;
        }

        current_ctx = current_ctx->parent;
    }

    return OBJECT_TYPE;
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
    if (strcmp(class.name.value, SELF_TYPE) == 0) {
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
    if (strcmp(class.superclass.value, INT_TYPE) == 0 ||
        strcmp(class.superclass.value, STRING_TYPE) == 0 ||
        strcmp(class.superclass.value, BOOL_TYPE) == 0 ||
        strcmp(class.superclass.value, SELF_TYPE) == 0) {
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
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);
        context->filename = class.filename;

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

    class_context *object = NULL;
    find_class_ctx(context, OBJECT_TYPE, &object);

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);
        context->filename = class.filename;

        class_context *class_ctx = NULL;
        find_class_ctx(context, class.name.value, &class_ctx);

        if (class_ctx == NULL || class_ctx == object) {
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
        context->filename = class.filename;

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
    if (strcmp(attribute.type.value, SELF_TYPE) == 0) {
        return 0;
    }

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
        context->filename = class.filename;

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

            int external = 0;
            if (attribute.value.kind == EXPR_EXTERN) {
                external = 1;
            }

            object_context object = {.name = attribute.name.value,
                                     .type = attribute.type.value,
                                     .external = external};
            ds_dynamic_array_append(&class_ctx->objects, &object);
        }
    }

    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);
        context->filename = class.filename;

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
    if (strcmp(formal.type.value, SELF_TYPE) == 0) {
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
    if (strcmp(method.type.value, SELF_TYPE) == 0) {
        return 0;
    }

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
        context->filename = class.filename;

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
                                         .type = formal.type.value,
                                         .external = 0};
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
        context->filename = class.filename;

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

static void find_method_env(method_environment *env, const char *class_name,
                            const char *method_name,
                            method_environment_item **item) {
    unsigned int n = env->items.count;
    for (unsigned int i = 0; i < env->items.count; i++) {
        method_environment_item *env_item = NULL;
        ds_dynamic_array_get_ref(&env->items, n - i - 1, (void **)&env_item);

        if (strcmp(env_item->class_name, class_name) == 0 &&
            strcmp(env_item->method_name, method_name) == 0) {
            *item = env_item;
            return;
        }
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

        class_context *current_ctx = class_ctx;
        do {
            for (unsigned int k = 0; k < current_ctx->methods.count; k++) {
                method_context *method_ctx = NULL;
                ds_dynamic_array_get_ref(&current_ctx->methods, k,
                                         (void **)&method_ctx);

                const char *method_name = method_ctx->name;

                int found_item = 0;
                for (unsigned int m = 0; m < env->items.count; m++) {
                    method_environment_item *env_item = NULL;
                    ds_dynamic_array_get_ref(&env->items, m,
                                             (void **)&env_item);

                    if (strcmp(env_item->class_name, class_name) == 0 &&
                        strcmp(env_item->method_name, method_name) == 0) {
                        found_item = 1;
                        break;
                    }
                }

                if (found_item) {
                    continue;
                }

                method_environment_item item = {.class_name = class_name,
                                                .method_name = method_name};
                ds_dynamic_array_init(&item.names, sizeof(const char *));
                ds_dynamic_array_init(&item.formals, sizeof(const char *));

                for (unsigned int m = 0; m < method_ctx->formals.count; m++) {
                    object_context formal_ctx;
                    ds_dynamic_array_get(&method_ctx->formals, m, &formal_ctx);

                    ds_dynamic_array_append(&item.names, &formal_ctx.name);
                    ds_dynamic_array_append(&item.formals, &formal_ctx.type);
                }

                item.type = method_ctx->type;

                ds_dynamic_array_append(&env->items, &item);
            }

            current_ctx = current_ctx->parent;
        } while (current_ctx != NULL && class_ctx != current_ctx);
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

        object_context object = {
            .name = "self", .type = SELF_TYPE, .external = 0};
        ds_dynamic_array_append(&item.objects, &object);

        ds_dynamic_array_append(&env->items, &item);
    }
}

static void get_object_environment(object_environment *env,
                                   const char *class_name,
                                   object_environment_item *item) {
    for (unsigned int i = 0; i < env->items.count; i++) {
        object_environment_item *env_item = NULL;
        ds_dynamic_array_get_ref(&env->items, i, (void **)&env_item);

        if (strcmp(env_item->class_name, class_name) == 0) {
            (*item).class_name = env_item->class_name;
            ds_dynamic_array_copy(&env_item->objects, &(*item).objects);
            return;
        }
    }
}

static const char *semantic_check_expression(
    semantic_context *context, expr_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env);

static int is_let_init_name_illegal(semantic_context *context,
                                    let_init_node *init) {
    if (strcmp(init->name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_let_init_name_illegal(context, init)                \
    context_show_errorf(context, init->name.line, init->name.col,              \
                        "Let variable has illegal name %s", init->name.value)

static int is_let_init_type_undefined(semantic_context *context,
                                      let_init_node *init) {
    if (strcmp(init->type.value, SELF_TYPE) == 0) {
        return 0;
    }

    class_context *class_ctx = NULL;
    find_class_ctx(context, init->type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_let_init_type_undefined(context, init)              \
    context_show_errorf(context, init->type.line, init->type.col,              \
                        "Let variable %s has undefined type %s",               \
                        init->name.value, init->type.value)

static int is_let_init_type_incompatible(semantic_context *context,
                                         const char *class_type,
                                         const char *expr_type,
                                         const char *init_type) {
    return !is_type_ancestor(context, class_type, expr_type, init_type);
}

#define context_show_error_let_init_type_incompatible(context, token, init,    \
                                                      init_type)               \
    context_show_errorf(context, token->line, token->col,                      \
                        "Type %s of initialization expression of identifier "  \
                        "%s is incompatible with declared type %s",            \
                        init_type, init->name.value, init->type.value)

static const char *semantic_check_let_expression(
    semantic_context *context, let_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {

    unsigned int depth = 0;
    for (unsigned int i = 0; i < expr->inits.count; i++) {
        let_init_node *init = NULL;
        ds_dynamic_array_get_ref(&expr->inits, i, (void **)&init);

        if (is_let_init_name_illegal(context, init)) {
            context_show_error_let_init_name_illegal(context, init);
            continue;
        }

        if (is_let_init_type_undefined(context, init)) {
            context_show_error_let_init_type_undefined(context, init);
            continue;
        }

        const char *init_type = init->type.value;

        if (init->init != NULL) {
            const char *expr_type = semantic_check_expression(
                context, init->init, class_ctx, method_env, object_env);

            if (expr_type != NULL) {
                if (is_let_init_type_incompatible(context, class_ctx->name,
                                                  expr_type, init_type)) {
                    context_show_error_let_init_type_incompatible(
                        context, token_get_node_info(init->init), init,
                        expr_type);
                }
            }
        }

        object_context object = {
            .name = init->name.value, .type = init_type, .external = 0};
        ds_dynamic_array_append(&object_env->objects, &object);

        depth++;
    }

    const char *body_type = semantic_check_expression(
        context, expr->body, class_ctx, method_env, object_env);

    for (unsigned int i = 0; i < depth; i++) {
        ds_dynamic_array_pop(&object_env->objects, NULL);
    }

    return body_type;
}

static int is_case_variable_name_illegal(semantic_context *context,
                                         branch_node *branch) {
    if (strcmp(branch->name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_case_variable_name_illegal(context, branch)         \
    context_show_errorf(context, branch->name.line, branch->name.col,          \
                        "Case variable has illegal name %s",                   \
                        branch->name.value)

static int is_case_variable_type_illegal(semantic_context *context,
                                         branch_node *branch) {
    if (strcmp(branch->type.value, SELF_TYPE) == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_case_variable_type_illegal(context, branch)         \
    context_show_errorf(context, branch->type.line, branch->type.col,          \
                        "Case variable %s has illegal type %s",                \
                        branch->name.value, branch->type.value)

static int is_case_variable_type_undefined(semantic_context *context,
                                           branch_node *branch) {
    class_context *class_ctx = NULL;
    find_class_ctx(context, branch->type.value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_case_variable_type_undefined(context, branch)       \
    context_show_errorf(context, branch->type.line, branch->type.col,          \
                        "Case variable %s has undefined type %s",              \
                        branch->name.value, branch->type.value)

static const char *semantic_check_case_expression(
    semantic_context *context, case_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *case_type = NULL;

    semantic_check_expression(context, expr->expr, class_ctx, method_env, object_env);

    for (unsigned int i = 0; i < expr->cases.count; i++) {
        branch_node *branch = NULL;
        ds_dynamic_array_get_ref(&expr->cases, i, (void **)&branch);

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

        object_context object = {.name = branch->name.value,
                                 .type = branch->type.value,
                                 .external = 0};
        ds_dynamic_array_append(&object_env->objects, &object);

        const char *branch_type = semantic_check_expression(
            context, branch->body, class_ctx, method_env, object_env);

        if (case_type == NULL) {
            case_type = branch_type;
        } else {
            case_type = least_common_ancestor(context, class_ctx->name,
                                              case_type, branch_type);
        }

        ds_dynamic_array_pop(&object_env->objects, NULL);
    }

    return case_type;
}

static int is_ident_undefined(object_environment_item *object_env,
                              node_info *ident) {
    for (unsigned int i = 0; i < object_env->objects.count; i++) {
        object_context object;
        ds_dynamic_array_get(&object_env->objects, i, &object);

        if (strcmp(object.name, ident->value) == 0) {
            return 0;
        }
    }

    return 1;
}

#define context_show_error_ident_undefined(context, ident)                     \
    context_show_errorf(context, ident->line, ident->col,                      \
                        "Undefined identifier %s", ident->value)

static int is_external_ident(object_environment_item *object_env,
                             node_info *ident) {
    for (unsigned int i = 0; i < object_env->objects.count; i++) {
        object_context object;
        ds_dynamic_array_get(&object_env->objects, object_env->objects.count - i - 1, &object);

        if (strcmp(object.name, ident->value) == 0) {
            return object.external;
        }
    }

    return 0;
}

#define context_show_error_external_ident(context, ident)                      \
    context_show_errorf(context, (ident)->line, (ident)->col,                  \
                        "Cannot use external attribute %s", (ident)->value)

static const char *semantic_check_ident_expression(
    semantic_context *context, node_info *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    if (is_ident_undefined(object_env, expr)) {
        context_show_error_ident_undefined(context, expr);
    }

    if (is_external_ident(object_env, expr)) {
        context_show_error_external_ident(context, expr);
    }

    unsigned int n = object_env->objects.count;
    for (unsigned int i = 0; i < n; i++) {
        object_context object;
        ds_dynamic_array_get(&object_env->objects, n - i - 1, &object);

        if (strcmp(object.name, expr->value) == 0) {
            return object.type;
        }
    }

    return NULL;
}

static int is_operand_not_int(const char *type) {
    return strcmp(type, INT_TYPE) != 0;
}

#define context_show_error_operand_not_int(context, expr, op, type)            \
    context_show_errorf(context, expr->line, expr->col,                        \
                        "Operand of %s has type %s instead of Int", op.value,  \
                        type)

static const char *semantic_check_arith_expression(
    semantic_context *context, expr_binary_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *left_type = semantic_check_expression(
        context, expr->lhs, class_ctx, method_env, object_env);
    const char *right_type = semantic_check_expression(
        context, expr->rhs, class_ctx, method_env, object_env);

    if (left_type == NULL || right_type == NULL) {
        return INT_TYPE;
    }

    if (is_operand_not_int(left_type)) {
        context_show_error_operand_not_int(
            context, token_get_node_info(expr->lhs), expr->op, left_type);
        return INT_TYPE;
    }

    if (is_operand_not_int(right_type)) {
        context_show_error_operand_not_int(
            context, token_get_node_info(expr->rhs), expr->op, right_type);
        return INT_TYPE;
    }

    return INT_TYPE;
}

static char *semantic_check_neg_expression(
    semantic_context *context, expr_unary_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *expr_type = semantic_check_expression(
        context, expr->expr, class_ctx, method_env, object_env);

    if (expr_type == NULL) {
        return INT_TYPE;
    }

    if (is_operand_not_int(expr_type)) {
        context_show_error_operand_not_int(
            context, token_get_node_info(expr->expr), expr->op, expr_type);
        return INT_TYPE;
    }

    return INT_TYPE;
}

static const char *semantic_check_cmp_expression(
    semantic_context *context, expr_binary_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *left_type = semantic_check_expression(
        context, expr->lhs, class_ctx, method_env, object_env);
    const char *right_type = semantic_check_expression(
        context, expr->rhs, class_ctx, method_env, object_env);

    if (left_type == NULL || right_type == NULL) {
        return BOOL_TYPE;
    }

    if (is_operand_not_int(left_type)) {
        context_show_error_operand_not_int(
            context, token_get_node_info(expr->lhs), expr->op, left_type);
        return BOOL_TYPE;
    }

    if (is_operand_not_int(right_type)) {
        context_show_error_operand_not_int(
            context, token_get_node_info(expr->rhs), expr->op, right_type);
        return BOOL_TYPE;
    }

    return BOOL_TYPE;
}

static int is_operand_types_not_comparable(const char *left_type,
                                           const char *right_type) {
    return strcmp(left_type, right_type) != 0 &&
           (strcmp(left_type, INT_TYPE) == 0 ||
            strcmp(right_type, INT_TYPE) == 0 ||
            strcmp(left_type, STRING_TYPE) == 0 ||
            strcmp(right_type, STRING_TYPE) == 0 ||
            strcmp(left_type, BOOL_TYPE) == 0 ||
            strcmp(right_type, BOOL_TYPE) == 0);
}

#define context_show_error_operand_types_not_comparable(context, op, left,     \
                                                        right)                 \
    context_show_errorf(context, op.line, op.col, "Cannot compare %s with %s", \
                        left, right)

static const char *semantic_check_eq_expression(
    semantic_context *context, expr_binary_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *left_type = semantic_check_expression(
        context, expr->lhs, class_ctx, method_env, object_env);
    const char *right_type = semantic_check_expression(
        context, expr->rhs, class_ctx, method_env, object_env);

    if (left_type == NULL || right_type == NULL) {
        return BOOL_TYPE;
    }

    if (is_operand_types_not_comparable(left_type, right_type)) {
        context_show_error_operand_types_not_comparable(context, expr->op,
                                                        left_type, right_type);
        return BOOL_TYPE;
    }

    return BOOL_TYPE;
}
static int is_operand_not_bool(const char *type) {
    return strcmp(type, BOOL_TYPE) != 0;
}

#define context_show_error_operand_not_bool(context, expr, op, type)           \
    context_show_errorf(context, expr->line, expr->col,                        \
                        "Operand of %s has type %s instead of Bool", op.value, \
                        type)

static char *semantic_check_not_expression(
    semantic_context *context, expr_unary_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *expr_type = semantic_check_expression(
        context, expr->expr, class_ctx, method_env, object_env);

    if (expr_type == NULL) {
        return BOOL_TYPE;
    }

    if (is_operand_not_bool(expr_type)) {
        context_show_error_operand_not_bool(
            context, token_get_node_info(expr->expr), expr->op, expr_type);
        return BOOL_TYPE;
    }

    return BOOL_TYPE;
}

static int is_assign_name_illegal(semantic_context *context,
                                  assign_node *expr) {
    if (strcmp(expr->name.value, "self") == 0) {
        return 1;
    }

    return 0;
}

#define context_show_error_assign_name_illegal(context, expr)                  \
    context_show_errorf(context, expr->name.line, expr->name.col,              \
                        "Cannot assign to self")

static int is_assign_incopatible_types(semantic_context *context,
                                       const char *class_type,
                                       const char *expr_type,
                                       const char *object_type) {
    return !is_type_ancestor(context, class_type, expr_type, object_type);
}

#define context_show_error_assign_incompatible_types(context, expr, init,      \
                                                     left, right)              \
    context_show_errorf(context, init->line, init->col,                        \
                        "Type %s of assigned expression is incompatible with " \
                        "declared type %s of identifier %s",                   \
                        right, left, expr->name.value)

static const char *semantic_check_assign_expression(
    semantic_context *context, assign_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {

    if (is_assign_name_illegal(context, expr)) {
        context_show_error_assign_name_illegal(context, expr);
        return NULL;
    }

    if (is_external_ident(object_env, &expr->name)) {
        context_show_error_external_ident(context, &expr->name);
    }

    const char *object_type = NULL;
    for (unsigned int i = 0; i < object_env->objects.count; i++) {
        object_context object;
        ds_dynamic_array_get(&object_env->objects, i, &object);

        if (strcmp(object.name, expr->name.value) == 0) {
            object_type = object.type;
            break;
        }
    }
    if (object_type == NULL) {
        return NULL;
    }

    const char *expr_type = semantic_check_expression(
        context, expr->value, class_ctx, method_env, object_env);

    if (expr_type == NULL) {
        return object_type;
    }

    if (is_assign_incopatible_types(context, class_ctx->name, expr_type,
                                    object_type)) {
        context_show_error_assign_incompatible_types(
            context, expr, token_get_node_info(expr->value), object_type,
            expr_type);
    }

    return expr_type;
}

static int is_new_type_undefined(semantic_context *context, node_info *expr) {
    if (strcmp(expr->value, SELF_TYPE) == 0) {
        return 0;
    }

    class_context *class_ctx = NULL;
    find_class_ctx(context, expr->value, &class_ctx);
    return class_ctx == NULL;
}

#define context_show_error_new_type_undefined(context, expr)                   \
    context_show_errorf(context, expr->node.line, expr->type.col,              \
                        "new is used with undefined type %s",                  \
                        expr->type.value)

static const char *semantic_check_new_expression(
    semantic_context *context, new_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    if (is_new_type_undefined(context, &expr->type)) {
        context_show_error_new_type_undefined(context, expr);
        return NULL;
    }

    return expr->type.value;
}

static int is_while_condition_not_bool(const char *type) {
    return strcmp(type, BOOL_TYPE) != 0;
}

#define context_show_error_while_condition_not_bool(context, expr, type)       \
    context_show_errorf(context, expr->line, expr->col,                        \
                        "While condition has type %s instead of Bool", type)

static const char *semantic_check_loop_expression(
    semantic_context *context, loop_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *cond_type = semantic_check_expression(
        context, expr->predicate, class_ctx, method_env, object_env);
    if (cond_type != NULL && is_while_condition_not_bool(cond_type)) {
        context_show_error_while_condition_not_bool(
            context, token_get_node_info(expr->predicate), cond_type);
    }

    // https://dijkstra.eecs.umich.edu/eecs483/crm/Loops.html
    const char *_ = semantic_check_expression(context, expr->body, class_ctx,
                                              method_env, object_env);

    return OBJECT_TYPE;
}

static int is_if_condition_not_bool(const char *type) {
    return strcmp(type, BOOL_TYPE) != 0;
}

#define context_show_error_if_condition_not_bool(context, expr, type)          \
    context_show_errorf(context, expr->line, expr->col,                        \
                        "If condition has type %s instead of Bool", type)

static const char *semantic_check_if_expression(
    semantic_context *context, cond_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *cond_type = semantic_check_expression(
        context, expr->predicate, class_ctx, method_env, object_env);
    if (cond_type != NULL && is_if_condition_not_bool(cond_type)) {
        context_show_error_if_condition_not_bool(
            context, token_get_node_info(expr->predicate), cond_type);
    }

    const char *then = semantic_check_expression(context, expr->then, class_ctx,
                                                 method_env, object_env);
    const char *else_ = semantic_check_expression(
        context, expr->else_, class_ctx, method_env, object_env);

    return least_common_ancestor(context, class_ctx->name, then, else_);
}

static const char *semantic_check_block_expression(
    semantic_context *context, block_node *block, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *block_type = NULL;
    for (unsigned int i = 0; i < block->exprs.count; i++) {
        expr_node *expr = NULL;
        ds_dynamic_array_get_ref(&block->exprs, i, (void **)&expr);

        block_type = semantic_check_expression(context, expr, class_ctx,
                                               method_env, object_env);
    }

    return block_type;
}

static const char *semantic_check_isvoid_expression(
    semantic_context *context, expr_unary_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    semantic_check_expression(context, expr->expr, class_ctx, method_env,
                              object_env);

    return BOOL_TYPE;
}

#define context_show_error_dispatch_method_undefined(context, expr, method,    \
                                                     class_type)               \
    context_show_errorf(context, expr.line, expr.col,                          \
                        "Undefined method %s in class %s", method, class_type)

static int
is_dispatch_method_wrong_number_of_args(method_environment_item *method_item,
                                        dispatch_node *expr) {
    return method_item->formals.count != expr->args.count;
}

#define context_show_error_dispatch_method_wrong_number_of_args(               \
    context, token, method, class)                                             \
    context_show_errorf(context, token.line, token.col,                        \
                        "Method %s of class %s is applied to wrong number of " \
                        "arguments",                                           \
                        method, class)

static int is_arg_type_incompatible(semantic_context *context,
                                    const char *class_type,
                                    const char *formal_type,
                                    const char *arg_type) {
    return !is_type_ancestor(context, class_type, formal_type, arg_type);
}

#define context_show_error_dispatch_arg_type_incompatible(                     \
    context, token, method, class, arg_type, formal, formal_type)              \
    context_show_errorf(context, token->line, token->col,                      \
                        "In call to method %s of class %s, actual type %s of " \
                        "formal parameter %s is incompatible with declared "   \
                        "type %s",                                             \
                        method, class, arg_type, formal, formal_type)

static const char *semantic_check_dispatch_expression(
    semantic_context *context, dispatch_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    method_environment_item *method_item = NULL;
    find_method_env(method_env, class_ctx->name, expr->method.value,
                    &method_item);

    if (method_item == NULL) {
        context_show_error_dispatch_method_undefined(
            context, expr->method, expr->method.value, class_ctx->name);
        return NULL;
    }

    if (is_dispatch_method_wrong_number_of_args(method_item, expr)) {
        context_show_error_dispatch_method_wrong_number_of_args(
            context, expr->method, method_item->method_name,
            method_item->class_name);
    }

    for (unsigned int i = 0; i < expr->args.count; i++) {
        expr_node *arg = NULL;
        ds_dynamic_array_get_ref(&expr->args, i, (void **)&arg);

        const char *arg_type = semantic_check_expression(
            context, arg, class_ctx, method_env, object_env);

        if (arg_type == NULL) {
            continue;
        }

        const char *formal_type = NULL;
        ds_dynamic_array_get(&method_item->formals, i, &formal_type);

        const char *formal_name = NULL;
        ds_dynamic_array_get(&method_item->names, i, &formal_name);

        if (is_arg_type_incompatible(context, class_ctx->name, arg_type,
                                     formal_type)) {
            context_show_error_dispatch_arg_type_incompatible(
                context, token_get_node_info(arg), method_item->method_name,
                method_item->class_name, arg_type, formal_name, formal_type);
        }
    }

    return method_item->type;
}

static int is_illegal_static_type(semantic_context *context, const char *type) {
    return strcmp(type, SELF_TYPE) == 0;
}

#define context_show_error_static_dispatch_illegal_type(context, token)        \
    context_show_errorf(context, token.line, token.col,                        \
                        "Type of static dispatch cannot be SELF_TYPE")

#define context_show_error_static_dispatch_type_undefined(context, token,      \
                                                          type)                \
    context_show_errorf(context, token.line, token.col,                        \
                        "Type %s of static dispatch is undefined", type)

static int is_not_valid_static_dispatch(semantic_context *context,
                                        const char *class_type,
                                        const char *expr_type,
                                        const char *static_type) {
    return !is_type_ancestor(context, class_type, expr_type, static_type);
}

#define context_show_error_static_dispatch_type_not_valid(                     \
    context, token, static_type, expr_type)                                    \
    context_show_errorf(context, token.line, token.col,                        \
                        "Type %s of static dispatch is not a superclass of "   \
                        "type %s",                                             \
                        static_type, expr_type)

static const char *semantic_check_dispatch_full_expression(
    semantic_context *context, dispatch_full_node *expr,
    class_context *class_ctx, method_environment *method_env,
    object_environment_item *object_env) {
    const char *expr_type = semantic_check_expression(
        context, expr->expr, class_ctx, method_env, object_env);

    if (expr_type == NULL) {
        return NULL;
    }

    const char *static_type = expr->type.value;
    if (static_type == NULL) {
        static_type = expr_type;

        if (strcmp(static_type, SELF_TYPE) == 0) {
            static_type = class_ctx->name;
        }
    } else {
        if (is_illegal_static_type(context, static_type)) {
            context_show_error_static_dispatch_illegal_type(context,
                                                            expr->type);
            return NULL;
        }

        class_context *static_ctx = NULL;
        find_class_ctx(context, static_type, &static_ctx);

        if (static_ctx == NULL) {
            context_show_error_static_dispatch_type_undefined(
                context, expr->type, static_type);
            return NULL;
        }

        if (is_not_valid_static_dispatch(context, class_ctx->name, expr_type,
                                         static_type)) {
            context_show_error_static_dispatch_type_not_valid(
                context, expr->type, static_type, expr_type);
            return NULL;
        }
    }

    method_environment_item *method_item = NULL;
    find_method_env(method_env, static_type, expr->dispatch->method.value,
                    &method_item);

    if (method_item == NULL) {
        context_show_error_dispatch_method_undefined(
            context, expr->dispatch->method, expr->dispatch->method.value,
            static_type);
        return NULL;
    }

    if (is_dispatch_method_wrong_number_of_args(method_item, expr->dispatch)) {
        context_show_error_dispatch_method_wrong_number_of_args(
            context, expr->dispatch->method, method_item->method_name,
            method_item->class_name);
    }

    for (unsigned int i = 0; i < expr->dispatch->args.count; i++) {
        expr_node *arg = NULL;
        ds_dynamic_array_get_ref(&expr->dispatch->args, i, (void **)&arg);

        const char *arg_type = semantic_check_expression(
            context, arg, class_ctx, method_env, object_env);

        if (arg_type == NULL) {
            continue;
        }

        const char *formal_type = NULL;
        ds_dynamic_array_get(&method_item->formals, i, &formal_type);

        const char *formal_name = NULL;
        ds_dynamic_array_get(&method_item->names, i, &formal_name);

        if (is_arg_type_incompatible(context, class_ctx->name, arg_type,
                                     formal_type)) {
            context_show_error_dispatch_arg_type_incompatible(
                context, token_get_node_info(arg), method_item->method_name,
                method_item->class_name, arg_type, formal_name, formal_type);
        }
    }

    if (strcmp(method_item->type, SELF_TYPE) == 0) {
        return expr_type;
    }

    return method_item->type;
}

static const char *semantic_check_expression(
    semantic_context *context, expr_node *expr, class_context *class_ctx,
    method_environment *method_env, object_environment_item *object_env) {
    const char *type = NULL;

    switch (expr->kind) {
    case EXPR_ASSIGN:
        type = semantic_check_assign_expression(
            context, &expr->assign, class_ctx, method_env, object_env);
        break;
    case EXPR_DISPATCH_FULL:
        type = semantic_check_dispatch_full_expression(
            context, &expr->dispatch_full, class_ctx, method_env, object_env);
        break;
    case EXPR_DISPATCH:
        type = semantic_check_dispatch_expression(
            context, &expr->dispatch, class_ctx, method_env, object_env);
        break;
    case EXPR_COND:
        type = semantic_check_if_expression(context, &expr->cond, class_ctx,
                                            method_env, object_env);
        break;
    case EXPR_LOOP:
        type = semantic_check_loop_expression(context, &expr->loop, class_ctx,
                                              method_env, object_env);
        break;
    case EXPR_BLOCK:
        type = semantic_check_block_expression(context, &expr->block, class_ctx,
                                               method_env, object_env);
        break;
    case EXPR_LET:
        type = semantic_check_let_expression(context, &expr->let, class_ctx,
                                             method_env, object_env);
        break;
    case EXPR_CASE:
        type = semantic_check_case_expression(context, &expr->case_, class_ctx,
                                              method_env, object_env);
        break;
    case EXPR_NEW:
        type = semantic_check_new_expression(context, &expr->new, class_ctx,
                                             method_env, object_env);
        break;
    case EXPR_ISVOID:
        type = semantic_check_isvoid_expression(
            context, &expr->isvoid, class_ctx, method_env, object_env);
        break;
    case EXPR_ADD:
        type = semantic_check_arith_expression(context, &expr->add, class_ctx,
                                               method_env, object_env);
        break;
    case EXPR_SUB:
        type = semantic_check_arith_expression(context, &expr->sub, class_ctx,
                                               method_env, object_env);
        break;
    case EXPR_MUL:
        type = semantic_check_arith_expression(context, &expr->mul, class_ctx,
                                               method_env, object_env);
        break;
    case EXPR_DIV:
        type = semantic_check_arith_expression(context, &expr->div, class_ctx,
                                               method_env, object_env);
        break;
    case EXPR_NEG:
        type = semantic_check_neg_expression(context, &expr->neg, class_ctx,
                                             method_env, object_env);
        break;
    case EXPR_LT:
        type = semantic_check_cmp_expression(context, &expr->lt, class_ctx,
                                             method_env, object_env);
        break;
    case EXPR_LE:
        type = semantic_check_cmp_expression(context, &expr->lt, class_ctx,
                                             method_env, object_env);
        break;
    case EXPR_EQ:
        type = semantic_check_eq_expression(context, &expr->eq, class_ctx,
                                            method_env, object_env);
        break;
    case EXPR_NOT:
        type = semantic_check_not_expression(context, &expr->not_, class_ctx,
                                             method_env, object_env);
        break;
    case EXPR_PAREN:
        type = semantic_check_expression(context, expr->paren, class_ctx,
                                         method_env, object_env);
        break;
    case EXPR_IDENT:
        type = semantic_check_ident_expression(context, &expr->ident, class_ctx,
                                               method_env, object_env);
        break;
    case EXPR_INT:
        type = INT_TYPE;
        break;
    case EXPR_STRING:
        type = STRING_TYPE;
        break;
    case EXPR_BOOL:
        type = BOOL_TYPE;
        break;
    default:
        type = NULL;
        break;
    }

    expr->type = type;

    return type;
}

static int is_method_return_type_incompatible(semantic_context *context,
                                              const char *class_type,
                                              const char *return_type,
                                              const char *method_type) {
    return !is_type_ancestor(context, class_type, return_type, method_type);
}

#define context_show_error_method_body_incompatible_return_type(               \
    context, token, method, return_type, method_type)                          \
    context_show_errorf(context, token->line, token->col,                      \
                        "Type %s of the body of method %s is incompatible "    \
                        "with declared return type %s",                        \
                        return_type, method->name.value, method_type)

static void semantic_check_method_body(semantic_context *context,
                                       program_node *program,
                                       method_environment *method_env,
                                       object_environment *object_envs) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node *class = NULL;
        ds_dynamic_array_get_ref(&program->classes, i, (void **)&class);
        context->filename = class->filename;

        class_context *class_ctx = NULL;
        find_class_ctx(context, class->name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        object_environment_item object_env = {0};
        get_object_environment(object_envs, class->name.value, &object_env);

        for (unsigned int j = 0; j < class->methods.count; j++) {
            method_node *method = NULL;
            ds_dynamic_array_get_ref(&class->methods, j, (void **)&method);

            method_context *method_ctx = NULL;
            find_method_ctx(class_ctx, method->name.value, &method_ctx);

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

            expr_node *body = &method->body;
            const char *body_type = semantic_check_expression(
                context, body, class_ctx, method_env, &object_env);

            if (body_type != NULL) {
                if (is_method_return_type_incompatible(context, class_ctx->name,
                                                       body_type,
                                                       method_ctx->type)) {
                    context_show_error_method_body_incompatible_return_type(
                        context, token_get_node_info(body), method, body_type,
                        method_ctx->type);
                }
            }

            for (unsigned int k = 0; k < depth; k++) {
                ds_dynamic_array_pop(&object_env.objects, NULL);
            }
        }
    }
}

static int is_attribute_value_type_incompatible(semantic_context *context,
                                                const char *class_type,
                                                const char *value_type,
                                                const char *attribute_type) {
    return !is_type_ancestor(context, class_type, value_type, attribute_type);
}

#define context_show_error_attribute_init_incompatible(context, token, attr,   \
                                                       attr_type, value_type)  \
    context_show_errorf(                                                       \
        context, token->line, token->col,                                      \
        "Type %s of initialization expression of attribute %s "                \
        "is incompatible with declared type %s",                               \
        value_type, attr->name.value, attr_type)

static void semantic_check_attribute_init(semantic_context *context,
                                          program_node *program,
                                          method_environment *method_env,
                                          object_environment *object_envs) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node *class = NULL;
        ds_dynamic_array_get_ref(&program->classes, i, (void **)&class);
        context->filename = class->filename;

        class_context *class_ctx = NULL;
        find_class_ctx(context, class->name.value, &class_ctx);

        if (class_ctx == NULL) {
            continue;
        }

        object_environment_item object_env = {0};
        get_object_environment(object_envs, class->name.value, &object_env);

        for (unsigned int j = 0; j < class->attributes.count; j++) {
            attribute_node *attribute = NULL;
            ds_dynamic_array_get_ref(&class->attributes, j,
                                     (void **)&attribute);

            object_context *object_ctx = NULL;
            find_object_ctx(class_ctx, attribute->name.value, &object_ctx);

            if (object_ctx == NULL) {
                continue;
            }

            expr_node *body = &attribute->value;
            const char *value_type = semantic_check_expression(
                context, body, class_ctx, method_env, &object_env);

            if (value_type != NULL) {
                if (is_attribute_value_type_incompatible(
                        context, class_ctx->name, value_type,
                        object_ctx->type)) {
                    context_show_error_attribute_init_incompatible(
                        context, token_get_node_info(body), attribute,
                        object_ctx->type, value_type);
                }
            }
        }
    }
}

static void find_class_mapping(semantic_mapping *mapping, const char *name,
                               semantic_mapping_item **item) {
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *it = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&it);

        if (strcmp(it->class_name, name) == 0) {
            *item = it;
            return;
        }
    }

    *item = NULL;
}

static void find_class_node(program_node *program, const char *name,
                            class_node **class) {
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node *node = NULL;
        ds_dynamic_array_get_ref(&program->classes, i, (void **)&node);

        if (strcmp(node->name.value, name) == 0) {
            *class = node;
            return;
        }
    }

    *class = NULL;
}

static void find_attribute_node(class_node *class, const char *name,
                                attribute_node **attribute) {
    for (unsigned int i = 0; i < class->attributes.count; i++) {
        attribute_node *node = NULL;
        ds_dynamic_array_get_ref(&class->attributes, i, (void **)&node);

        if (strcmp(node->name.value, name) == 0) {
            *attribute = node;
            return;
        }
    }

    *attribute = NULL;
}

static void find_method_node(class_node *class, const char *name,
                             method_node **method) {
    for (unsigned int i = 0; i < class->methods.count; i++) {
        method_node *node = NULL;
        ds_dynamic_array_get_ref(&class->methods, i, (void **)&node);

        if (strcmp(node->name.value, name) == 0) {
            *method = node;
            return;
        }
    }

    *method = NULL;
}

void semantic_print_mapping(semantic_mapping *mapping) {
    printf("parent_mapping\n");
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *item = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&item);

        if (item->parent != NULL) {
            printf("%s -> %s\n", item->class_name, item->parent->class_name);
        }
    }

    printf("class_mapping\n");
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *item = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&item);

        printf("%s\n", item->class_name);
        for (unsigned int j = 0; j < item->attributes.count; j++) {
            class_mapping_attribute *attribute = NULL;
            ds_dynamic_array_get_ref(&item->attributes, j, (void **)&attribute);

            printf("  %s\n", attribute->attribute_name);
        }
    }

    printf("implementations_mapping\n");
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *item = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&item);

        for (unsigned int j = 0; j < item->methods.count; j++) {
            implementation_mapping_item *method = NULL;
            ds_dynamic_array_get_ref(&item->methods, j, (void **)&method);

            printf("%s@%s.%s\n", item->class_name, method->from_class, method->method_name);
        }
    }
}

static void search_dfs_class_tree(semantic_context *context,
                                  program_node *program,
                                  class_context *current_ctx,
                                  ds_linked_list *class_queue) {
    ds_linked_list stack;
    ds_linked_list_init(&stack, sizeof(class_context *));

    ds_linked_list_push_front(&stack, &current_ctx);

    while (ds_linked_list_empty(&stack) == 0) {
        class_context *current_ctx = NULL;
        ds_linked_list_pop_front(&stack, &current_ctx);

        ds_linked_list_push_back(class_queue, &current_ctx);

        for (unsigned int i = 0; i < context->classes.count; i++) {
            class_context *child_ctx = NULL;
            ds_dynamic_array_get_ref(&context->classes, i, (void **)&child_ctx);

            if (child_ctx->parent == current_ctx) {
                ds_linked_list_push_front(&stack, &child_ctx);
            }
        }
    }
}

static void build_semantic_mapping(semantic_context *context,
                                   program_node *program,
                                   semantic_mapping *mapping) {
    ds_linked_list class_queue;
    ds_linked_list_init(&class_queue, sizeof(class_context *));

    class_context *root = NULL;
    find_class_ctx(context, OBJECT_TYPE, &root);

    search_dfs_class_tree(context, program, root, &class_queue);

    ds_dynamic_array_init(&mapping->classes, sizeof(semantic_mapping_item));

    // Initialize each class
    while (ds_linked_list_empty(&class_queue) == 0) {
        class_context *class_ctx = NULL;
        ds_linked_list_pop_front(&class_queue, &class_ctx);

        if (strcmp(class_ctx->name, SELF_TYPE) == 0) {
            continue;
        }

        ds_dynamic_array attributes;
        ds_dynamic_array_init(&attributes, sizeof(class_mapping_attribute));

        ds_dynamic_array methods;
        ds_dynamic_array_init(&methods, sizeof(implementation_mapping_item));

        semantic_mapping_item item = {.class_name = class_ctx->name,
                                      .parent = NULL,
                                      .attributes = attributes,
                                      .methods = methods};

        ds_dynamic_array_append(&mapping->classes, &item);
    }

    // Set the parents
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *item = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&item);

        class_context *class_ctx = NULL;
        find_class_ctx(context, item->class_name, &class_ctx);

        class_context *parent_ctx = class_ctx->parent;
        if (parent_ctx == NULL) {
            continue;
        }

        semantic_mapping_item *parent_item = NULL;
        find_class_mapping(mapping, parent_ctx->name, &parent_item);

        if (parent_item != NULL) {
            item->parent = parent_item;
        }
    }

    // set the attributes
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *item = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&item);

        class_context *class_ctx = NULL;
        find_class_ctx(context, item->class_name, &class_ctx);

        if (strcmp(class_ctx->name, SELF_TYPE) == 0) {
            continue;
        }

        const char *class_name = class_ctx->name;

        semantic_mapping_item *parent_item = NULL;
        find_class_mapping(mapping, class_ctx->name, &parent_item);

        while (parent_item != NULL) {
            class_context *current_ctx = NULL;
            find_class_ctx(context, parent_item->class_name, &current_ctx);

            for (unsigned int k = 0; k < current_ctx->objects.count; k++) {
                object_context *object = NULL;
                ds_dynamic_array_get_ref(&current_ctx->objects,
                                         current_ctx->objects.count - k - 1,
                                         (void **)&object);

                const char *attribute_name = object->name;

                class_mapping_attribute attr = {.attribute_name = attribute_name};

                class_node *class = NULL;
                find_class_node(program, parent_item->class_name, &class);

                attribute_node *attribute = NULL;
                if (class != NULL) {
                    find_attribute_node(class, attribute_name, &attribute);
                }

                attr.attribute = attribute;

                ds_dynamic_array_append(&item->attributes, &attr);
            }

            parent_item = parent_item->parent;
        }

        ds_dynamic_array_reverse(&item->attributes);
    }

    // set the methods
    for (unsigned int i = 0; i < mapping->classes.count; i++) {
        semantic_mapping_item *item = NULL;
        ds_dynamic_array_get_ref(&mapping->classes, i, (void **)&item);

        class_context *class_ctx = NULL;
        find_class_ctx(context, item->class_name, &class_ctx);

        if (strcmp(class_ctx->name, SELF_TYPE) == 0) {
            continue;
        }

        const char *class_name = class_ctx->name;

        ds_linked_list class_stack;
        ds_linked_list_init(&class_stack, sizeof(class_context *));

        class_context *current_ctx = class_ctx;
        while (current_ctx != NULL) {
            ds_linked_list_push_front(&class_stack, &current_ctx);

            current_ctx = current_ctx->parent;
        }

        while (ds_linked_list_empty(&class_stack) == 0)  {
            class_context *current_ctx = NULL;

            ds_linked_list_pop_front(&class_stack, &current_ctx);

            const char *parent_name = current_ctx->name;

            for (unsigned int i = 0; i < current_ctx->methods.count; i++) {
                method_context *method_ctx = NULL;
                ds_dynamic_array_get_ref(&current_ctx->methods, i, (void **)&method_ctx);

                const char *method_name = method_ctx->name;

                int found = 0;
                unsigned int j = 0;
                for (j = 0; j < item->methods.count; j++) {
                    implementation_mapping_item *it = NULL;
                    ds_dynamic_array_get_ref(&item->methods, j, (void **)&it);

                    if (strcmp(it->method_name, method_name) == 0) {
                        found = 1;
                        break;
                    }
                }

                class_node *class = NULL;
                find_class_node(program, parent_name, &class);

                method_node *method = NULL;
                if (class != NULL) {
                    find_method_node(class, method_name, &method);
                }

                if (!found) {
                    implementation_mapping_item m = {.from_class = parent_name,
                                                        .method_name = method_name,
                                                        .method = method};

                    ds_dynamic_array_append(&item->methods, &m);
                } else {
                    implementation_mapping_item *m = NULL;
                    ds_dynamic_array_get_ref(&item->methods, j, (void **)&m);

                    m->from_class = parent_name;
                    m->method = method;
                }
            }
        }
    }
}

enum semantic_result semantic_check(program_node *program, semantic_mapping *mapping) {
    semantic_context context = {.filename = program->filename};

    context.result = SEMANTIC_OK;
    ds_dynamic_array_init(&context.classes, sizeof(class_context));
    context.error_fd = stderr;

    semantic_check_classes(&context, program);
    semantic_check_attributes(&context, program);
    semantic_check_methods(&context, program);

    object_environment object_env;
    build_object_environment(&context, program, &object_env);

    method_environment method_env;
    build_method_environment(&context, program, &method_env);

    semantic_check_method_body(&context, program, &method_env, &object_env);
    semantic_check_attribute_init(&context, program, &method_env, &object_env);

    if (context.result == SEMANTIC_OK) {
        build_semantic_mapping(&context, program, mapping);
    }

    return context.result;
}
