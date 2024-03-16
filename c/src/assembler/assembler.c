#include "assembler.h"
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
        union {
                struct {
                        const char *len_label;
                        const char *value;
                } str;
                int64_t integer;
                int8_t boolean;
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

static void assembler_new_const(assembler_context *context,
                                asm_const_value value, asm_const **result) {
    // TODO: Maybe this can be optimized by checking for duplicates

    *result = NULL;

    int count = context->consts.count;
    const char *prefix = NULL;

    switch (value.type) {
    case ASM_CONST_STR: {
        prefix = "str_const";
        break;
    }
    case ASM_CONST_INT: {
        prefix = "int_const";
        break;
    }
    case ASM_CONST_BOOL: {
        prefix = "bool_const";
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
    assembler_emit_fmt(context, 0, "type tag", "%s dw %d", c.name,
                       c.value.type);

    switch (c.value.type) {
    case ASM_CONST_STR: {
        assembler_emit_fmt(context, align, "pointer to length", "dq %s",
                           c.value.str.len_label);
        assembler_emit_fmt(context, align, "string value", "db \"%s\", 0",
                           c.value.str.value);
        break;
    }
    case ASM_CONST_INT: {
        assembler_emit_fmt(context, align, "integer value", "dq %d",
                           c.value.integer);
        break;
    }
    case ASM_CONST_BOOL: {
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
    assembler_emit(context, "_int_tag dw %d", ASM_CONST_INT);
    assembler_emit(context, "_string_tag dw %d", ASM_CONST_STR);
    assembler_emit(context, "_bool_tag dw %d", ASM_CONST_BOOL);

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

        assembler_emit_fmt(context, 4, "pointer to class name", "dq %s",
                           str_const->name);
    }
}

enum assembler_result assembler_run(const char *filename, program_node *program,
                                    semantic_mapping *mapping) {

    int result = 0;
    assembler_context context;
    if (assembler_context_init(&context, filename, program, mapping) != 0) {
        return_defer(1);
    }

    assembler_emit(&context, "format ELF64 executable 3");
    assembler_emit(&context, "entry start");
    assembler_emit(&context, "segment readable executable");
    assembler_emit(&context, "start:\n");
    assembler_emit(&context, "    mov     rax, 60");
    assembler_emit(&context, "    xor     rdi, rdi");
    assembler_emit(&context, "    syscall");

    assembler_emit_class_name_table(&context, program, mapping);
    assembler_emit_consts(&context, program, mapping);

defer:
    result = context.result;
    assembler_context_destroy(&context);

    return result;
}
