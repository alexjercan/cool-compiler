#include "assembler.h"
#include "codegen.h"
#include "ds.h"
#include "parser.h"
#include "semantic.h"
#include "stdio.h"
#include <stdarg.h>

#define ASM_INDENT_SIZE 4

#define WORD_SIZE 8
#define LOCALS_OFFSET 8
#define ARGUMENTS_OFFSET 16
#define DISPTABLE_OFFSET 16
#define ATTRIBUTE_OFFSET 24

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
        semantic_mapping *mapping;
        int result;
        int int_tag;
        int str_tag;
        int bool_tag;
        ds_dynamic_array consts; // asm_const

        // context
        class_mapping_item *current_class;
        implementation_mapping_item *current_method;
} assembler_context;

static int assembler_context_init(assembler_context *context,
                                  const char *filename,
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
            padding = 1;
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
        ds_string_builder sb;
        ds_string_builder_init(&sb);

        size_t length = strlen(c.value.str.value);
        for (size_t i = 0; i < length; i++) {
            ds_string_builder_append(&sb, "%d", c.value.str.value[i]);
            if (i < length - 1) {
                ds_string_builder_appendc(&sb, ',');
            }
        }

        char *str = NULL;
        ds_string_builder_build(&sb, &str);

        if (strlen(str) == 0) {
            assembler_emit_fmt(context, align, "string value", "db 0");
        } else {
            assembler_emit_fmt(context, align, "string value", "db %s,0", str);
        }

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
        assembler_emit_fmt(context, align, "boolean value", "dq %d",
                           c.value.boolean);
        break;
    }
    }
}

static void assembler_emit_consts(assembler_context *context) {
    assembler_emit(context, "segment readable");

    for (size_t i = 0; i < context->consts.count; i++) {
        asm_const *c = NULL;
        ds_dynamic_array_get_ref(&context->consts, i, (void **)&c);

        assembler_emit_const(context, *c);
    }
}

static void assembler_emit_class_name_table(assembler_context *context) {
    assembler_emit(context, "segment readable");
    assembler_emit(context, "class_nameTab:");

    for (size_t i = 0; i < context->mapping->parents.classes.count; i++) {
        class_node *class = NULL;
        ds_dynamic_array_get_ref(&context->mapping->parents.classes, i,
                                 (void **)&class);

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
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %s",
                           str_const->name);
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

        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %s",
                           int_const->name);
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

        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %s",
                           bool_const->name);
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

        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %s",
                           str_const->name);
        break;
    }
    case EXPR_EXTERN: {
        if (strcmp(node->type.value, STRING_TYPE) == 0) {
            assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq \"\"");
        } else if (strcmp(node->type.value, INT_TYPE) == 0) {
            assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %d", 0);
        } else if (strcmp(node->type.value, BOOL_TYPE) == 0) {
            assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %d", 0);
        } else {
            assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %d", 0);
        }
        break;
    }
    default:
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "dq %d", 0);
        break;
    }
}

static void assembler_emit_object_prototype(assembler_context *context,
                                            size_t i) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&context->mapping->classes.items, i,
                             (void **)&class);

    const char *class_name = class->class_name;

    assembler_emit(context, "segment readable");
    assembler_emit_fmt(context, 0, NULL, "%s_protObj:", class_name);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, "object tag", "dq %d", i);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, "object size", "dq %d",
                       class->attributes.count + 3);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "dq %s_dispTab",
                       class_name);

    for (size_t j = 0; j < class->attributes.count; j++) {
        class_mapping_attribute *attr = NULL;
        ds_dynamic_array_get_ref(&class->attributes, j, (void **)&attr);

        assembler_emit_attribute_init(context, attr);
    }
}

static void assembler_emit_object_prototypes(assembler_context *context) {
    for (size_t i = 0; i < context->mapping->parents.classes.count; i++) {
        assembler_emit_object_prototype(context, i);
    }
}

