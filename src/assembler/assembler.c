#include "assembler.h"
#include "codegen.h"
#include "ds.h"
#include "parser.h"
#include "semantic.h"
#include "stdio.h"
#include <stdarg.h>

enum asm_const_type {
    ASM_CONST_INT,
    ASM_CONST_STR,
    ASM_CONST_BOOL,
};

typedef struct asm_const_value {
        enum asm_const_type type;
        int tag;
        union {
                struct {
                        const char *len_label;
                        const char *value;
                } str;
                unsigned int integer;
                unsigned int boolean;
        };
} asm_const_value;

typedef struct asm_const {
        const char *name;
        asm_const_value value;
} asm_const;

typedef struct assembler_context {
        FILE *file;
        program_node *program;
        semantic_mapping *mapping;
        int result;
        int int_tag;
        int str_tag;
        int bool_tag;
        ds_dynamic_array consts; // asm_const
} assembler_context;

static int assembler_context_init(assembler_context *context,
                                  const char *filename, program_node *program,
                                  semantic_mapping *mapping) {
    int result = 0;

    if (filename == NULL) {
        context->file = stdout;
    } else {
        context->file = fopen(filename, "w");
        if (context->file == NULL) {
            return_defer(1);
        }
    }

    context->program = program;
    context->mapping = mapping;
    context->result = 0;

    ds_dynamic_array_init(&context->consts, sizeof(asm_const));

defer:
    if (result != 0 && filename != NULL && context->file != NULL) {
        fclose(context->file);
    }
    context->result = result;
    return result;
}

static void assembler_context_destroy(assembler_context *context) {
    if (context->file != NULL && context->file != stdout) {
        fclose(context->file);
    }
}

#define COMMENT_START_COLUMN 40

static void assembler_emit_fmt(assembler_context *context, int align,
                               const char *comment, const char *format, ...) {
    fprintf(context->file, "%*s", align, "");

    va_list args;
    va_start(args, format);
    int size = vsnprintf(NULL, 0, format, args) + align;
    int padding = COMMENT_START_COLUMN - size;
    va_end(args);

    va_start(args, format);
    vfprintf(context->file, format, args);
    va_end(args);

    if (comment != NULL) {
        if (padding < 0) {
            padding = 0;
        }
        fprintf(context->file, "%*s; %s", padding, "", comment);
    }

    fprintf(context->file, "\n");
}

