#include "parser.h"
#include "ds.h"
#include "lexer.h"
#include <stdarg.h>

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

static void build_expr(struct parser *parser, expr_node *expr);

static void build_expr_if(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_COND;
    expr->cond.predicate = malloc(sizeof(expr_node));
    expr->cond.then = malloc(sizeof(expr_node));
    expr->cond.else_ = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != IF) {
        parser_show_expected(parser, IF, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->cond.predicate);

    parser_current(parser, &token);
    if (token.type != THEN) {
        parser_show_expected(parser, THEN, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->cond.then);

    parser_current(parser, &token);
    if (token.type != ELSE) {
        parser_show_expected(parser, ELSE, token.type);
        if (token.type == END || token.type == FI) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->cond.else_);

    parser_current(parser, &token);
    if (token.type != FI) {
        parser_show_expected(parser, FI, token.type);
        if (token.type == END) {
            return;
        }
    }
}

static void build_expr_while(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_LOOP;
    expr->loop.predicate = malloc(sizeof(expr_node));
    expr->loop.body = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != WHILE) {
        parser_show_expected(parser, WHILE, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->loop.predicate);

    parser_current(parser, &token);
    if (token.type != LOOP) {
        parser_show_expected(parser, LOOP, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->loop.body);

    parser_current(parser, &token);
    if (token.type != POOL) {
        parser_show_expected(parser, POOL, token.type);
        if (token.type == END) {
            return;
        }
    }
}

static void build_expr_block(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_BLOCK;
    ds_dynamic_array_init(&expr->block, sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        parser_show_expected(parser, LBRACE, token.type);
        if (token.type == END) {
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

        expr_node expr;

        build_expr(parser, &expr);

        ds_dynamic_array_append(&expr.block, &expr);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            parser_show_expected(parser, SEMICOLON, token.type);
            if (token.type == END || token.type == RBRACE) {
                return;
            }
        }
    }
    parser_advance(parser);
}

static void build_let_init(struct parser *parser, let_init_node *init) {
    struct token token;

    init->name.value = NULL;
    init->type.value = NULL;
    init->init = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type == IDENT) {
        init->name.value = token.literal;
        init->name.line = token.line;
        init->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        init->type.value = token.literal;
        init->type.line = token.line;
        init->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == ASSIGN) {
        parser_advance(parser);

        build_expr(parser, init->init);
    }
}

static void build_expr_let(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_LET;
    ds_dynamic_array_init(&expr->let.inits, sizeof(let_init_node));
    expr->let.body = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != LET) {
        parser_show_expected(parser, LET, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == IDENT) {
        let_init_node init;

        build_let_init(parser, &init);

        ds_dynamic_array_append(&expr->let.inits, &init);
    } else {
        parser_show_expected(parser, IDENT, token.type);
        if (token.type == END) {
            return;
        }
    }

    parser_current(parser, &token);
    while (token.type != IN) {
        if (token.type != COMMA) {
            parser_show_extected_2(parser, COMMA, IN, token.type);
            if (token.type == END) {
                return;
            }
        }
        parser_advance(parser);

        let_init_node init;

        build_let_init(parser, &init);

        ds_dynamic_array_append(&expr->let.inits, &init);

        parser_current(parser, &token);
    }
    parser_advance(parser);

    build_expr(parser, expr->let.body);
}

static void build_branch(struct parser *parser, branch_node *branch) {
    struct token token;

    branch->name.value = NULL;
    branch->type.value = NULL;
    branch->body = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type == IDENT) {
        branch->name.value = token.literal;
        branch->name.line = token.line;
        branch->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        branch->type.value = token.literal;
        branch->type.line = token.line;
        branch->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != ARROW) {
        parser_show_expected(parser, ARROW, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, branch->body);
}

static void build_expr_case(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_CASE;
    expr->case_.expr = malloc(sizeof(expr_node));
    ds_dynamic_array_init(&expr->case_.cases, sizeof(branch_node));

    parser_current(parser, &token);
    if (token.type != CASE) {
        parser_show_expected(parser, CASE, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->case_.expr);

    parser_current(parser, &token);
    if (token.type != OF) {
        parser_show_expected(parser, OF, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    do {
        branch_node branch;

        build_branch(parser, &branch);

        ds_dynamic_array_append(&expr->case_.cases, &branch);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            parser_show_expected(parser, SEMICOLON, token.type);
            if (token.type == END || token.type == ESAC) {
                return;
            }
        }
        parser_advance(parser);

        parser_current(parser, &token);
    } while (token.type != ESAC);
    parser_advance(parser);
}

static void build_expr_new(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_NEW;
    expr->new.value = NULL;

    parser_current(parser, &token);
    if (token.type != NEW) {
        parser_show_expected(parser, NEW, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        expr->new.value = token.literal;
        expr->new.line = token.line;
        expr->new.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);
}

static void build_expr_isvoid(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_ISVOID;
    expr->isvoid = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != ISVOID) {
        parser_show_expected(parser, ISVOID, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->isvoid);
}

static void build_expr_neg(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_NEG;
    expr->neg.expr = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != TILDE) {
        parser_show_expected(parser, TILDE, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->neg.expr);
}

static void build_expr_not(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_NOT;
    expr->not.expr = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != NOT) {
        parser_show_expected(parser, NOT, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->not.expr);
}

static void build_expr_paren(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_PAREN;
    expr->paren = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != LPAREN) {
        parser_show_expected(parser, LPAREN, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);

    build_expr(parser, expr->paren);

    parser_current(parser, &token);
    if (token.type != RPAREN) {
        parser_show_expected(parser, RPAREN, token.type);
        if (token.type == END) {
            return;
        }
    }
    parser_advance(parser);
}

static void build_expr(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = EXPR_NONE;

    parser_current(parser, &token);
    switch (token.type) {
    case IF: {
        build_expr_if(parser, expr);
        break;
    }
    case WHILE: {
        build_expr_while(parser, expr);
        break;
    }
    case LBRACE: {
        build_expr_block(parser, expr);
        break;
    }
    case LET: {
        build_expr_let(parser, expr);
        break;
    }
    case CASE: {
        build_expr_case(parser, expr);
        break;
    }
    case NEW: {
        build_expr_new(parser, expr);
        break;
    }
    case ISVOID: {
        build_expr_isvoid(parser, expr);
        break;
    }
    case TILDE: {
        build_expr_neg(parser, expr);
        break;
    }
    case NOT: {
        build_expr_not(parser, expr);
        break;
    }
    case LPAREN: {
        build_expr_paren(parser, expr);
        break;
    }
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

    attribute->name.value = NULL;
    attribute->type.value = NULL;

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

    formal->name.value = NULL;
    formal->type.value = NULL;

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

    method->name.value = NULL;
    method->type.value = NULL;
    ds_dynamic_array_init(&method->formals, sizeof(formal_node));

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

        build_formal(parser, &formal);

        ds_dynamic_array_append(&method->formals, &formal);

        parser_current(parser, &token);
    }

    while (token.type != RPAREN) {
        struct formal_node formal;

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

        build_attribute(parser, &attribute);

        ds_dynamic_array_append(&class->attributes, &attribute);
    } else {
        method_node method;

        build_method(parser, &method);

        ds_dynamic_array_append(&class->methods, &method);
    }
}

static void build_class(struct parser *parser, class_node *class) {
    struct token token;

    class->name.value = NULL;
    class->superclass.value = NULL;
    ds_dynamic_array_init(&class->attributes, sizeof(attribute_node));
    ds_dynamic_array_init(&class->methods, sizeof(method_node));

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