// rax <- ident
static void assembler_emit_load_variable(assembler_context *context,
                                         tac_result *tac, char *ident) {
    if (strcmp(ident, "self") == 0) {
        const char *comment = comment_fmt("load self");
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                           "mov     rax, rbx");
        return;
    }

    if (tac != NULL) {
        for (size_t i = 0; i < tac->locals.count; i++) {
            char *local = NULL;
            ds_dynamic_array_get_ref(&tac->locals, i, (void **)&local);

            if (strcmp(local, ident) == 0) {
                int offset = i;
                const char *comment = comment_fmt("load %s", ident);
                assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                                   "mov     rax, qword [rbp-%d]",
                                   LOCALS_OFFSET + WORD_SIZE * offset);
                return;
            }
        }
    }

    implementation_mapping_item *method = context->current_method;
    if (method != NULL) {
        method_node *node = (method_node *)method->method;

        for (size_t i = 0; i < node->formals.count; i++) {
            formal_node *formal = NULL;
            ds_dynamic_array_get_ref(&node->formals, i, (void **)&formal);

            if (strcmp(formal->name.value, ident) == 0) {
                int offset = i;
                const char *comment = comment_fmt("load %s", ident);
                assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                                   "mov     rax, qword [rbp+%d]",
                                   ARGUMENTS_OFFSET + WORD_SIZE * offset);
                return;
            }
        }
    }

    class_mapping_item *class = context->current_class;
    for (size_t i = 0; i < class->attributes.count; i++) {
        class_mapping_attribute *attribute = NULL;
        ds_dynamic_array_get_ref(&class->attributes, i, (void **)&attribute);

        if (strcmp(attribute->name, ident) == 0) {
            int offset = i;
            const char *comment = comment_fmt("load %s", ident);
            assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                               "mov     rax, qword [rbx+%d]",
                               ATTRIBUTE_OFFSET + WORD_SIZE * offset);
            return;
        }
    }

    DS_PANIC("not implemented: rax <- %s", ident);
}

// ident <- rax
static void assembler_emit_store_variable(assembler_context *context,
                                          tac_result *tac, const char *ident) {
    if (tac != NULL) {
        for (size_t i = 0; i < tac->locals.count; i++) {
            char *local = NULL;
            ds_dynamic_array_get_ref(&tac->locals, i, (void **)&local);

            if (strcmp(local, ident) == 0) {
                int offset = i;
                const char *comment = comment_fmt("store %s", ident);
                assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                                   "mov     qword [rbp-%d], rax",
                                   LOCALS_OFFSET + WORD_SIZE * offset);
                return;
            }
        }
    }

    implementation_mapping_item *method = context->current_method;
    if (method != NULL) {
        method_node *node = (method_node *)method->method;

        for (size_t i = 0; i < node->formals.count; i++) {
            formal_node *formal = NULL;
            ds_dynamic_array_get_ref(&node->formals, i, (void **)&formal);

            if (strcmp(formal->name.value, ident) == 0) {
                int offset = i;
                const char *comment = comment_fmt("store %s", ident);
                assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                                   "mov     qword [rbp+%d], rax",
                                   ARGUMENTS_OFFSET + WORD_SIZE * offset);
                return;
            }
        }
    }

    class_mapping_item *class = context->current_class;
    for (size_t i = 0; i < class->attributes.count; i++) {
        class_mapping_attribute *attribute = NULL;
        ds_dynamic_array_get_ref(&class->attributes, i, (void **)&attribute);

        if (strcmp(attribute->name, ident) == 0) {
            int offset = i;
            const char *comment = comment_fmt("store %s", ident);
            assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                               "mov     qword [rbx+%d], rax",
                               ATTRIBUTE_OFFSET + WORD_SIZE * offset);
            return;
        }
    }

    DS_PANIC("not implemented: %s <- rax", ident);
}