#define assembler_emit(context, format, ...)                                   \
    assembler_emit_fmt(context, 0, NULL, format, ##__VA_ARGS__)

static inline const char *comment_fmt(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    char *comment = malloc(size + 1);

    va_start(args, format);
    vsnprintf(comment, size + 1, format, args);
    va_end(args);

    return comment;
}

static void assembler_find_const(assembler_context *context,
                                 asm_const_value value, asm_const **result) {
    for (size_t i = 0; i < context->consts.count; i++) {
        asm_const *c = NULL;
        ds_dynamic_array_get_ref(&context->consts, i, (void **)&c);

        if (c->value.type != value.type) {
            continue;
        }

        switch (value.type) {
        case ASM_CONST_STR: {
            if (strcmp(c->value.str.value, value.str.value) == 0) {
                *result = c;
                return;
            }
            break;
        }
        case ASM_CONST_INT: {
            if (c->value.integer == value.integer) {
                *result = c;
                return;
            }
            break;
        }
        case ASM_CONST_BOOL: {
            if (c->value.boolean == value.boolean) {
                *result = c;
                return;
            }
            break;
        }
        }
    }

    *result = NULL;
}

static void assembler_new_const(assembler_context *context,
                                asm_const_value value, asm_const **result) {
    *result = NULL;

    assembler_find_const(context, value, result);
    if (*result != NULL) {
        return;
    }

    int count = context->consts.count;
    const char *prefix = NULL;

    switch (value.type) {
    case ASM_CONST_STR: {
        prefix = "str_const";
        value.tag = context->str_tag;
        break;
    }
    case ASM_CONST_INT: {
        prefix = "int_const";
        value.tag = context->int_tag;
        break;
    }
    case ASM_CONST_BOOL: {
        prefix = "bool_const";
        value.tag = context->bool_tag;
        break;
    }
    }

    size_t needed = snprintf(NULL, 0, "%s%d", prefix, count);
    char *name = malloc(needed + 1);
    if (name == NULL) {
        return;
    }

    snprintf(name, needed + 1, "%s%d", prefix, count);

    asm_const constant = {.name = name, .value = value};
    ds_dynamic_array_append(&context->consts, &constant);

    ds_dynamic_array_get_ref(&context->consts, count, (void **)result);
}

static void assembler_emit_const(assembler_context *context, asm_const c) {
    int align = strlen(c.name) + 1;
    assembler_emit_fmt(context, 0, "type tag", "%s dq %d", c.name, c.value.tag);

    switch (c.value.type) {
    case ASM_CONST_STR: {
        assembler_emit_fmt(context, align, "object size", "dq %d", 5);
        assembler_emit_fmt(context, align, "dispatch table",
                           "dq String_dispTab");
        assembler_emit_fmt(context, align, "pointer to length", "dq %s",
                           c.value.str.len_label);
        assembler_emit_fmt(context, align, "string value", "db \"%s\", 0",
                           c.value.str.value);
        break;
    }
    case ASM_CONST_INT: {
        assembler_emit_fmt(context, align, "object size", "dq %d", 4);
        assembler_emit_fmt(context, align, "dispatch table", "dq Int_dispTab");
        assembler_emit_fmt(context, align, "integer value", "dq %d",
                           c.value.integer);
        break;
    }
    case ASM_CONST_BOOL: {
        assembler_emit_fmt(context, align, "object size", "dq %d", 4);
        assembler_emit_fmt(context, align, "dispatch table", "dq Bool_dispTab");
        assembler_emit_fmt(context, align, "boolean value", "db %d",
                           c.value.boolean);
        break;
    }
    }
}

static void assembler_emit_consts(assembler_context *context,
                                  program_node *program,
                                  semantic_mapping *mapping) {
    assembler_emit(context, "segment readable");
    assembler_emit(context, "_int_tag dq %d", context->int_tag);
    assembler_emit(context, "_string_tag dq %d", context->str_tag);
    assembler_emit(context, "_bool_tag dq %d", context->bool_tag);

    for (size_t i = 0; i < context->consts.count; i++) {
        asm_const *c = NULL;
        ds_dynamic_array_get_ref(&context->consts, i, (void **)&c);

        assembler_emit_const(context, *c);
    }
}

static void assembler_emit_class_name_table(assembler_context *context,
                                            program_node *program,
                                            semantic_mapping *mapping) {
    assembler_emit(context, "segment readable");
    assembler_emit(context, "class_nameTab:");

    for (size_t i = 0; i < mapping->parents.classes.count; i++) {
        class_node *class = NULL;
        ds_dynamic_array_get_ref(&mapping->parents.classes, i, (void **)&class);

        const char *class_name = class->name.value;
        const int class_name_length = strlen(class_name);

        asm_const *int_const = NULL;
        assembler_new_const(context,
                            (asm_const_value){.type = ASM_CONST_INT,
                                              .integer = class_name_length},
                            &int_const);

        asm_const *str_const = NULL;
        assembler_new_const(
            context,
            (asm_const_value){.type = ASM_CONST_STR,
                              .str = {int_const->name, class_name}},
            &str_const);

        const char *comment =
            comment_fmt("pointer to class name %s", class_name);
        assembler_emit_fmt(context, 4, comment, "dq %s", str_const->name);
    }
}

static void assembler_emit_class_object_table(assembler_context *context,
                                              program_node *program,
                                              semantic_mapping *mapping) {
    assembler_emit(context, "segment readable");
    assembler_emit(context, "class_objTab:");

    for (size_t i = 0; i < mapping->parents.classes.count; i++) {
        class_node *class = NULL;
        ds_dynamic_array_get_ref(&mapping->parents.classes, i, (void **)&class);

        const char *class_name = class->name.value;

        assembler_emit_fmt(context, 4, NULL, "dq %s_protObj", class_name);
        assembler_emit_fmt(context, 4, NULL, "dq %s_init", class_name);
    }
}

static void assembler_emit_attribute_init(assembler_context *context,
                                          class_mapping_attribute *attr) {
    const attribute_node *node = attr->attribute;

    const char *comment = comment_fmt("attribute %s", attr->name);
    switch (node->value.kind) {
    case EXPR_INT: {
        asm_const *int_const = NULL;
        assembler_new_const(
            context,
            (asm_const_value){.type = ASM_CONST_INT,
                              .integer = atoi(node->value.integer.value)},
            &int_const);

        assembler_emit_fmt(context, 4, comment, "dq %s", int_const->name);
        break;
    }
    case EXPR_BOOL: {
        asm_const *bool_const = NULL;
        assembler_new_const(
            context,
            (asm_const_value){
                .type = ASM_CONST_BOOL,
                .boolean = strcmp(node->value.boolean.value, "true") ? 1 : 0},
            &bool_const);

        assembler_emit_fmt(context, 4, comment, "dq %s", bool_const->name);
        break;
    }
    case EXPR_STRING: {
        asm_const *int_const = NULL;
        assembler_new_const(
            context,
            (asm_const_value){.type = ASM_CONST_INT,
                              .integer = strlen(node->value.string.value)},
            &int_const);

        asm_const *str_const = NULL;
        assembler_new_const(
            context,
            (asm_const_value){
                .type = ASM_CONST_STR,
                .str = {int_const->name, node->value.string.value}},
            &str_const);

        assembler_emit_fmt(context, 4, comment, "dq %s", str_const->name);
        break;
    }
    case EXPR_EXTERN: {
        if (strcmp(node->type.value, STRING_TYPE) == 0) {
            assembler_emit_fmt(context, 4, comment, "dq \"\"");
        } else if (strcmp(node->type.value, INT_TYPE) == 0) {
            assembler_emit_fmt(context, 4, comment, "dq %d", 0);
        } else if (strcmp(node->type.value, BOOL_TYPE) == 0) {
            assembler_emit_fmt(context, 4, comment, "dq %d", 0);
        } else {
            assembler_emit_fmt(context, 4, comment, "dq %d", 0);
        }
        break;
    }
    default:
        assembler_emit_fmt(context, 4, comment, "dq %d", 0);
        break;
    }
}

static void assembler_emit_object_prototype(assembler_context *context,
                                            size_t i, program_node *program,
                                            semantic_mapping *mapping) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, i, (void **)&class);

    const char *class_name = class->class_name;

    assembler_emit(context, "segment readable");
    assembler_emit_fmt(context, 0, NULL, "%s_protObj:", class_name);
    assembler_emit_fmt(context, 4, "object tag", "dq %d", i);
    assembler_emit_fmt(context, 4, "object size", "dq %d",
                       class->attributes.count + 3);
    assembler_emit_fmt(context, 4, NULL, "dq %s_dispTab", class_name);

    for (size_t j = 0; j < class->attributes.count; j++) {
        class_mapping_attribute *attr = NULL;
        ds_dynamic_array_get_ref(&class->attributes, j, (void **)&attr);

        assembler_emit_attribute_init(context, attr);
    }
}

