#include "parser.h"
#include "ds.h"
#include "lexer.h"
#include <stdarg.h>

static void node_info_init(node_info *info) { info->value = NULL; }

static void expr_node_init(expr_node *expr) { expr->type = EXPR_NONE; }

static void attribute_node_init(attribute_node *attribute) {
    node_info_init(&attribute->name);
    node_info_init(&attribute->type);
    expr_node_init(&attribute->value);
}

static void formal_node_init(formal_node *formal) {
    node_info_init(&formal->name);
    node_info_init(&formal->type);
}

static void method_node_init(method_node *method) {
    node_info_init(&method->name);
    node_info_init(&method->type);
    ds_dynamic_array_init(&method->formals, sizeof(formal_node));
    expr_node_init(&method->body);
}

static void class_node_init(class_node *class) {
    node_info_init(&class->name);
    node_info_init(&class->superclass);
    ds_dynamic_array_init(&class->attributes, sizeof(attribute_node));
    ds_dynamic_array_init(&class->methods, sizeof(method_node));
}

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

static int parser_advance(struct parser *parser) {
    if (parser->index >= parser->tokens->count) {
        return 1;
    }

    parser->index++;

    return 0;
}

static void parser_show_errorf(struct parser *parser, const char *format, ...) {
    parser->result = PARSER_ERROR;

    struct token token;
    parser_current(parser, &token);

    const char *filename = parser->filename;

    if (token.type == ILLEGAL) {
        if (filename != NULL) {
            printf("\"%s\", ", filename);
        }

        printf("line %d:%d, Lexical error: %s", token.line, token.col,
               error_type_to_string(token.error));

        if (token.literal != NULL) {
            printf(": %s", token.literal);
        }

        printf("\n");
    } else {
        if (filename != NULL) {
            printf("\"%s\", ", filename);
        }
        printf("line %d:%d, Syntax error: ", token.line, token.col);

        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);

        printf("\n");
    }
}

#define parser_show_expected(parser, expected, got)                            \
    parser_show_errorf(parser, "expected %s, got %s",                          \
                       token_type_to_string(expected),                         \
                       token_type_to_string(got))

#define parser_show_extected_2(parser, expected1, expected2, got)              \
    parser_show_errorf(                                                        \
        parser, "expected %s or %s, got %s", token_type_to_string(expected1),  \
        token_type_to_string(expected2), token_type_to_string(got))

static void build_expr(struct parser *parser, expr_node *expr) {
    struct token token;

    parser_current(parser, &token);
    switch (token.type) {
    case INT_LITERAL: {
        expr->type = EXPR_INT;
        expr->integer.value = token.literal;
        expr->integer.line = token.line;
        expr->integer.col = token.col;
        parser_advance(parser);
        break;
    }
    case STRING_LITERAL: {
        expr->type = EXPR_STRING;
        expr->string.value = token.literal;
        expr->string.line = token.line;
        expr->string.col = token.col;
        parser_advance(parser);
        break;
    }
    case BOOL_LITERAL: {
        expr->type = EXPR_BOOL;
        expr->boolean.value = token.literal;
        expr->boolean.line = token.line;
        expr->boolean.col = token.col;
        parser_advance(parser);
        break;
    }
    case IDENT: {
        expr->type = EXPR_IDENT;
        expr->ident.value = token.literal;
        expr->ident.line = token.line;
        expr->ident.col = token.col;
        parser_advance(parser);
        break;
    }
    default:
        return;
    }
}

static void build_attribute(struct parser *parser, attribute_node *attribute) {
    struct token token;

    parser_current(parser, &token);
    if (token.type == IDENT) {
        attribute->name.value = token.literal;
        attribute->name.line = token.line;
        attribute->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        attribute->type.value = token.literal;
        attribute->type.line = token.line;
        attribute->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == ASSIGN) {
        parser_advance(parser);

        build_expr(parser, &attribute->value);
    }
}