// rax <- ident.attr
static void assembler_emit_get_attr(assembler_context *context, tac_result tac,
                                    char *ident, char *type, char *attr) {
    const char *comment = NULL;

    class_mapping_item *class = NULL;
    for (size_t i = 0; i < context->mapping->classes.items.count; i++) {
        class_mapping_item *c = NULL;
        ds_dynamic_array_get_ref(&context->mapping->classes.items, i,
                                 (void **)&c);

        if (strcmp(c->class_name, type) == 0) {
            class = c;
            break;
        }
    }

    if (class == NULL) {
        DS_PANIC("unreachable");
    }

    size_t attribute_slot = 0;
    for (size_t i = 0; i < class->attributes.count; i++) {
        class_mapping_attribute *attribute = NULL;
        ds_dynamic_array_get_ref(&class->attributes, i, (void **)&attribute);

        if (strcmp(attribute->name, attr) == 0) {
            attribute_slot = ATTRIBUTE_OFFSET + WORD_SIZE * i;
            break;
        }
    }

    if (attribute_slot == 0) {
        DS_PANIC("unreachable");
    }

    assembler_emit_load_variable(context, &tac, ident);

    comment = comment_fmt("get %s.%s", ident, attr);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "add     rax, %d",
                       attribute_slot);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                       "mov     rax, qword [rax]");
}

// ident.attr <- rax
static void assembler_emit_set_attr(assembler_context *context, tac_result tac,
                                    char *ident, char *type, char *attr) {
    const char *comment = NULL;

    class_mapping_item *class = NULL;
    for (size_t i = 0; i < context->mapping->classes.items.count; i++) {
        class_mapping_item *c = NULL;
        ds_dynamic_array_get_ref(&context->mapping->classes.items, i,
                                 (void **)&c);

        if (strcmp(c->class_name, type) == 0) {
            class = c;
            break;
        }
    }

    if (class == NULL) {
        DS_PANIC("unreachable");
    }

    size_t attribute_slot = 0;
    for (size_t i = 0; i < class->attributes.count; i++) {
        class_mapping_attribute *attribute = NULL;
        ds_dynamic_array_get_ref(&class->attributes, i, (void **)&attribute);

        if (strcmp(attribute->name, attr) == 0) {
            attribute_slot = ATTRIBUTE_OFFSET + WORD_SIZE * i;
            break;
        }
    }

    if (attribute_slot == 0) {
        DS_PANIC("unreachable");
    }

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    assembler_emit_load_variable(context, &tac, ident);

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "xchg    rdi, rax");

    comment = comment_fmt("set %s.%s", ident, attr);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "add     rdi, %d",
                       attribute_slot);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment,
                       "mov     qword [rdi], rax");
}

// rax <- new TYPE
static void assembler_emit_new_type(assembler_context *context, char *type) {
    const char *comment = NULL;

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL,
                       "mov     rax, %s_protObj", type);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "call    Object.copy");
    comment = comment_fmt("new %s", type);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "call    %s_init",
                       type);
}

// TAC => ASM
static void assembler_emit_tac_dispatch_call(assembler_context *context,
                                             tac_result tac,
                                             tac_dispatch_call instr);

static void assembler_emit_tac_label(assembler_context *context, tac_result tac,
                                     tac_label label) {
    assembler_emit_fmt(context, 0, NULL, ".%s:", label.label);
}

static void assembler_emit_tac_jump(assembler_context *context, tac_result tac,
                                    tac_jump jump) {
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "jmp     .%s",
                       jump.label);
}

static void assembler_emit_tac_jump_if_true(assembler_context *context,
                                            tac_result tac,
                                            tac_jump_if_true jump) {
    const char *comment;

    assembler_emit_get_attr(context, tac, jump.expr, "Bool", "val");

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "test    rax, rax");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "jnz     .%s",
                       jump.label);
}

static void assembler_emit_tac_assign_isinstance(assembler_context *context,
                                                 tac_result tac,
                                                 tac_isinstance instr) {
    // TODO: does not feel right we might actually need the dispatch table after
    // all

    const char *comment = NULL;

    asm_const *type_const = NULL;
    assembler_find_const(
        context,
        (asm_const_value){.type = ASM_CONST_STR, .str = {NULL, instr.type}},
        &type_const);

    ds_dynamic_array args;
    ds_dynamic_array_init(&args, sizeof(char *));

    tac_dispatch_call type_name_call = {
        .ident = instr.ident,
        .expr = instr.expr,
        .type = "Object",
        .method = "type_name",
        .args = args,
    };

    // t0 <- expr@Object.type_name()
    assembler_emit_tac_dispatch_call(context, tac, type_name_call);

    assembler_emit_load_variable(context, &tac, instr.ident);

    // check if address of t0 is equal to address of type
    comment = comment_fmt("isinstance %s", instr.type);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "cmp     rax, %s",
                       type_const->name);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "sete    al");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "movzx   rax, al");

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "push    rax");

    // t0 <- new Bool
    assembler_emit_new_type(context, "Bool");
    assembler_emit_store_variable(context, &tac, instr.ident);

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "pop     rax");

    // set t0.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Bool", "val");
}

