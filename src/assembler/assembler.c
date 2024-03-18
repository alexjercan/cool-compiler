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
        assembler_emit_fmt(context, align, "object size", "dq %d", 4);
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

        assembler_emit_fmt(context, align, "string value", "db %s,0", str);

        break;
    }
    case ASM_CONST_INT: {
        assembler_emit_fmt(context, align, "object size", "dq %d", 3);
        assembler_emit_fmt(context, align, "integer value", "dq %d",
                           c.value.integer);
        break;
    }
    case ASM_CONST_BOOL: {
        assembler_emit_fmt(context, align, "object size", "dq %d", 3);
        assembler_emit_fmt(context, align, "boolean value", "dq %d",
                           c.value.boolean);
        break;
    }
    }
}

static void assembler_emit_consts(assembler_context *context,
                                  program_node *program,
                                  semantic_mapping *mapping) {
    assembler_emit(context, "segment readable");
    assembler_emit(context, "_string_tag dq %d", context->str_tag);
    assembler_emit(context, "_int_tag dq %d", context->int_tag);
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
                       class->attributes.count + 2);

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

static void assembler_get_stack_offset(assembler_context *context,
                                       size_t class_idx, program_node *program,
                                       semantic_mapping *mapping,
                                       tac_result tac, char *ident,
                                       int *offset) {
    for (size_t i = 0; i < tac.locals.count; i++) {
        char *local = NULL;
        ds_dynamic_array_get_ref(&tac.locals, i, (void **)&local);

        if (strcmp(local, ident) == 0) {
            *offset = i + 1;
            return;
        }
    }

    return;
}

static void assembler_emit_tac_label(assembler_context *context,
                                     size_t class_idx, program_node *program,
                                     semantic_mapping *mapping, tac_result tac,
                                     tac_label label) {
    assembler_emit_fmt(context, 0, NULL, ".%s:", label.label);
}

static void assembler_emit_tac_jump(assembler_context *context,
                                    size_t class_idx, program_node *program,
                                    semantic_mapping *mapping, tac_result tac,
                                    tac_jump jump) {
    assembler_emit_fmt(context, 4, NULL, "jmp     .%s", jump.label);
}

static void assembler_emit_tac_jump_if_true(
    assembler_context *context, size_t class_idx, program_node *program,
    semantic_mapping *mapping, tac_result tac, tac_jump_if_true jump) {
    int offset = 0;
    const char *comment;

    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               jump.expr, &offset);

    comment = comment_fmt("get %s", jump.expr);
    assembler_emit_fmt(context, 4, comment, "mov     rax, qword [rbp-%d]",
                       8 + 8 * offset);
    comment = comment_fmt("access bool value");
    assembler_emit_fmt(context, 4, comment, "add     rax, [bool_slot]");
    comment = comment_fmt("dereference bool");
    assembler_emit_fmt(context, 4, comment, "mov     rax, qword [rax]");

    assembler_emit_fmt(context, 4, NULL, "test    rax, rax");
    assembler_emit_fmt(context, 4, NULL, "jnz     .%s", jump.label);
}

static void assembler_emit_tac_assign_value(
    assembler_context *context, size_t class_idx, program_node *program,
    semantic_mapping *mapping, tac_result tac, tac_assign_value instr) {
    const char *comment;
    int offset;

    offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.expr, &offset);

    comment = comment_fmt("load %s", instr.expr);
    assembler_emit_fmt(context, 4, comment, "mov     rax, qword [rbp-%d]",
                       8 + 8 * offset);

    offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.ident, &offset);

    comment = comment_fmt("store %s in %s", instr.expr, instr.ident);
    assembler_emit_fmt(context, 4, comment, "mov     qword [rbp-%d], rax",
                       8 + 8 * offset);
}

