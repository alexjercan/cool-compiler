#include "util.h"
#include <stdio.h>
#include <string.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "assembler.h"
#include "codegen.h"
#include "ds.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

// Add support for the following:
// - abort for dispatch on void
// - abort for case on void
// - abort for case on no match
// - exception handling
//
// Future plans:
// - add a new class Linux for the syscalls and implement a prelude for it
// - implement a better main/build system
// - add graphics to IO

enum status_code {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_STOP = 2,
};

typedef struct build_context {
    struct argparse_parser *parser;
    ds_dynamic_array prelude_filepaths; // const char *
    ds_dynamic_array user_filepaths; // const char *
    ds_dynamic_array asm_filepaths; // const char *

    ds_dynamic_array user_programs; // program_node
    program_node program;
    semantic_mapping mapping;
} build_context;

static int build_context_init(build_context *context, struct argparse_parser *parser) {
    int result = 0;

    context->parser = parser;

    ds_dynamic_array_init(&context->prelude_filepaths, sizeof(const char *));
    ds_dynamic_array_init(&context->user_filepaths, sizeof(const char *));
    ds_dynamic_array_init(&context->asm_filepaths, sizeof(const char *));

    // TODO: add prelude files
    const char *prelude_files[] = {
        "lib/prelude.cl",
    };
    size_t num_prelude_files = sizeof(prelude_files) / sizeof(prelude_files[0]);
    if (ds_dynamic_array_append_many(&context->prelude_filepaths, (void **)prelude_files, num_prelude_files) != 0) {
        DS_LOG_ERROR("Failed to append prelude file");
        return_defer(1);
    }

    // TODO: add asm files
    const char *asm_files[] = {
        "lib/prelude.asm",
    };
    size_t num_asm_files = sizeof(asm_files) / sizeof(asm_files[0]);
    if (ds_dynamic_array_append_many(&context->asm_filepaths, (void **)asm_files, num_asm_files) != 0) {
        DS_LOG_ERROR("Failed to append asm file");
        return_defer(1);
    }

    ds_dynamic_array_init(&context->user_programs, sizeof(program_node));
    ds_dynamic_array_init(&context->program.classes, sizeof(class_node));
    context->mapping = (struct semantic_mapping){0};

defer:
    return result;
}

static enum status_code parse_prelude(build_context *context) {
    int length;
    program_node program;
    char *buffer = NULL;
    ds_dynamic_array tokens; // struct token

    enum parser_result parser_status = PARSER_OK;

    enum status_code result = STATUS_OK;

    for (size_t i = 0; i < context->prelude_filepaths.count; i++) {
        const char *filepath = NULL;
        ds_dynamic_array_get(&context->prelude_filepaths, i, (void **)&filepath);

        length = util_read_file(filepath, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", filepath);
            return_defer(STATUS_ERROR);
        }

        // tokenize prelude
        ds_dynamic_array_init(&tokens, sizeof(struct token));
        if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
            DS_LOG_ERROR("Failed to tokenize input");
            return_defer(STATUS_ERROR);
        }

        // parse tokens
        if (parser_run(filepath, &tokens, &program) != PARSER_OK) {
            parser_status = PARSER_ERROR;
            continue;
        }