static void assembler_emit_tac_assign_cast(assembler_context *context,
                                           tac_result tac,
                                           tac_cast isinstance) {
    // TODO: might need to change the tag of the object

    // t0 <- expr
    assembler_emit_load_variable(context, &tac, isinstance.expr);
    assembler_emit_store_variable(context, &tac, isinstance.ident);
}

static void assembler_emit_tac_assign_value(assembler_context *context,
                                            tac_result tac,
                                            tac_assign_value instr) {
    // t0 <- value
    assembler_emit_load_variable(context, &tac, instr.expr);
    assembler_emit_store_variable(context, &tac, instr.ident);
}

static void assembler_emit_tac_dispatch_call(assembler_context *context,
                                             tac_result tac,
                                             tac_dispatch_call instr) {
    for (size_t i = 0; i < instr.args.count; i++) {
        char *arg = NULL;
        ds_dynamic_array_get(&instr.args, instr.args.count - i - 1, &arg);

        assembler_emit_load_variable(context, &tac, arg);

        const char *comment = comment_fmt("arg0: %s", arg);
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "push    rax");
    }

    if (instr.expr == NULL) {
        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rax, rbx");
    } else {
        assembler_emit_load_variable(context, &tac, instr.expr);
    }

    if (instr.type == NULL) {
        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, qword [rax+%d]", DISPTABLE_OFFSET);

        size_t method_index = 0;
        const char *class_name = NULL;
        for (size_t i = 0; i < context->mapping->implementations.items.count; i++) {
            implementation_mapping_item *method = NULL;
            ds_dynamic_array_get_ref(&context->mapping->implementations.items, i,
                                     (void **)&method);

            if (class_name == NULL || strcmp(method->class_name, class_name) != 0) {
                class_name = method->class_name;
                method_index = 0;
            }

            if (strcmp(method->method_name, instr.method) == 0) {
                break;
            }

            method_index++;
        }

        size_t method_offset = method_index * WORD_SIZE;
        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, qword [rdi+%d]", method_offset);
        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "call    rdi");
    } else {
        for (size_t i = 0; i < context->mapping->implementations.items.count; i++) {
            implementation_mapping_item *method = NULL;
            ds_dynamic_array_get_ref(&context->mapping->implementations.items, i,
                                     (void **)&method);

            const char *class_name = instr.type;

            if (strcmp(method->class_name, class_name) != 0) {
                continue;
            }

            if (strcmp(method->method_name, instr.method) != 0) {
                continue;
            }

            assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "call    %s.%s",
                               method->parent_name, method->method_name);

            break;
        }
    }

    assembler_emit_store_variable(context, &tac, instr.ident);

    const char *comment = comment_fmt("free %d args", instr.args.count);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "add     rsp, %d",
                       WORD_SIZE * instr.args.count);
}

static void assembler_emit_tac_assign_new(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_new instr) {
    // t0 <- new TYPE
    assembler_emit_new_type(context, instr.type);
    assembler_emit_store_variable(context, &tac, instr.ident);
}

