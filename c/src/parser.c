#include "parser.h"
#include "ds.h"
#include "lexer.h"

struct parser {
        const char *filename;
        ds_dynamic_array *tokens;
        unsigned int index;
        int result;
};

static int parser_current(struct parser *parser, struct token *token) {
    if (parser->index >= parser->tokens->count) {
        return 1;
    }

    ds_dynamic_array_get(parser->tokens, parser->index, token);

    return 0;
}

static void parser_show_error(struct parser *parser) {
    parser->result = PARSER_ERROR;

    struct token token;
    parser_current(parser, &token);

    if (token.type == ILLEGAL) {
        if (parser->filename == NULL) {
            if (token.literal != NULL) {
                printf("line %d:%d, Lexical error: %s: %s\n", token.line,
                       token.col, error_type_to_string(token.error),
                       token.literal);
            } else {
                printf("line %d:%d, Lexical error: %s\n", token.line, token.col,
                       error_type_to_string(token.error));
            }
        } else {
            if (token.literal != NULL) {
                printf("\"%s\", %d:%d: Lexical error: %s: %s\n",
                       parser->filename, token.line, token.col,
                       error_type_to_string(token.error), token.literal);
            } else {
                printf("\"%s\", %d:%d: Lexical error: %s\n", parser->filename,
                       token.line, token.col,
                       error_type_to_string(token.error));
            }
        }
    } else {
        if (parser->filename == NULL) {
            if (token.literal != NULL) {
                printf("line %d:%d, Syntax error: Unexpected %s: %s\n",
                       token.line, token.col, token_type_to_string(token.type),
                       token.literal);
            } else {
                printf("line %d:%d, Syntax error: Unexpected %s\n", token.line,
                       token.col, token_type_to_string(token.type));
            }
        } else {
            if (token.literal != NULL) {
                printf("\"%s\", %d:%d: Syntax error: Unexpected %s: %s\n",
                       parser->filename, token.line, token.col,
                       token_type_to_string(token.type), token.literal);
            } else {
                printf("\"%s\", %d:%d: Syntax error: Unexpected %s\n",
                       parser->filename, token.line, token.col,
                       token_type_to_string(token.type));
            }
        }
    }
}

static int parser_advance(struct parser *parser) {
    if (parser->index >= parser->tokens->count) {
        return 1;
    }

    parser->index++;

    return 0;
}

static void parser_recovery(struct parser *parser) {
    struct token token;
    parser_current(parser, &token);

    while (token.type != END) {
        parser_advance(parser);

        if (token.type == SEMICOLON) {
            break;
        }

        parser_current(parser, &token);
    }
}

static int parser_attribute(struct parser *parser, attribute_node *attribute) {
    int result = 0;
    struct token token;

    parser_current(parser, &token);
    if (token.type != IDENT) {
        return_defer(1);
    }
    parser_advance(parser);

    attribute->name.value = token.literal;
    attribute->name.line = token.line;
    attribute->name.col = token.col;

    parser_current(parser, &token);
    if (token.type != COLON) {
        return_defer(1);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != CLASS_NAME) {
        return_defer(1);
    }
    parser_advance(parser);

    attribute->type.value = token.literal;
    attribute->type.line = token.line;
    attribute->type.col = token.col;

    // Handle initial value optional

    parser_current(parser, &token);
    if (token.type != SEMICOLON) {
        return_defer(1);
    }
    parser_advance(parser);

defer:
    if (result != 0) {
        parser_show_error(parser);
        parser_recovery(parser);
    }

    return result;
}

static int parser_class(struct parser *parser, class_node *class) {
    int result = 0;
    struct token token;

    parser_current(parser, &token);
    if (token.type != CLASS) {
        return_defer(1);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != CLASS_NAME) {
        return_defer(1);
    }
    parser_advance(parser);

    class->name.value = token.literal;
    class->name.line = token.line;
    class->name.col = token.col;
    class->superclass.value = NULL;
    ds_dynamic_array_init(&class->attributes, sizeof(attribute_node));

    parser_current(parser, &token);
    if (token.type == INHERITS) {
        parser_advance(parser);

        parser_current(parser, &token);
        if (token.type != CLASS_NAME) {
            return_defer(1);
        }

        class->superclass.value = token.literal;
        class->superclass.line = token.line;
        class->superclass.col = token.col;

        parser_advance(parser);
    }

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        return_defer(1);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    while (token.type != RBRACE) {
        attribute_node attribute;
        parser_attribute(parser, &attribute);

        ds_dynamic_array_append(&class->attributes, &attribute);

        parser_current(parser, &token);
        if (token.type == END) {
            return_defer(1);
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != SEMICOLON) {
        return_defer(1);
    }
    parser_advance(parser);

defer:
    if (result != 0) {
        parser_show_error(parser);
        parser_recovery(parser);
    }
    return result;
}

static int parser_program(struct parser *parser, program_node *program) {
    int result = 0;

    struct token token;
    do {
        class_node class;
        result = parser_class(parser, &class);

        if (result != 0) {
            parser_recovery(parser);
        }

        ds_dynamic_array_append(&program->classes, &class);

        parser_current(parser, &token);
    } while (token.type != END);

    return result;
}

int parser_run(const char *filename, ds_dynamic_array *tokens,
               program_node *program) {
    ds_dynamic_array_init(&program->classes, sizeof(class_node));

    struct parser parser = {.filename = filename,
                            .tokens = tokens,
                            .index = 0,
                            .result = PARSER_OK};

    parser_program(&parser, program);

    return parser.result;
}
