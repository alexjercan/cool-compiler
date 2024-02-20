#ifndef ARGPARSE_H
#define ARGPARSE_H

#define ARG_TERMINAL_RED "\033[1;31m"
#define ARG_TERMINAL_YLW "\033[1;33m"
#define ARG_TERMINAL_RST "\033[0m"

#define ARG_LOG_WARN(format, ...)                                              \
    fprintf(stdout,                                                            \
            ARG_TERMINAL_YLW "WARN" ARG_TERMINAL_RST ": %s:%d: " format "\n",  \
            __FILE__, __LINE__, ##__VA_ARGS__)

#define ARG_LOG_ERROR(format, ...)                                             \
    fprintf(stderr,                                                            \
            ARG_TERMINAL_RED "ERROR" ARG_TERMINAL_RST ": %s:%d: " format "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)

#define ARG_PANIC(format, ...)                                                 \
    do {                                                                       \
        ARG_LOG_ERROR(format, ##__VA_ARGS__);                                  \
        exit(1);                                                               \
    } while (0)

#define ARG_INIT_CAPACITY 8192
#define ARG_DA_REALLOC(ptr, old_sz, new_sz) realloc(ptr, new_sz)
#define arg_da_append(da, item)                                                \
    do {                                                                       \
        if ((da)->count >= (da)->capacity) {                                   \
            unsigned int new_capacity = (da)->capacity * 2;                    \
            if (new_capacity == 0) {                                           \
                new_capacity = ARG_INIT_CAPACITY;                              \
            }                                                                  \
                                                                               \
            (da)->items = ARG_DA_REALLOC(                                      \
                (da)->items, (da)->capacity * sizeof(*(da)->items),            \
                new_capacity * sizeof(*(da)->items));                          \
            if ((da)->items == NULL) {                                         \
                ARG_PANIC("Failed to reallocate dynamic array");               \
            }                                                                  \
                                                                               \
            (da)->capacity = new_capacity;                                     \
        }                                                                      \
                                                                               \
        (da)->items[(da)->count++] = (item);                                   \
    } while (0)

// Argument types
enum argument_type {
    ARGUMENT_TYPE_VALUE,      // Argument with a value
    ARGUMENT_TYPE_FLAG,       // Flag argument
    ARGUMENT_TYPE_POSITIONAL, // Positional argument
};

// Argument options
struct argparse_options {
        char short_name;         // Short name of the argument
        char *long_name;         // Long name of the argument
        char *description;       // Description of the argument
        enum argument_type type; // Type of the argument
        unsigned int required;   // Whether the argument is required
};

// Argument parser
struct argparse_parser;

struct argparse_parser *argparse_new(char *name, char *description,
                                     char *version);
void argparse_add_argument(struct argparse_parser *parser,
                           struct argparse_options options);
void argparse_parse(struct argparse_parser *parser, int argc, char *argv[]);
char *argparse_get_value(struct argparse_parser *parser, char *long_name);
unsigned int argparse_get_flag(struct argparse_parser *parser, char *long_name);
char *argparse_get_positional(struct argparse_parser *parser, char *long_name);

void argparse_print_help(struct argparse_parser *parser);
void argparse_print_version(struct argparse_parser *parser);

void argparse_free(struct argparse_parser *parser);

#endif // ARGPARSE_H

#ifdef ARGPARSE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct argument {
        struct argparse_options options;
        char *value;
        unsigned int flag;
};

struct argparse_parser {
        char *name;
        char *description;
        char *version;
        struct argument *items;
        size_t count;
        size_t capacity;
};

// Create a new argument parser
//
// Allocates memory for a new argument parser and initializes it with the given
// name, description, and version. It also adds the default help and version
// arguments.
//
// Arguments:
// - name: name of the program
// - description: description of the program
// - version: version of the program
//
// Returns:
// - a new argument parser
struct argparse_parser *argparse_new(char *name, char *description,
                                     char *version) {
    struct argparse_parser *parser = malloc(sizeof(struct argparse_parser));

    parser->name = name;
    parser->description = description;
    parser->version = version;
    parser->items = NULL;
    parser->count = 0;
    parser->capacity = 0;

    argparse_add_argument(
        parser,
        (struct argparse_options){.short_name = 'v',
                                  .long_name = "version",
                                  .description = "print the program version",
                                  .type = ARGUMENT_TYPE_FLAG,
                                  .required = 0});
    argparse_add_argument(parser, (struct argparse_options){
                                      .short_name = 'h',
                                      .long_name = "help",
                                      .description = "print this help message",
                                      .type = ARGUMENT_TYPE_FLAG,
                                      .required = 0});

    return parser;
}

// Add an argument to the parser
//
// Adds a new argument to the parser with the given options.
//
// Arguments:
// - parser: argument parser
// - options: argument options
void argparse_add_argument(struct argparse_parser *parser,
                           struct argparse_options options) {
    if (options.short_name == '\0' && options.long_name == NULL) {
        ARG_PANIC("short_name or long_name must be set");
    }
    if (options.type == ARGUMENT_TYPE_FLAG && options.required == 1) {
        ARG_LOG_WARN("flag argument cannot be required");
    }
    if (options.type == ARGUMENT_TYPE_POSITIONAL && options.required == 0) {
        ARG_LOG_WARN("positional argument must be required");
    }

    struct argument arg = {
        .options = options,
        .value = NULL,
        .flag = 0,
    };

    arg_da_append(parser, arg);
}

// Parse the command line arguments
//
// Parses the command line arguments and sets the values of the arguments in the
// parser.
//
// Arguments:
// - parser: argument parser
// - argc: number of command line arguments
// - argv: command line arguments
void argparse_parse(struct argparse_parser *parser, int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        char *name = argv[i];

        if (strcmp(name, "-h") == 0 || strcmp(name, "--help") == 0) {
            argparse_print_help(parser);
            exit(0);
        }

        if (strcmp(name, "-v") == 0 || strcmp(name, "--version") == 0) {
            argparse_print_version(parser);
            exit(0);
        }

        if (name[0] == '-') {
            struct argument *arg = NULL;

            for (size_t j = 0; j < parser->count; j++) {
                struct argument *item = &parser->items[j];

                if ((name[1] == '-' && item->options.long_name != NULL &&
                     strcmp(name + 2, item->options.long_name) == 0) ||
                    (name[1] != '-' && item->options.short_name != '\0' &&
                     name[1] == item->options.short_name)) {
                    arg = item;
                    break;
                }
            }

            if (arg == NULL) {
                ARG_LOG_ERROR("invalid argument: %s", name);
                argparse_print_help(parser);
                exit(1);
            }

            switch (arg->options.type) {
            case ARGUMENT_TYPE_FLAG: {
                arg->flag = 1;
                break;
            }
            case ARGUMENT_TYPE_VALUE: {
                if (i + 1 >= argc) {
                    ARG_LOG_ERROR("missing value for argument: %s", name);
                    argparse_print_help(parser);
                    exit(1);
                }

                arg->value = argv[++i];
                break;
            }
            default: {
                ARG_LOG_ERROR("type not supported for argument: %s", name);
                argparse_print_help(parser);
                exit(1);
            }
            }
        } else {
            struct argument *arg = NULL;

            for (size_t j = 0; j < parser->count; j++) {
                struct argument *item = &parser->items[j];

                if (item->options.type == ARGUMENT_TYPE_POSITIONAL &&
                    item->value == NULL) {
                    arg = item;
                    break;
                }
            }

            if (arg == NULL) {
                ARG_LOG_ERROR("unexpected positional argument: %s", name);
                argparse_print_help(parser);
                exit(1);
            }

            arg->value = name;
        }
    }

    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->options.type == ARGUMENT_TYPE_POSITIONAL &&
            item->value == NULL) {
            ARG_LOG_ERROR("missing value for positional argument: %s",
                          item->options.long_name);
            argparse_print_help(parser);
            exit(1);
        }

        if (item->options.type == ARGUMENT_TYPE_VALUE &&
            item->options.required == 1 && item->value == NULL) {
            ARG_LOG_ERROR("missing required argument: --%s",
                          item->options.long_name);
            argparse_print_help(parser);
            exit(1);
        }
    }
}