static void assembler_emit_tac_assign_default(assembler_context *context,
                                              tac_result tac,
                                              tac_assign_new instr) {
    // t0 <- default TYPE
    if (strcmp(instr.type, "Int") == 0) {
        asm_const *int_const = NULL;
        assembler_new_const(
            context, (asm_const_value){.type = ASM_CONST_INT, .integer = 0},
            &int_const);

        const char *comment = comment_fmt("default %s", instr.type);
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "mov     rax, %s",
                           int_const->name);
    } else if (strcmp(instr.type, "String") == 0) {
        asm_const *int_const = NULL;
        assembler_new_const(
            context, (asm_const_value){.type = ASM_CONST_INT, .integer = 0},
            &int_const);

        asm_const *str_const = NULL;
        assembler_new_const(context,
                            (asm_const_value){.type = ASM_CONST_STR,
                                              .str = {int_const->name, ""}},
                            &str_const);

        const char *comment = comment_fmt("default %s", instr.type);
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "mov     rax, %s",
                           str_const->name);
    } else if (strcmp(instr.type, "Bool") == 0) {
        asm_const *bool_const = NULL;
        assembler_new_const(
            context, (asm_const_value){.type = ASM_CONST_BOOL, .boolean = 0},
            &bool_const);

        const char *comment = comment_fmt("default %s", instr.type);
        assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "mov     rax, %s",
                           bool_const->name);
    } else {
        // any other type is null
        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rax, 0");
    }
    assembler_emit_store_variable(context, &tac, instr.ident);
}

static void assembler_emit_tac_assign_isvoid(assembler_context *context,
                                             tac_result tac,
                                             tac_assign_unary instr) {
    // t0 <- new Bool
    assembler_emit_new_type(context, "Bool");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // compare expr to 0
    assembler_emit_load_variable(context, &tac, instr.expr);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "test    rax, rax");

    // set rax to 1 if rax == 0
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "setz    al");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "movzx   rax, al");

    // set t0.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Bool", "val");
}

static void assembler_emit_tac_assign_add(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_binary instr) {
    const char *comment;

    // t0 <- new Int
    assembler_emit_new_type(context, "Int");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rdi to t1
    assembler_emit_get_attr(context, tac, instr.lhs, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    // set rax to t2
    assembler_emit_get_attr(context, tac, instr.rhs, "Int", "val");

    // set rax to t1 + t2
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "add     rax, rdi");

    // set t0.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Int", "val");
}

static void assembler_emit_tac_assign_sub(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_binary instr) {
    const char *comment;

    // t0 <- new Int
    assembler_emit_new_type(context, "Int");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rax to t2
    assembler_emit_get_attr(context, tac, instr.rhs, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    // set rdi to t1
    assembler_emit_get_attr(context, tac, instr.lhs, "Int", "val");

    // set rax to t1 - t2
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "sub     rax, rdi");

    // set t0.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Int", "val");
}

static void assembler_emit_tac_assign_mul(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_binary instr) {
    const char *comment;

    // t0 <- new Int
    assembler_emit_new_type(context, "Int");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rdi to t1
    assembler_emit_get_attr(context, tac, instr.lhs, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    // set rax to t2
    assembler_emit_get_attr(context, tac, instr.rhs, "Int", "val");

    // set rax to t1 * t2
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mul     rdi");

    // set t0.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Int", "val");
}

static void assembler_emit_tac_assign_div(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_binary instr) {
    const char *comment;

    // t0 <- new Int
    assembler_emit_new_type(context, "Int");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rax to t2
    assembler_emit_get_attr(context, tac, instr.rhs, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    // set rdi to t1
    assembler_emit_get_attr(context, tac, instr.lhs, "Int", "val");

    // set rax to t1 / t2
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "xor     rdx, rdx");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "div     rdi");

    // set t0.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Int", "val");
}

static void assembler_emit_tac_assign_neg(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_unary instr) {
    const char *comment;

    // t0 <- new Int
    assembler_emit_new_type(context, "Int");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rax to ~t0.val
    assembler_emit_get_attr(context, tac, instr.expr, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "neg     rax");

    // set t1 to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Bool", "val");
}

static void assembler_emit_tac_assign_lt(assembler_context *context,
                                         tac_result tac,
                                         tac_assign_binary instr) {
    const char *comment;

    // t2 <- new Bool
    assembler_emit_new_type(context, "Bool");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rdi to t0
    assembler_emit_get_attr(context, tac, instr.lhs, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    // set rax to t1
    assembler_emit_get_attr(context, tac, instr.rhs, "Int", "val");

    // set rax to t0 < t1
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "cmp     rdi, rax");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "setl    al");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "and     al, 1");
    comment = comment_fmt("%s.val < %s.val", instr.lhs, instr.rhs);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "movzx   rax, al");

    // set t2.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Bool", "val");
}

