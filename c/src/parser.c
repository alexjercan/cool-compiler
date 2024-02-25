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

static int parser_peek(struct parser *parser, struct token *token) {
    if (parser->index + 1 >= parser->tokens->count) {
        return 1;
    }

    ds_dynamic_array_get(parser->tokens, parser->index + 1, token);

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

static void parser_recovery_formal(struct parser *parser) {
    struct token token;
    parser_current(parser, &token);

    while (token.type != END) {
        if (token.type == RPAREN) {
            return;
        }

        parser_peek(parser, &token);

        if (token.type == COMMA) {
            return;
        }

        parser_advance(parser);
        parser_current(parser, &token);
    }
}

static void parser_recovery_feature(struct parser *parser) {
    struct token token;
    parser_current(parser, &token);

    while (token.type != END) {
        if (token.type == SEMICOLON) {
            return;
        }

        parser_advance(parser);
        parser_current(parser, &token);
    }
}

static void parser_recovery_class(struct parser *parser) {
    struct token token;
    parser_current(parser, &token);

    while (token.type != END) {
        if (token.type == SEMICOLON) {
            return;
        }

        parser_advance(parser);
        parser_current(parser, &token);
    }
}

static int build_attribute(struct parser *parser, attribute_node *attribute) {
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

    parser_current(parser, &token);
    if (token.type == ASSIGN) {
        parser_advance(parser);

        // TODO: fix this
        parser_current(parser, &token);
        if (token.type != INT_LITERAL) {
            return_defer(1);
        }
        parser_advance(parser);

        attribute->value.value = token.literal;
        attribute->value.line = token.line;
        attribute->value.col = token.col;
    }

defer:
    return result;
}

static int build_formal(struct parser *parser, formal_node *formal) {
    int result = 0;
    struct token token;

    parser_current(parser, &token);
    if (token.type != IDENT) {
        return_defer(1);
    }
    parser_advance(parser);

    formal->name.value = token.literal;
    formal->name.line = token.line;
    formal->name.col = token.col;

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

    formal->type.value = token.literal;
    formal->type.line = token.line;
    formal->type.col = token.col;

defer:
    return result;
}

static int build_method(struct parser *parser, method_node *method) {
    int result = 0;
    struct token token;

    parser_current(parser, &token);
    if (token.type != IDENT) {
        return_defer(1);
    }
    parser_advance(parser);

    method->name.value = token.literal;
    method->name.line = token.line;
    method->name.col = token.col;

    parser_current(parser, &token);
    if (token.type != LPAREN) {
        return_defer(1);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == IDENT) {
        struct formal_node formal;
        formal.name.value = NULL;
        formal.type.value = NULL;

        if (build_formal(parser, &formal) != 0) {
            parser_show_error(parser);
            parser_recovery_formal(parser);
        }

        ds_dynamic_array_append(&method->formals, &formal);

        parser_current(parser, &token);
    }

    while (token.type != RPAREN) {
        if (token.type == END) {
            return_defer(1);
        }

        struct formal_node formal;
        formal.name.value = NULL;
        formal.type.value = NULL;

        if (token.type != COMMA) {
            return_defer(1);
        }
        parser_advance(parser);

        if (build_formal(parser, &formal) != 0) {
            parser_show_error(parser);
            parser_recovery_formal(parser);
        }

        ds_dynamic_array_append(&method->formals, &formal);

        parser_current(parser, &token);
    }
    parser_advance(parser);

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

    method->type.value = token.literal;
    method->type.line = token.line;
    method->type.col = token.col;

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        return_defer(1);
    }
    parser_advance(parser);

    // TODO: fix this
    parser_current(parser, &token);
    if (token.type != INT_LITERAL) {
        return_defer(1);
    }
    parser_advance(parser);

    method->body.value = token.literal;
    method->body.line = token.line;
    method->body.col = token.col;

    parser_current(parser, &token);
    if (token.type != RBRACE) {
        return_defer(1);
    }
    parser_advance(parser);

defer:
    return result;
}

static int build_feature(struct parser *parser, class_node *class) {
    int result = 0;
    struct token token;

    parser_peek(parser, &token);
    if (token.type == COLON) {
        attribute_node attribute;
        attribute.name.value = NULL;
        attribute.type.value = NULL;
        attribute.value.value = NULL;

        result = build_attribute(parser, &attribute);

        ds_dynamic_array_append(&class->attributes, &attribute);
    } else {
        method_node method;
        method.name.value = NULL;
        method.type.value = NULL;
        ds_dynamic_array_init(&method.formals, sizeof(formal_node));
        method.body.value = NULL;

        result = build_method(parser, &method);

        ds_dynamic_array_append(&class->methods, &method);
    }

    return result;
}

static int build_class(struct parser *parser, class_node *class) {
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
    ds_dynamic_array_init(&class->attributes, sizeof(attribute_node));
    ds_dynamic_array_init(&class->methods, sizeof(method_node));

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
        if (token.type == END) {
            return_defer(1);
        }

        if (build_feature(parser, class) != 0) {
            parser_show_error(parser);
            parser_recovery_feature(parser);
        }

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            return_defer(1);
        }
        parser_advance(parser);

        parser_current(parser, &token);
    }
    parser_advance(parser);

defer:
    return result;
}

static int build_program(struct parser *parser, program_node *program) {
    int result = 0;

    struct token token;
    do {
        class_node class;
        class.name.value = NULL;
        class.superclass.value = NULL;
        ds_dynamic_array_init(&class.attributes, sizeof(attribute_node));
        ds_dynamic_array_init(&class.methods, sizeof(method_node));

        if (build_class(parser, &class) != 0) {
            parser_show_error(parser);
            parser_recovery_class(parser);
        }

        ds_dynamic_array_append(&program->classes, &class);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            return_defer(1);
        }
        parser_advance(parser);

        parser_current(parser, &token);
    } while (token.type != END);

defer:
    return result;
}

int parser_run(const char *filename, ds_dynamic_array *tokens,
               program_node *program) {
    ds_dynamic_array_init(&program->classes, sizeof(class_node));

    struct parser parser = {.filename = filename,
                            .tokens = tokens,
                            .index = 0,
                            .result = PARSER_OK};

    build_program(&parser, program);

    return parser.result;
}