// Get the value of an argument
//
// Returns the value of the argument with the given long name.
//
// Arguments:
// - parser: argument parser
// - long_name: long name of the argument
//
// Returns:
// - value of the argument
char *argparse_get_value(struct argparse_parser *parser, char *long_name) {
    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->options.long_name != NULL &&
            strcmp(long_name, item->options.long_name) == 0) {
            if (item->options.type != ARGUMENT_TYPE_VALUE &&
                item->options.type != ARGUMENT_TYPE_POSITIONAL) {
                ARG_LOG_WARN("argument is not a value: %s", long_name);
            }
            return item->value;
        }
    }

    return NULL;
}

// Get the value of a positional argument
//
// Returns the value of the positional argument with the given long name.
//
// Arguments:
// - parser: argument parser
// - long_name: long name of the argument
//
// Returns:
// - value of the flag argument
unsigned int argparse_get_flag(struct argparse_parser *parser,
                               char *long_name) {
    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->options.long_name != NULL &&
            strcmp(long_name, item->options.long_name) == 0) {
            if (item->options.type != ARGUMENT_TYPE_FLAG) {
                ARG_LOG_WARN("argument is not a flag: %s", long_name);
            }
            return item->flag;
        }
    }

    return 0;
}

// Show the help message
//
// Prints the help message for the argument parser.
//
// Arguments:
// - parser: argument parser
void argparse_print_help(struct argparse_parser *parser) {
    printf("usage: %s [options]", parser->name);
    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->options.type == ARGUMENT_TYPE_VALUE &&
            item->options.required == 1) {
            printf(" -%c <%s>", item->options.short_name,
                   item->options.long_name);
        }
    }
    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        if (item->options.type == ARGUMENT_TYPE_POSITIONAL) {
            printf(" <%s>", item->options.long_name);
        }
    }
    printf("\n");
    printf("%s\n", parser->description);
    printf("\n");
    printf("options:\n");

    for (size_t i = 0; i < parser->count; i++) {
        struct argument *item = &parser->items[i];

        switch (item->options.type) {
        case ARGUMENT_TYPE_POSITIONAL: {
            printf("  %c, %s\n", item->options.short_name,
                   item->options.long_name);
            printf("      %s\n", item->options.description);
            printf("\n");
            break;
        }
        case ARGUMENT_TYPE_FLAG: {
            printf("  -%c, --%s\n", item->options.short_name,
                   item->options.long_name);
            printf("      %s\n", item->options.description);
            printf("\n");
            break;
        }
        case ARGUMENT_TYPE_VALUE: {
            printf("  -%c, --%s <value>\n", item->options.short_name,
                   item->options.long_name);
            printf("      %s\n", item->options.description);
            printf("\n");
            break;
        }
        default: {
            ARG_PANIC("invalid argument type");
        }
        }
    }
}

// Show the version
//
// Prints the version of the program.
//
// Arguments:
// - parser: argument parser
void argparse_print_version(struct argparse_parser *parser) {
    printf("%s %s\n", parser->name, parser->version);
}

// Free the argument parser
//
// Frees the memory allocated for the argument parser.
//
// Arguments:
// - parser: argument parser
void argparse_free(struct argparse_parser *parser) {
    free(parser->items);
    free(parser);
}

#endif // ARGPARSE_IMPLEMENTATION