static void assembler_emit_tac_assign_le(assembler_context *context,
                                         tac_result tac,
                                         tac_assign_binary instr) {
    const char *comment;

    // t2 <- new Bool
    assembler_emit_new_type(context, "Bool");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rdi to t0
    assembler_emit_get_attr(context, tac, instr.lhs, "Int", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rdi, rax");

    // set rax to t1
    assembler_emit_get_attr(context, tac, instr.rhs, "Int", "val");

    // set rax to t0 <= t1
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "cmp     rdi, rax");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "setle   al");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "and     al, 1");
    comment = comment_fmt("%s.val < %s.val", instr.lhs, instr.rhs);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "movzx   rax, al");

    // set t2.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Bool", "val");
}

static void assembler_emit_tac_assign_eq(assembler_context *context,
                                         tac_result tac, tac_assign_eq instr) {
    ds_dynamic_array args;
    ds_dynamic_array_init(&args, sizeof(char *));

    ds_dynamic_array_append(&args, &instr.rhs);

    tac_dispatch_call dispatch_call = {
        .ident = instr.ident,
        .expr = instr.lhs,
        .type = instr.type,
        .method = "equals",
        .args = args,
    };

    // t0 <- lhs@type.equals(rhs)
    assembler_emit_tac_dispatch_call(context, tac, dispatch_call);
}

static void assembler_emit_tac_assign_not(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_unary instr) {
    const char *comment;
    int offset;

    // t1 <- new Bool
    assembler_emit_new_type(context, "Bool");
    assembler_emit_store_variable(context, &tac, instr.ident);

    // set rax to not t0.val
    assembler_emit_get_attr(context, tac, instr.expr, "Bool", "val");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "xor     rax, 1");

    // set t1.val to rax
    assembler_emit_set_attr(context, tac, instr.ident, "Bool", "val");
}

static void assembler_emit_tac_ident(assembler_context *context, tac_result tac,
                                     tac_ident instr) {
    assembler_emit_load_variable(context, &tac, instr.name);
}

static void assembler_emit_tac_assign_int(assembler_context *context,
                                          tac_result tac,
                                          tac_assign_int instr) {
    asm_const *int_const = NULL;
    assembler_new_const(
        context,
        (asm_const_value){.type = ASM_CONST_INT, .integer = instr.value},
        &int_const);

    const char *comment = comment_fmt("load %d", instr.value);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "mov     rax, %s",
                       int_const->name);
    assembler_emit_store_variable(context, &tac, instr.ident);
}

static void assembler_emit_tac_assign_string(assembler_context *context,
                                             tac_result tac,
                                             tac_assign_string instr) {
    asm_const *int_const = NULL;
    assembler_new_const(context,
                        (asm_const_value){.type = ASM_CONST_INT,
                                          .integer = strlen(instr.value)},
                        &int_const);

    asm_const *str_const = NULL;
    assembler_new_const(
        context,
        (asm_const_value){.type = ASM_CONST_STR,
                          .str = {int_const->name, instr.value}},
        &str_const);

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rax, %s",
                       str_const->name);
    assembler_emit_store_variable(context, &tac, instr.ident);
}

static void assembler_emit_tac_assign_bool(assembler_context *context,
                                           tac_result tac,
                                           tac_assign_bool instr) {
    asm_const *bool_const = NULL;
    assembler_new_const(
        context,
        (asm_const_value){.type = ASM_CONST_BOOL, .boolean = instr.value},
        &bool_const);

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rax, %s",
                       bool_const->name);
    assembler_emit_store_variable(context, &tac, instr.ident);
}

