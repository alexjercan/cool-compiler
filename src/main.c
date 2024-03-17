#include "util.h"
#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "assembler.h"
#include "codegen.h"
#include "ds.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

// Add support for the following:
// - class_nameTable (done)
// - class_objTable (done)
//   - X_protObj (done)
//     - X_dispTable (done)
//       - X_methods (partially done, implement body)
//     - attributes (done)
//   - X_init (partially done, implement body)
//
// External things:
// - implement external classes
// - implement equality
// - abort for dispatch on void
// - abort for case on void
// - abort for case on no match
// - exception handling
//
// Future plans:
// - extend external classes (e.g add graphics to IO)

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

    {
        // read prelude file
        // TODO: take this file from ENV/PATH
        length = util_read_file("lib/prelude.cl", &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: lib/stdlib.cl");
            return_defer(1);
        }

        // tokenize prelude
        ds_dynamic_array_init(&tokens, sizeof(struct token));
        if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
            DS_LOG_ERROR("Failed to tokenize input");
            return_defer(1);
        }

        // parse tokens
        program_node program;
        if (parser_run(basename, &tokens, &program) != PARSER_OK) {
            printf("Compilation halted\n");
            return_defer(1);
        }
        ds_dynamic_array_append(&programs, &program);
    }

    size_t index = programs.count;

    // front-end
    {
        // read input file
        length = util_read_file(filename, &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: %s", filename);
            return_defer(1);
        }

        // tokenize input
        ds_dynamic_array_init(&tokens, sizeof(struct token));
        if (lexer_tokenize(buffer, length, &tokens) != LEXER_OK) {
            DS_LOG_ERROR("Failed to tokenize input");
            return_defer(1);
        }

        if (argparse_get_flag(parser, ARG_LEXER) == 1) {
            lexer_print_tokens(&tokens);
            return_defer(0);
        }

        // parse tokens
        program_node program;
        if (parser_run(basename, &tokens, &program) != PARSER_OK) {
            printf("Compilation halted\n");
            return_defer(1);
        }
        ds_dynamic_array_append(&programs, &program);

        if (argparse_get_flag(parser, ARG_SYNTAX) == 1) {
            parser_print_ast(&program);
            return_defer(0);
        }
    }

    // merge programs
    program_node program;
    parser_merge(programs, &program);

    // gatekeeping
    {
        // semantic check
        if (semantic_check(basename, &program, &mapping) != SEMANTIC_OK) {
            printf("Compilation halted\n");
            return_defer(1);
        }

        if (argparse_get_flag(parser, ARG_SEMANTIC) == 1) {
            program_node *program = NULL;
            ds_dynamic_array_get_ref(&programs, index, (void **)&program);

            parser_print_ast(program);
            return_defer(0);
        }

        if (argparse_get_flag(parser, ARG_MAPPING) == 1) {
            semantic_print_mapping(&mapping);
            return_defer(0);
        }
    }

    // codegen
    {
        if (argparse_get_flag(parser, ARG_TACGEN) == 1) {
            codegen_tac_print(&program);
            return_defer(0);
        }

        // read asm prelude file
        // TODO: take this file from ENV/PATH
        length = util_read_file("lib/prelude.asm", &buffer);
        if (length < 0) {
            DS_LOG_ERROR("Failed to read file: lib/stdlib.cl");
            return_defer(1);
        }

        if (util_write_file(output, buffer) != 0) {
            DS_LOG_ERROR("Failed to write file: %s", output);
            return_defer(1);
        }

        // assembler
        if (assembler_run(output, &program, &mapping) != ASSEMBLER_OK) {
            printf("Compilation halted\n");
            return_defer(1);
        }

        if (argparse_get_flag(parser, ARG_ASSEMBLER) == 1) {
            return_defer(0);
        }
    }

defer:
    for (unsigned int i = 0; i < tokens.count; i++) {
        struct token tok;
        ds_dynamic_array_get(&tokens, i, &tok);
        if (tok.literal)
            DS_FREE(NULL, tok.literal);
    }
    ds_dynamic_array_free(&tokens);
    if (buffer != NULL)
        DS_FREE(NULL, buffer);
    argparse_free(parser);
    return result;
}