static void assembler_emit_tac_dispatch_call(
    assembler_context *context, size_t class_idx, program_node *program,
    semantic_mapping *mapping, tac_result tac, tac_dispatch_call instr) {
    const char *comment;

    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, class_idx,
                             (void **)&class);

    int offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.ident, &offset);

    for (size_t i = 0; i < instr.args.count; i++) {
        char *arg;
        ds_dynamic_array_get(&instr.args, instr.args.count - i - 1, &arg);

        int offset = 0;
        assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                                   arg, &offset);

        comment = comment_fmt("arg0: %s", arg);
        assembler_emit_fmt(context, 4, comment, "push    qword [rbp-%d]",
                           8 + 8 * offset);
    }

    if (strcmp(instr.expr, "self") == 0) {
        comment = comment_fmt("get self");
        assembler_emit_fmt(context, 4, comment, "mov     rax, rbx");
    } else {
        int offset = 0;
        assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                                   instr.expr, &offset);

        comment = comment_fmt("get %s", instr.expr);
        assembler_emit_fmt(context, 4, comment, "mov     rax, qword [rbp-%d]",
                           8 + 8 * offset);
    }

    for (size_t i = 0; i < mapping->implementations.items.count; i++) {
        implementation_mapping_item *method = NULL;
        ds_dynamic_array_get_ref(&mapping->implementations.items, i,
                                 (void **)&method);

        if (strcmp(method->class_name, class->class_name) != 0) {
            continue;
        }

        if (strcmp(method->method_name, instr.method) != 0) {
            continue;
        }

        assembler_emit_fmt(context, 4, NULL, "call    %s.%s",
                           method->parent_name, method->method_name);

        break;
    }

    comment = comment_fmt("save result in %s", instr.ident);
    assembler_emit_fmt(context, 4, comment, "mov     qword [rbp-%d], rax",
                       8 + 8 * offset);

    comment = comment_fmt("free %d args", instr.args.count);
    assembler_emit_fmt(context, 4, comment, "add     rsp, %d",
                       8 * instr.args.count);
}
static void
assembler_emit_tac_assign_not(assembler_context *context, size_t class_idx,
                              program_node *program, semantic_mapping *mapping,
                              tac_result tac, tac_assign_unary instr) {
    const char *comment;
    int offset;

    offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.expr, &offset);

    comment = comment_fmt("load %s", instr.expr);
    assembler_emit_fmt(context, 4, comment, "mov     rax, qword [rbp-%d]",
                       8 + 8 * offset);

    comment = comment_fmt("copy the object");
    assembler_emit_fmt(context, 4, comment, "call    Object.copy");

    comment = comment_fmt("access the bool value");
    assembler_emit_fmt(context, 4, comment, "add     rax, qword [bool_slot]");

    comment = comment_fmt("flip the bool value");
    assembler_emit_fmt(context, 4, comment, "xor     qword [rax], 1");

    offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.ident, &offset);

    comment = comment_fmt("store not %s in %s", instr.expr, instr.ident);
    assembler_emit_fmt(context, 4, comment, "mov     qword [rbp-%d], rax",
                       8 + 8 * offset);
}

static void assembler_emit_tac_ident(assembler_context *context,
                                     size_t class_idx, program_node *program,
                                     semantic_mapping *mapping, tac_result tac,
                                     tac_ident instr) {
    int offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.name, &offset);

    const char *comment = comment_fmt("load %s", instr.name);
    assembler_emit_fmt(context, 4, comment, "mov     rax, qword [rbp-%d]",
                       8 + 8 * offset);
}

static void
assembler_emit_tac_assign_int(assembler_context *context, size_t class_idx,
                              program_node *program, semantic_mapping *mapping,
                              tac_result tac, tac_assign_int instr) {
    int offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.ident, &offset);

    asm_const *int_const = NULL;
    assembler_new_const(
        context,
        (asm_const_value){.type = ASM_CONST_INT, .integer = instr.value},
        &int_const);

    const char *comment =
        comment_fmt("store %d in %s", instr.value, instr.ident);
    assembler_emit_fmt(context, 4, comment, "mov     qword [rbp-%d], %s",
                       8 + 8 * offset, int_const->name);
}

static void assembler_emit_tac_assign_string(
    assembler_context *context, size_t class_idx, program_node *program,
    semantic_mapping *mapping, tac_result tac, tac_assign_string instr) {
    int offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.ident, &offset);

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

    // TODO: string repr => show instr.value
    const char *comment =
        comment_fmt("store %s in %s", str_const->name, instr.ident);
    assembler_emit_fmt(context, 4, comment, "mov     qword [rbp-%d], %s",
                       8 + 8 * offset, str_const->name);
}

static void
assembler_emit_tac_assign_bool(assembler_context *context, size_t class_idx,
                               program_node *program, semantic_mapping *mapping,
                               tac_result tac, tac_assign_bool instr) {
    int offset = 0;
    assembler_get_stack_offset(context, class_idx, program, mapping, tac,
                               instr.ident, &offset);

    asm_const *bool_const = NULL;
    assembler_new_const(
        context,
        (asm_const_value){.type = ASM_CONST_BOOL, .boolean = instr.value},
        &bool_const);

    const char *comment =
        comment_fmt("store %d in %s", instr.value, instr.ident);
    assembler_emit_fmt(context, 4, comment, "mov     qword [rbp-%d], %s",
                       8 + 8 * offset, bool_const->name);
}