static void build_formal(struct parser *parser, formal_node *formal) {
    struct token token;

    parser_current(parser, &token);
    if (token.type == IDENT) {
        formal->name.value = token.literal;
        formal->name.line = token.line;
        formal->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        if (token.type == END || token.type == COMMA || token.type == RPAREN) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        if (token.type == END || token.type == COMMA || token.type == RPAREN) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        formal->type.value = token.literal;
        formal->type.line = token.line;
        formal->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END || token.type == COMMA || token.type == RPAREN) {
            return;
        }
    }
    parser_advance(parser);
}

static void build_method(struct parser *parser, method_node *method) {
    struct token token;

    parser_current(parser, &token);
    if (token.type == IDENT) {
        method->name.value = token.literal;
        method->name.line = token.line;
        method->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != LPAREN) {
        parser_show_expected(parser, LPAREN, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == IDENT) {
        struct formal_node formal;
        formal_node_init(&formal);

        build_formal(parser, &formal);

        ds_dynamic_array_append(&method->formals, &formal);

        parser_current(parser, &token);
    }

    while (token.type != RPAREN) {
        struct formal_node formal;
        formal_node_init(&formal);

        if (token.type != COMMA) {
            parser_show_extected_2(parser, COMMA, RPAREN, token.type);
            if (token.type == END) {
                return;
            }
        }
        parser_advance(parser);

        build_formal(parser, &formal);

        ds_dynamic_array_append(&method->formals, &formal);

        parser_current(parser, &token);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        method->type.value = token.literal;
        method->type.line = token.line;
        method->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        parser_show_expected(parser, LBRACE, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, &method->body);

    parser_current(parser, &token);
    if (token.type != RBRACE) {
        parser_show_expected(parser, RBRACE, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);
}

static void build_feature(struct parser *parser, class_node *class) {
    struct token token;

    parser_current(parser, &token);
    if (token.type != IDENT) {
        parser_show_expected(parser, IDENT, token.type);
        return;
    }

    parser_peek(parser, &token);
    if (token.type == COLON) {
        attribute_node attribute;
        attribute_node_init(&attribute);

        build_attribute(parser, &attribute);

        ds_dynamic_array_append(&class->attributes, &attribute);
    } else {
        method_node method;
        method_node_init(&method);

        build_method(parser, &method);

        ds_dynamic_array_append(&class->methods, &method);
    }
}

static void build_class(struct parser *parser, class_node *class) {
    struct token token;

    parser_current(parser, &token);
    if (token.type != CLASS) {
        parser_show_expected(parser, CLASS, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        class->name.value = token.literal;
        class->name.line = token.line;
        class->name.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == INHERITS) {
        parser_advance(parser);

        parser_current(parser, &token);
        if (token.type == CLASS_NAME) {
            class->superclass.value = token.literal;
            class->superclass.line = token.line;
            class->superclass.col = token.col;
        } else {
            parser_show_expected(parser, CLASS_NAME, token.type);
            if (token.type == END) {
                return;
            }
        }

        parser_advance(parser);
    }

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        parser_show_expected(parser, LBRACE, token.type);
        if (token.type == END || token.type == SEMICOLON) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    while (token.type != RBRACE) {
        if (token.type == END) {
            parser_show_expected(parser, RBRACE, token.type);
            return;
        }

        build_feature(parser, class);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            parser_show_expected(parser, SEMICOLON, token.type);
            if (token.type == END || token.type == RBRACE) {
                return;
            }
        }
        parser_advance(parser);

        parser_current(parser, &token);
    }
    parser_advance(parser);
}

static void build_program(struct parser *parser, program_node *program) {
    struct token token;
    do {
        class_node class;
        class_node_init(&class);

        build_class(parser, &class);

        ds_dynamic_array_append(&program->classes, &class);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            parser_show_expected(parser, SEMICOLON, token.type);
            if (token.type == END) {
                return;
            }
        }
        parser_advance(parser);

        parser_current(parser, &token);
    } while (token.type != END);
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
