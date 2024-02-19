#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#define DS_SB_IMPLEMENTATION
#include "ds.h"
#include "lexer.h"

#define LINE_MAX 2048

int read_file(const char *filename, char **buffer) {
    int result = 0;

    FILE *file = NULL;
    if (filename != NULL) {
        file = fopen(filename, "r");
        if (file == NULL) {
            DS_LOG_ERROR("Failed to open file: %s", filename);
            return_defer(1);
        }
    } else {
        file = stdin;
    }

    ds_string_builder sb = { .items = NULL, .count = 0, .capacity = 0 };
    char line[LINE_MAX];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (ds_string_builder_append(&sb, line) != 0) {
            DS_LOG_ERROR("Failed to append line to string builder");
            return_defer(1);
        }
    }

    if (ds_string_builder_build(&sb, buffer) != 0) {
        DS_LOG_ERROR("Failed to build string from string builder");
        return_defer(1);
    }

defer:
    if (filename != NULL && file != NULL)
        fclose(file);
    ds_string_builder_free(&sb);
    return result;
}

int main(int argc, char **argv) {
    int result = 0;

    struct argparse_parser *parser = argparse_new("coolc", "A cool compiler", "0.1.0");

    argparse_add_argument(parser, ((struct argparse_options) {
        .short_name = 'l',
        .long_name = "lex",
        .description = "Lex the input file",
        .type = ARGUMENT_TYPE_FLAG,
        .required = 0
    }));

    argparse_add_argument(parser, ((struct argparse_options) {
        .short_name = 'i',
        .long_name = "input",
        .description = "Input file",
        .type = ARGUMENT_TYPE_POSITIONAL,
        .required = 1
    }));

    argparse_parse(parser, argc, argv);

    char *input = argparse_get_value(parser, "input");
    char *buffer = NULL;
    if (read_file(input, &buffer) != 0) {
        DS_LOG_ERROR("Failed to read file: %s", input);
        return_defer(1);
    }

    unsigned int lex = argparse_get_flag(parser, "lex");
    if (lex == 1) {
        run_lexer(buffer);
    }

defer:
    argparse_free(parser);
    if (buffer != NULL) free(buffer);
    return result;
}