static void assembler_emit_tac(assembler_context *context, tac_result tac,
                               size_t instr_idx) {
    tac_instr *instr = NULL;
    ds_dynamic_array_get_ref(&tac.instrs, instr_idx, (void **)&instr);

    switch (instr->kind) {
    case TAC_LABEL:
        return assembler_emit_tac_label(context, tac, instr->label);
    case TAC_JUMP:
        return assembler_emit_tac_jump(context, tac, instr->jump);
    case TAC_JUMP_IF_TRUE:
        return assembler_emit_tac_jump_if_true(context, tac,
                                               instr->jump_if_true);
    case TAC_ASSIGN_ISINSTANCE:
        return assembler_emit_tac_assign_isinstance(context, tac,
                                                    instr->isinstance);
    case TAC_CAST:
        return assembler_emit_tac_assign_cast(context, tac, instr->cast);
    case TAC_ASSIGN_VALUE:
        return assembler_emit_tac_assign_value(context, tac,
                                               instr->assign_value);
    case TAC_DISPATCH_CALL:
        return assembler_emit_tac_dispatch_call(context, tac,
                                                instr->dispatch_call);
    case TAC_ASSIGN_NEW:
        return assembler_emit_tac_assign_new(context, tac, instr->assign_new);
    case TAC_ASSIGN_DEFAULT:
        return assembler_emit_tac_assign_default(context, tac,
                                                 instr->assign_new);
    case TAC_ASSIGN_ISVOID:
        return assembler_emit_tac_assign_isvoid(context, tac,
                                                instr->assign_unary);
    case TAC_ASSIGN_ADD:
        return assembler_emit_tac_assign_add(context, tac,
                                             instr->assign_binary);
    case TAC_ASSIGN_SUB:
        return assembler_emit_tac_assign_sub(context, tac,
                                             instr->assign_binary);
    case TAC_ASSIGN_MUL:
        return assembler_emit_tac_assign_mul(context, tac,
                                             instr->assign_binary);
    case TAC_ASSIGN_DIV:
        return assembler_emit_tac_assign_div(context, tac,
                                             instr->assign_binary);
    case TAC_ASSIGN_NEG:
        return assembler_emit_tac_assign_neg(context, tac, instr->assign_unary);
    case TAC_ASSIGN_LT:
        return assembler_emit_tac_assign_lt(context, tac, instr->assign_binary);
    case TAC_ASSIGN_LE:
        return assembler_emit_tac_assign_le(context, tac, instr->assign_binary);
    case TAC_ASSIGN_EQ:
        return assembler_emit_tac_assign_eq(context, tac, instr->assign_eq);
    case TAC_ASSIGN_NOT:
        return assembler_emit_tac_assign_not(context, tac, instr->assign_unary);
    case TAC_IDENT:
        return assembler_emit_tac_ident(context, tac, instr->ident);
    case TAC_ASSIGN_INT:
        return assembler_emit_tac_assign_int(context, tac, instr->assign_int);
    case TAC_ASSIGN_STRING:
        return assembler_emit_tac_assign_string(context, tac,
                                                instr->assign_string);
    case TAC_ASSIGN_BOOL:
        return assembler_emit_tac_assign_bool(context, tac, instr->assign_bool);
        break;
    }
}

static void assembler_emit_expr(assembler_context *context,
                                const expr_node *expr) {
    tac_result tac;
    codegen_expr_to_tac(expr, &tac);

    const char *comment = comment_fmt("allocate %d locals", tac.locals.count);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, comment, "sub     rsp, %d",
                       WORD_SIZE * tac.locals.count);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "push    rbx");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rbx, rax");

    for (size_t j = 0; j < tac.instrs.count; j++) {
        assembler_emit_tac(context, tac, j);
    }

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "pop     rbx");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "add     rsp, %d",
                       WORD_SIZE * tac.locals.count);
}

static void assembler_emit_object_init_attribute(assembler_context *context,
                                                 size_t attr_idx) {
    class_mapping_item *class = context->current_class;

    class_mapping_attribute *attr = NULL;
    ds_dynamic_array_get_ref(&class->attributes, attr_idx, (void **)&attr);

    if (attr->attribute->value.kind == EXPR_EXTERN) {
        return;
    }

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rax, rbx");
    assembler_emit_expr(context, &attr->attribute->value);

    const char *comment = comment_fmt("init %s", attr->name);
    assembler_emit_store_variable(context, NULL, attr->name);
}

static void assembler_emit_object_init_attributes(assembler_context *context) {
    class_mapping_item *class = context->current_class;

    for (size_t j = 0; j < class->attributes.count; j++) {
        assembler_emit_object_init_attribute(context, j);
    }
}