static void assembler_emit_object_prototypes(assembler_context *context,
                                             program_node *program,
                                             semantic_mapping *mapping) {
    for (size_t i = 0; i < mapping->parents.classes.count; i++) {
        assembler_emit_object_prototype(context, i, program, mapping);
    }
}

static void assembler_emit_expr(assembler_context *context, size_t i,
                                program_node *program,
                                semantic_mapping *mapping,
                                const expr_node *expr) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, i, (void **)&class);

    ds_dynamic_array tac;
    ds_dynamic_array_init(&tac, sizeof(tac_instr));

    codegen_expr_to_tac(expr, &tac);

    // TODO: generate asm from tac
}

static void assembler_emit_object_init_attribute(assembler_context *context,
                                                 size_t i, size_t j,
                                                 program_node *program,
                                                 semantic_mapping *mapping) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, i, (void **)&class);

    class_mapping_attribute *attr = NULL;
    ds_dynamic_array_get_ref(&class->attributes, j, (void **)&attr);

    assembler_emit_expr(context, i, program, mapping, &attr->attribute->value);
}

static void assembler_emit_object_init_attributes(assembler_context *context,
                                                  size_t i,
                                                  program_node *program,
                                                  semantic_mapping *mapping) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, i, (void **)&class);

    for (size_t j = 0; j < class->attributes.count; j++) {
        assembler_emit_object_init_attribute(context, i, j, program, mapping);
    }
}

static void assembler_emit_object_init(assembler_context *context, size_t i,
                                       program_node *program,
                                       semantic_mapping *mapping) {
    parent_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->parents.classes, i, (void **)&class);

    assembler_emit(context, "segment readable executable");
    assembler_emit_fmt(context, 0, NULL, "%s_init:", class->name);
    assembler_emit_fmt(context, 4, NULL, "push    rbp");
    assembler_emit_fmt(context, 4, NULL, "mov     rbp, rsp");
    assembler_emit_fmt(context, 4, NULL, "push    rbx");
    assembler_emit_fmt(context, 4, "save self", "mov     rbx, rax");
    if (class->parent != NULL) {
        assembler_emit_fmt(context, 4, NULL, "call %s_init",
                           class->parent->name);
    }

    assembler_emit_object_init_attributes(context, i, program, mapping);

    assembler_emit_fmt(context, 4, "restore self", "mov     rax, rbx");
    assembler_emit_fmt(context, 4, NULL, "pop     rbx");
    assembler_emit_fmt(context, 4, NULL, "pop     rbp");
    assembler_emit_fmt(context, 4, NULL, "ret");
}