        for (unsigned int j = 0; j < program.classes.count; j++) {
            class_node *c = NULL;
            ds_dynamic_array_get_ref(&program.classes, j, (void **)&c);

            if (ds_dynamic_array_append(&context->program.classes, c) != 0) {
                DS_LOG_ERROR("Failed to append class");
                return_defer(1);
            }
        }
    }

    if (parser_status != PARSER_OK) {
        return_defer(STATUS_ERROR);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code parse_user(build_context *context) {
    int length;
    program_node program;
    char *buffer = NULL;
    ds_dynamic_array tokens; // struct token

    int lexer_stop = argparse_get_flag(context->parser, ARG_LEXER);
    int parser_stop = argparse_get_flag(context->parser, ARG_SYNTAX);

    enum parser_result parser_status = PARSER_OK;

    int result = STATUS_OK;

    for (size_t i = 0; i < context->user_filepaths.count; i++) {
        const char *filepath = NULL;
        ds_dynamic_array_get(&context->user_filepaths, i, (void **)&filepath);

        // read input file
        length = util_read_file(filepath, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", filepath);
            return_defer(STATUS_ERROR);
        }

        // tokenize input
        ds_dynamic_array_init(&tokens, sizeof(struct token));
        if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
            DS_LOG_ERROR("Failed to tokenize input");
            return_defer(STATUS_ERROR);
        }

        // TODO: Maybe print filepath\ntokens
        if (lexer_stop == 1) {
            lexer_print_tokens(&tokens);
            continue;
        }

        // parse tokens
        program_node program;
        if (parser_run(filepath, &tokens, &program) != PARSER_OK) {
            parser_status = PARSER_ERROR;
            continue;
        }

        for (unsigned int j = 0; j < program.classes.count; j++) {
            class_node *c = NULL;
            ds_dynamic_array_get_ref(&program.classes, j, (void **)&c);

            if (ds_dynamic_array_append(&context->program.classes, c) != 0) {
                DS_LOG_ERROR("Failed to append class");
                return_defer(STATUS_ERROR);
            }
        }

        if (ds_dynamic_array_append(&context->user_programs, &program) != 0) {
            DS_LOG_ERROR("Failed to append program");
            return_defer(1);
        }
    }

    if (lexer_stop == 1) {
        return_defer(STATUS_STOP);
    }

    if (parser_status != PARSER_OK) {
        return_defer(STATUS_ERROR);
    }

    // TODO: Think about this one: Maybe print filepath\nast
    if (parser_stop == 1) {
        for (size_t i = 0; i < context->user_programs.count; i++) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&context->user_programs, i, (void **)&program);
            parser_print_ast(program);
        }
        return_defer(STATUS_STOP);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code gatekeeping(build_context *context) {
    const char *basename = util_filepath_to_basename(argparse_get_value(context->parser, ARG_INPUT));

    int semantic_stop = argparse_get_flag(context->parser, ARG_SEMANTIC);
    int mapping_stop = argparse_get_flag(context->parser, ARG_MAPPING);

    int result = STATUS_OK;

    // TODO: This should not require basename, the names should be stored in each class node
    if (semantic_check(basename, &context->program, &context->mapping) != SEMANTIC_OK) {
        return_defer(STATUS_ERROR);
    }

    if (semantic_stop == 1) {
        for (size_t i = 0; i < context->user_programs.count; i++) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&context->user_programs, i, (void **)&program);
            parser_print_ast(program);
        }
        return_defer(STATUS_STOP);
    }

    if (mapping_stop == 1) {
        semantic_print_mapping(&context->mapping);
        return_defer(STATUS_STOP);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

static enum status_code codegen(build_context *context) {
    int length;
    char *buffer = NULL;

    char *output = argparse_get_value(context->parser, ARG_OUTPUT);
    int tacgen_stop = argparse_get_flag(context->parser, ARG_TACGEN);
    int assembler_stop = argparse_get_flag(context->parser, ARG_ASSEMBLER);

    int result = STATUS_OK;

    if (tacgen_stop == 1) {
        for (size_t i = 0; i < context->user_programs.count; i++) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&context->user_programs, i, (void **)&program);
            codegen_tac_print(&context->mapping, program);
        }
        return_defer(STATUS_STOP);
    }

    for (size_t i = 0; i < context->asm_filepaths.count; i++) {
        const char *asm_filepath = NULL;
        ds_dynamic_array_get(&context->asm_filepaths, i, (void **)&asm_filepath);

        // read asm prelude file
        length = util_read_file(asm_filepath, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", asm_filepath);
            return_defer(STATUS_ERROR);
        }

        if (util_write_file(output, buffer) != 0) {
            DS_LOG_ERROR("Failed to write file: %s", output);
            return_defer(STATUS_ERROR);
        }
    }

    // assembler
    if (assembler_run(output, &context->mapping) != ASSEMBLER_OK) {
        return_defer(STATUS_ERROR);
    }

    if (assembler_stop == 1) {
        return_defer(STATUS_STOP);
    }

    return_defer(STATUS_OK);

defer:
    return result;
}

int main(int argc, char **argv) {
    int result = 0;
    int length = 0;
    char *buffer = NULL;
    ds_dynamic_array programs;
    ds_dynamic_array tokens;
    struct argparse_parser *parser = NULL;
    struct semantic_mapping mapping = {0};

    ds_dynamic_array_init(&programs, sizeof(program_node));

    parser = util_parse_arguments(argc, argv);
    if (parser == NULL) {
        DS_LOG_ERROR("Failed to parse arguments");
        return_defer(1);
    }

    char *filename = argparse_get_value(parser, ARG_INPUT);
    const char *basename = util_filepath_to_basename(filename);
    char *output = argparse_get_value(parser, ARG_OUTPUT);

    const char *prelude_asm = "lib/prelude.asm";

    build_context context;
    build_context_init(&context, parser);

    // TODO: Take from args
    ds_dynamic_array_append(&context.user_filepaths, &filename);

    int prelude_result = parse_prelude(&context);
    int user_result = parse_user(&context);
    if (prelude_result == STATUS_STOP || user_result == STATUS_STOP) {
        return_defer(0);
    }
    if (prelude_result != STATUS_OK || user_result != STATUS_OK) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    int gatekeeping_result = gatekeeping(&context);
    if (gatekeeping_result == STATUS_STOP) {
        return_defer(0);
    }
    if (gatekeeping_result != STATUS_OK) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    int codegen_result = codegen(&context);
    if (codegen_result == STATUS_STOP) {
        return_defer(0);
    }
    if (codegen_result != STATUS_OK) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    return_defer(0);

defer:
    return result;
}
