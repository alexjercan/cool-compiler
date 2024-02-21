#include <stdio.h>
#define ARGPARSE_IMPLEMENTATION
#include "argparse.h"
#include "ds.h"
#include "lexer.h"

#define LINE_MAX 4096

int read_file(const char *filename, char **buffer) {
    int result = 0;

    FILE *file = NULL;
    if (filename != NULL) {
        file = fopen(filename, "r");
        if (file == NULL) {
            DS_LOG_ERROR("Failed to open file: %s", filename);
            return_defer(-1);
        }
    } else {
        file = stdin;
    }

    ds_string_builder sb = {.items = NULL, .count = 0, .capacity = 0};
    char line[LINE_MAX] = {0};
    while (fgets(line, sizeof(line), file) != NULL) {
        unsigned int len = 0;
        for (len = 0; len < LINE_MAX; len++) {
            if (line[len] == '\n' || line[len] == EOF) {
                break;
            }
        }

        if (len == LINE_MAX) {
            len = strlen(line);
        } else {
            len += 1;
            line[len] = '\0';
        }

        if (len == LINE_MAX) {
            DS_LOG_ERROR("Line too long");
            return_defer(-1);
        }

        if (ds_string_builder_appendn(&sb, line, len) != 0) {
            DS_LOG_ERROR("Failed to append line to string builder");
            return_defer(-1);
        }

        memset(line, 0, sizeof(line));
    }

    if (ds_string_builder_build(&sb, buffer) != 0) {
        DS_LOG_ERROR("Failed to build string from string builder");
        return_defer(-1);
    }

    result = sb.count;

defer:
    if (filename != NULL && file != NULL)
        fclose(file);
    ds_string_builder_free(&sb);
    return result;
}

int main(int argc, char **argv) {
    int result = 0;

    struct argparse_parser *parser =
        argparse_new("coolc", "A cool compiler", "0.1.0");

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'l',
                                           .long_name = "lex",
                                           .description = "Lex the input file",
                                           .type = ARGUMENT_TYPE_FLAG,
                                           .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'i',
                                           .long_name = "input",
                                           .description = "Input file",
                                           .type = ARGUMENT_TYPE_POSITIONAL,
                                           .required = 0}));

    argparse_add_argument(
        parser, ((struct argparse_options){.short_name = 'o',
                                           .long_name = "output",
                                           .description = "Output file",
                                           .type = ARGUMENT_TYPE_VALUE,
                                           .required = 0}));

    argparse_parse(parser, argc, argv);

    char *input = argparse_get_value(parser, "input");
    char *buffer = NULL;
    int length = read_file(input, &buffer);
    if (length < 0) {
        DS_LOG_ERROR("Failed to read file: %s", input);
        return_defer(1);
    }

    ds_dynamic_array tokens;
    ds_dynamic_array_init(&tokens, sizeof(struct token));

    if(run_lexer(buffer, length, input, &tokens) != 0) {
        printf("Compilation halted\n");
        return_defer(1);
    }

    unsigned int lex = argparse_get_flag(parser, "lex");
    if (lex == 1) {
        for (unsigned int i = 0; i < tokens.count; i++) {
            struct token tok;
            ds_dynamic_array_get(&tokens, i, &tok);

            printf("%s", token_type_to_string(tok.type));
            if (tok.literal != NULL) {
                printf("(%s)", tok.literal);
            }
            printf("\n");
        }
    }

defer:
    argparse_free(parser);
    if (buffer != NULL)
        free(buffer);
    return result;
}