static void assembler_emit_object_inits(assembler_context *context,
                                        program_node *program,
                                        semantic_mapping *mapping) {
    for (size_t i = 0; i < mapping->parents.classes.count; i++) {
        assembler_emit_object_init(context, i, program, mapping);
    }
}

static void assembler_emit_dispatch_table(assembler_context *context, size_t i,
                                          program_node *program,
                                          semantic_mapping *mapping) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, i, (void **)&class);

    const char *class_name = class->class_name;

    assembler_emit(context, "segment readable");
    assembler_emit_fmt(context, 0, NULL, "%s_dispTab:", class_name);

    for (size_t j = 0; j < mapping->implementations.items.count; j++) {
        implementation_mapping_item *method = NULL;
        ds_dynamic_array_get_ref(&mapping->implementations.items, j,
                                 (void **)&method);

        if (strcmp(method->class_name, class_name) != 0) {
            continue;
        }

        assembler_emit_fmt(context, 4, NULL, "dq %s.%s", method->parent_name,
                           method->method_name);
    }
}

static void assembler_emit_dispatch_tables(assembler_context *context,
                                           program_node *program,
                                           semantic_mapping *mapping) {
    for (size_t i = 0; i < mapping->parents.classes.count; i++) {
        assembler_emit_dispatch_table(context, i, program, mapping);
    }
}

static void assembler_emit_method(assembler_context *context, size_t i,
                                  program_node *program,
                                  semantic_mapping *mapping) {
    implementation_mapping_item *method = NULL;
    ds_dynamic_array_get_ref(&mapping->implementations.items, i,
                             (void **)&method);

    if (strcmp(method->class_name, method->parent_name) != 0) {
        return;
    }

    size_t j = 0;
    for (j = 0; j < mapping->parents.classes.count; j++) {
        parent_mapping_item *class = NULL;
        ds_dynamic_array_get_ref(&mapping->parents.classes, j, (void **)&class);

        if (strcmp(class->name, method->class_name) == 0) {
            break;
        }
    }

    if (method->method->body.kind == EXPR_EXTERN) {
        return;
    }

    assembler_emit(context, "segment readable executable");
    assembler_emit_fmt(context, 0, NULL, "%s.%s:", method->parent_name,
                       method->method_name);
    assembler_emit_fmt(context, 4, NULL, "push    rbp");
    assembler_emit_fmt(context, 4, NULL, "mov     rbp, rsp");
    assembler_emit_fmt(context, 4, NULL, "push    rbx");
    assembler_emit_fmt(context, 4, "save self", "mov     rbx, rax");

    assembler_emit_expr(context, j, program, mapping, &method->method->body);

    assembler_emit_fmt(context, 4, NULL, "pop     rbx");
    assembler_emit_fmt(context, 4, NULL, "pop     rbp");
    assembler_emit_fmt(context, 4, NULL, "ret");
}

static void assembler_emit_methods(assembler_context *context,
                                   program_node *program,
                                   semantic_mapping *mapping) {
    for (size_t i = 0; i < mapping->implementations.items.count; i++) {
        assembler_emit_method(context, i, program, mapping);
    }
}

enum assembler_result assembler_run(const char *filename, program_node *program,
                                    semantic_mapping *mapping) {

    int result = 0;
    assembler_context context;
    if (assembler_context_init(&context, filename, program, mapping) != 0) {
        return_defer(1);
    }

    int int_tag = 0, str_tag = 0, bool_tag = 0;
    for (size_t i = 0; i < mapping->parents.classes.count; i++) {
        class_node *class = NULL;
        ds_dynamic_array_get_ref(&mapping->parents.classes, i, (void **)&class);

        if (strcmp(class->name.value, "Int") == 0) {
            int_tag = i;
        } else if (strcmp(class->name.value, "String") == 0) {
            str_tag = i;
        } else if (strcmp(class->name.value, "Bool") == 0) {
            bool_tag = i;
        }
    }
    context.int_tag = int_tag;
    context.str_tag = str_tag;
    context.bool_tag = bool_tag;

    assembler_emit_class_name_table(&context, program, mapping);
    assembler_emit_class_object_table(&context, program, mapping);
    assembler_emit_object_prototypes(&context, program, mapping);
    assembler_emit_object_inits(&context, program, mapping);
    assembler_emit_dispatch_tables(&context, program, mapping);
    assembler_emit_methods(&context, program, mapping);
    assembler_emit_consts(&context, program, mapping);

defer:
    result = context.result;
    assembler_context_destroy(&context);

    return result;
}
