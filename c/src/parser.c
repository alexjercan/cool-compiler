#include "parser.h"
#include "ds.h"
#include "lexer.h"

struct parser {
        const char *filename;
        ds_dynamic_array *tokens;
        unsigned int index;
};

static int parser_peek(struct parser *parser, struct token *token) {
    if (parser->index >= parser->tokens->count) {
        return 1;
    }

    ds_dynamic_array_get(parser->tokens, parser->index, token);

    return 0;
}

static int parser_next(struct parser *parser, struct token *token) {
    if (parser->index >= parser->tokens->count) {
        return 1;
    }

    ds_dynamic_array_get(parser->tokens, parser->index, token);
    parser->index++;

    return 0;
}

static void parser_recovery(struct parser *parser) {
    while (parser->index < parser->tokens->count) {
        struct token token;
        ds_dynamic_array_get(parser->tokens, parser->index, &token);

        if (token.type == SEMICOLON) {
            parser->index++;
            break;
        }

        parser->index++;
    }
}

static int parser_attribute(struct parser *parser, attribute_node *attribute) {
    int result = 0;
    struct token token;

    parser_next(parser, &token);
    if (token.type != IDENT) {
        return_defer(1);
    }

    attribute->name.value = token.literal;
    attribute->name.pos = token.pos;

    parser_next(parser, &token);
    if (token.type != COLON) {
        return_defer(1);
    }

    parser_next(parser, &token);
    if (token.type != CLASS_NAME) {
        return_defer(1);
    }

    attribute->type.value = token.literal;
    attribute->type.pos = token.pos;

    // Handle initial value optional

    parser_next(parser, &token);
    if (token.type != SEMICOLON) {
        return_defer(1);
    }

defer:
    return result;
}

static int parser_class(struct parser *parser, class_node *class) {
    int result = 0;
    struct token token;

    parser_next(parser, &token);
    if (token.type != CLASS) {
        return_defer(1);
    }

    parser_next(parser, &token);
    if (token.type != CLASS_NAME) {
        return_defer(1);
    }

    class->name.value = token.literal;
    class->name.pos = token.pos;
    class->superclass.value = NULL;
    ds_dynamic_array_init(&class->attributes, sizeof(attribute_node));

    parser_next(parser, &token);
    if (token.type == INHERITS) {
        parser_next(parser, &token);
        if (token.type != CLASS_NAME) {
            return_defer(1);
        }

        class->superclass.value = token.literal;
        class->superclass.pos = token.pos;

        parser_next(parser, &token);
    }

    if (token.type != LBRACE) {
        return_defer(1);
    }

    parser_peek(parser, &token);
    while (token.type != RBRACE) {
        attribute_node attribute;
        result = parser_attribute(parser, &attribute);

        if (result != 0) {
            parser_recovery(parser);
        }

        ds_dynamic_array_append(&class->attributes, &attribute);

        parser_peek(parser, &token);
    }
    parser_next(parser, &token);

    parser_next(parser, &token);
    if (token.type != SEMICOLON) {
        return_defer(1);
    }

defer:
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

        parser_peek(parser, &token);
    } while (token.type != END);

    return result;
}

int parser_run(const char *filename, ds_dynamic_array *tokens,
               program_node *program) {
    ds_dynamic_array_init(&program->classes, sizeof(class_node));

    struct parser parser = {.filename = filename, .tokens = tokens, .index = 0};

    int result = parser_program(&parser, program);

    return result;
}

#define INDENT_SIZE 2

static void class_print(class_node *class, unsigned int indent) {
    printf("%*sclass\n", indent, "");
    printf("%*s%s\n", indent + INDENT_SIZE, "", class->name.value);
    if (class->superclass.value != NULL) {
        printf("%*s%s\n", indent + INDENT_SIZE, "", class->superclass.value);
    }
    for (unsigned int i = 0; i < class->attributes.count; i++) {
        attribute_node attribute;
        ds_dynamic_array_get(&class->attributes, i, &attribute);
        printf("%*sattribute\n", indent + INDENT_SIZE, "");
        printf("%*s%s\n", indent + INDENT_SIZE * 2, "", attribute.name.value);
        printf("%*s%s\n", indent + INDENT_SIZE * 2, "", attribute.type.value);
    }
}

static void program_print(program_node *program, unsigned int indent) {
    printf("%*sprogram\n", indent, "");
    for (unsigned int i = 0; i < program->classes.count; i++) {
        class_node class;
        ds_dynamic_array_get(&program->classes, i, &class);
        class_print(&class, indent + INDENT_SIZE);
    }
}

void parser_print(program_node *program) { program_print(program, 0); }