static void assembler_emit_tac(assembler_context *context, size_t class_idx,
                               program_node *program, semantic_mapping *mapping,
                               tac_result tac, size_t instr_idx) {
    tac_instr *instr = NULL;
    ds_dynamic_array_get_ref(&tac.instrs, instr_idx, (void **)&instr);

    // TODO: implement
    switch (instr->kind) {
    case TAC_LABEL:
        return assembler_emit_tac_label(context, class_idx, program, mapping,
                                        tac, instr->label);
    case TAC_JUMP:
        return assembler_emit_tac_jump(context, class_idx, program, mapping,
                                       tac, instr->jump);
    case TAC_JUMP_IF_TRUE:
        return assembler_emit_tac_jump_if_true(
            context, class_idx, program, mapping, tac, instr->jump_if_true);
    case TAC_ASSIGN_ISINSTANCE:
        // return assembler_emit_tac_assign_isinstance(instr->isinstance);
    case TAC_CAST:
        // return assembler_emit_tac_cast(instr->cast);
    case TAC_ASSIGN_VALUE:
        return assembler_emit_tac_assign_value(
            context, class_idx, program, mapping, tac, instr->assign_value);
    case TAC_DISPATCH_CALL:
        return assembler_emit_tac_dispatch_call(
            context, class_idx, program, mapping, tac, instr->dispatch_call);
    case TAC_ASSIGN_NEW:
        // return assembler_emit_tac_assign_new(instr->assign_new);
    case TAC_ASSIGN_ISVOID:
        // return assembler_emit_tac_assign_unary(instr->assign_unary,
        // "isvoid");
    case TAC_ASSIGN_ADD:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "+");
    case TAC_ASSIGN_SUB:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "-");
    case TAC_ASSIGN_MUL:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "*");
    case TAC_ASSIGN_DIV:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "/");
    case TAC_ASSIGN_NEG:
        // return assembler_emit_tac_assign_unary(instr->assign_unary, "~");
    case TAC_ASSIGN_LT:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "<");
    case TAC_ASSIGN_LE:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "<=");
    case TAC_ASSIGN_EQ:
        // return assembler_emit_tac_assign_binary(instr->assign_binary, "=");
    case TAC_ASSIGN_NOT:
        return assembler_emit_tac_assign_not(context, class_idx, program,
                                             mapping, tac, instr->assign_unary);
    case TAC_IDENT:
        return assembler_emit_tac_ident(context, class_idx, program, mapping,
                                        tac, instr->ident);
    case TAC_ASSIGN_INT:
        return assembler_emit_tac_assign_int(context, class_idx, program,
                                             mapping, tac, instr->assign_int);
    case TAC_ASSIGN_STRING:
        return assembler_emit_tac_assign_string(
            context, class_idx, program, mapping, tac, instr->assign_string);
    case TAC_ASSIGN_BOOL:
        return assembler_emit_tac_assign_bool(context, class_idx, program,
                                              mapping, tac, instr->assign_bool);
        break;
    }
}

static void assembler_emit_expr(assembler_context *context, size_t class_idx,
                                program_node *program,
                                semantic_mapping *mapping,
                                const expr_node *expr) {
    tac_result tac;
    codegen_expr_to_tac(expr, &tac);

    const char *comment = comment_fmt("allocate %d locals", tac.locals.count);
    assembler_emit_fmt(context, 4, comment, "sub     rsp, %d",
                       8 * tac.locals.count);

    for (size_t j = 0; j < tac.instrs.count; j++) {
        assembler_emit_tac(context, class_idx, program, mapping, tac, j);
    }

    assembler_emit_fmt(context, 4, "free locals", "add     rsp, %d",
                       8 * tac.locals.count);
}

static void assembler_emit_object_init_attribute(assembler_context *context,
                                                 size_t class_idx,
                                                 size_t attr_idx,
                                                 program_node *program,
                                                 semantic_mapping *mapping) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, class_idx,
                             (void **)&class);

    class_mapping_attribute *attr = NULL;
    ds_dynamic_array_get_ref(&class->attributes, attr_idx, (void **)&attr);

    assembler_emit_expr(context, class_idx, program, mapping,
                        &attr->attribute->value);
}

static void assembler_emit_object_init_attributes(assembler_context *context,
                                                  size_t class_idx,
                                                  program_node *program,
                                                  semantic_mapping *mapping) {
    class_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->classes.items, class_idx,
                             (void **)&class);

    for (size_t j = 0; j < class->attributes.count; j++) {
        assembler_emit_object_init_attribute(context, class_idx, j, program,
                                             mapping);
    }
}

static void assembler_emit_object_init(assembler_context *context,
                                       size_t class_idx, program_node *program,
                                       semantic_mapping *mapping) {
    parent_mapping_item *class = NULL;
    ds_dynamic_array_get_ref(&mapping->parents.classes, class_idx,
                             (void **)&class);

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

    assembler_emit_object_init_attributes(context, class_idx, program, mapping);

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

static void assembler_emit_method(assembler_context *context, size_t method_idx,
                                  program_node *program,
                                  semantic_mapping *mapping) {
    implementation_mapping_item *method = NULL;
    ds_dynamic_array_get_ref(&mapping->implementations.items, method_idx,
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
    assembler_emit_methods(&context, program, mapping);
    assembler_emit_consts(&context, program, mapping);

defer:
    result = context.result;
    assembler_context_destroy(&context);

    return result;
}