static void assembler_emit_object_init(assembler_context *context,
                                       size_t class_idx) {
    parent_mapping_item *classp = NULL;
    ds_dynamic_array_get_ref(&context->mapping->parents.classes, class_idx,
                             (void **)&classp);

    assembler_emit(context, "segment readable executable");
    assembler_emit_fmt(context, 0, NULL, "%s_init:", classp->name);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "push    rbp");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rbp, rsp");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "push    rbx");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, "save self",
                       "mov     rbx, rax");
    if (classp->parent != NULL) {
        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "call    %s_init",
                           classp->parent->name);
    }

    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&context->mapping->classes.items, class_idx,
                             (void **)&class);

    context->current_class = class;
    context->current_method = NULL;
    assembler_emit_object_init_attributes(context);
    context->current_class = NULL;

    assembler_emit_fmt(context, ASM_INDENT_SIZE, "restore self",
                       "mov     rax, rbx");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "pop     rbx");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "pop     rbp");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "ret");
}

static void assembler_emit_object_inits(assembler_context *context) {
    for (size_t i = 0; i < context->mapping->parents.classes.count; i++) {
        assembler_emit_object_init(context, i);
    }
}

static void assembler_emit_method(assembler_context *context,
                                  size_t method_idx) {
    implementation_mapping_item *method = NULL;
    ds_dynamic_array_get_ref(&context->mapping->implementations.items,
                             method_idx, (void **)&method);

    if (strcmp(method->class_name, method->parent_name) != 0) {
        return;
    }

    class_mapping_item *class = NULL;
    for (size_t j = 0; j < context->mapping->classes.items.count; j++) {
        ds_dynamic_array_get_ref(&context->mapping->classes.items, j,
                                 (void **)&class);

        if (strcmp(class->class_name, method->class_name) == 0) {
            break;
        }
    }

    if (method->method->body.kind == EXPR_EXTERN) {
        return;
    }

    assembler_emit(context, "segment readable executable");
    assembler_emit_fmt(context, 0, NULL, "%s.%s:", method->parent_name,
                       method->method_name);
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "push    rbp");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "mov     rbp, rsp");

    context->current_class = class;
    context->current_method = method;
    assembler_emit_expr(context, &method->method->body);
    context->current_method = NULL;
    context->current_class = NULL;

    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "pop     rbp");
    assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "ret");
}

static void assembler_emit_methods(assembler_context *context) {
    for (size_t i = 0; i < context->mapping->implementations.items.count; i++) {
        assembler_emit_method(context, i);
    }
}

static void assembler_emit_dispatch_table(assembler_context *context,
                                          size_t class_idx) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&context->mapping->classes.items, class_idx,
                             (void **)&class);

    const char *class_name = class->class_name;

    assembler_emit(context, "segment readable");
    assembler_emit_fmt(context, 0, NULL, "%s_dispTab:", class_name);

    for (size_t j = 0; j < context->mapping->implementations.items.count; j++) {
        implementation_mapping_item *method = NULL;
        ds_dynamic_array_get_ref(&context->mapping->implementations.items, j,
                                 (void **)&method);

        if (strcmp(method->class_name, class_name) != 0) {
            continue;
        }

        assembler_emit_fmt(context, ASM_INDENT_SIZE, NULL, "dq %s.%s",
                           method->parent_name, method->method_name);
    }
}

static void assembler_emit_dispatch_tables(assembler_context *context) {
    for (size_t i = 0; i < context->mapping->parents.classes.count; i++) {
        assembler_emit_dispatch_table(context, i);
    }
}

enum assembler_result assembler_run(const char *filename,
                                    semantic_mapping *mapping) {

    int result = 0;
    assembler_context context;
    if (assembler_context_init(&context, filename, mapping) != 0) {
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

    assembler_emit_class_name_table(&context);
    assembler_emit_dispatch_tables(&context);
    assembler_emit_object_prototypes(&context);
    assembler_emit_object_inits(&context);
    assembler_emit_methods(&context);
    assembler_emit_consts(&context);

defer:
    result = context.result;
    assembler_context_destroy(&context);

    return result;
}
