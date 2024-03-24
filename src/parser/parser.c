#include "parser.h"
#include "ds.h"
#include "lexer.h"
#include <stdarg.h>

struct parser {
        const char *filename;
        ds_dynamic_array *tokens;
        unsigned int index;
        int result;
        int panicd;
        FILE *error_fd;
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

    parser->panicd = 0;

    return 0;
}

static void parser_show_errorf(struct parser *parser, const char *format, ...) {
    parser->result = PARSER_ERROR;

    struct token token;
    parser_current(parser, &token);

    const char *filename = parser->filename;

    if (token.type == ILLEGAL) {
        if (filename != NULL) {
            fprintf(parser->error_fd, "\"%s\", ", filename);
        }

        fprintf(parser->error_fd, "line %d:%d, Lexical error: %s", token.line, token.col,
               error_type_to_string(token.error));

        if (token.literal != NULL) {
            fprintf(parser->error_fd, ": %s", token.literal);
        }

        fprintf(parser->error_fd, "\n");
    } else if (parser->panicd == 0) {
        if (filename != NULL) {
            fprintf(parser->error_fd, "\"%s\", ", filename);
        }
        fprintf(parser->error_fd, "line %d:%d, Syntax error: ", token.line, token.col);

        va_list args;
        va_start(args, format);
        vfprintf(parser->error_fd, format, args);
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

static int parser_is_sync_point(struct parser *parser) {
    struct token token;
    parser_current(parser, &token);
    switch (token.type) {
    case FI:
    case POOL:
    case ESAC:
    case RBRACE:
    case RPAREN:
    case SEMICOLON:
    case END:
        return 1;
    default:
        return 0;
    }
}

static void parser_panic_mode(struct parser *parser) {
    while (!parser_is_sync_point(parser)) {
        parser_advance(parser);
    }

    parser->panicd = 1;
}

static void build_expr(struct parser *parser, expr_node *expr);

static void build_node_if(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_COND;
    expr->cond.predicate = malloc(sizeof(expr_node));
    expr->cond.then = malloc(sizeof(expr_node));
    expr->cond.else_ = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != IF) {
        parser_show_expected(parser, IF, token.type);
        return parser_panic_mode(parser);
    }

    expr->cond.node.value = "if";

    expr->cond.node.line = token.line;
    expr->cond.node.col = token.col;

    parser_advance(parser);

    build_expr(parser, expr->cond.predicate);

    parser_current(parser, &token);
    if (token.type != THEN) {
        parser_show_expected(parser, THEN, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    build_expr(parser, expr->cond.then);

    parser_current(parser, &token);
    if (token.type != ELSE) {
        parser_show_expected(parser, ELSE, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    build_expr(parser, expr->cond.else_);

    parser_current(parser, &token);
    if (token.type != FI) {
        parser_show_expected(parser, FI, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);
}

static void build_node_while(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_LOOP;
    expr->loop.predicate = malloc(sizeof(expr_node));
    expr->loop.body = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != WHILE) {
        parser_show_expected(parser, WHILE, token.type);
        return parser_panic_mode(parser);
    }

    expr->loop.node.value = "while";

    expr->loop.node.line = token.line;
    expr->loop.node.col = token.col;

    parser_advance(parser);

    build_expr(parser, expr->loop.predicate);

    parser_current(parser, &token);
    if (token.type != LOOP) {
        parser_show_expected(parser, LOOP, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    build_expr(parser, expr->loop.body);

    parser_current(parser, &token);
    if (token.type != POOL) {
        parser_show_expected(parser, POOL, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);
}

static void build_node_block(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_BLOCK;
    ds_dynamic_array_init(&expr->block.exprs, sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        parser_show_expected(parser, LBRACE, token.type);
        return parser_panic_mode(parser);
    }

    expr->block.node.value = "{";

    expr->block.node.line = token.line;
    expr->block.node.col = token.col;

    parser_advance(parser);

    parser_current(parser, &token);
    while (token.type != RBRACE) {
        if (token.type == END) {
            parser_show_expected(parser, RBRACE, token.type);
            return;
        }

        expr_node line;

        build_expr(parser, &line);

        ds_dynamic_array_append(&expr->block.exprs, &line);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            parser_show_expected(parser, SEMICOLON, token.type);
            return parser_panic_mode(parser);
        }
        parser_advance(parser);

        parser_current(parser, &token);
    }

    parser_advance(parser);
}

static void build_node_let_init(struct parser *parser, let_init_node *init) {
    struct token token;

    init->name.value = NULL;
    init->type.value = NULL;
    init->init = NULL;

    parser_current(parser, &token);
    if (token.type == IDENT) {
        init->name.value = token.literal;
        init->name.line = token.line;
        init->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        init->type.value = token.literal;
        init->type.line = token.line;
        init->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == ASSIGN) {
        init->init = malloc(sizeof(expr_node));

        parser_advance(parser);

        build_expr(parser, init->init);
    }
}

static void build_node_let(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_LET;
    ds_dynamic_array_init(&expr->let.inits, sizeof(let_init_node));
    expr->let.body = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != LET) {
        parser_show_expected(parser, LET, token.type);
        return parser_panic_mode(parser);
    }

    expr->let.node.value = "let";

    expr->let.node.line = token.line;
    expr->let.node.col = token.col;

    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == IDENT) {
        let_init_node init;

        build_node_let_init(parser, &init);

        ds_dynamic_array_append(&expr->let.inits, &init);
    } else {
        parser_show_expected(parser, IDENT, token.type);
        return parser_panic_mode(parser);
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

        build_node_let_init(parser, &init);

        ds_dynamic_array_append(&expr->let.inits, &init);

        parser_current(parser, &token);
    }
    parser_advance(parser);

    build_expr(parser, expr->let.body);
}

static void build_node_branch(struct parser *parser, branch_node *branch) {
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
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        branch->type.value = token.literal;
        branch->type.line = token.line;
        branch->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != ARROW) {
        parser_show_expected(parser, ARROW, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    build_expr(parser, branch->body);
}

static void build_node_case(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_CASE;
    expr->case_.expr = malloc(sizeof(expr_node));
    ds_dynamic_array_init(&expr->case_.cases, sizeof(branch_node));

    parser_current(parser, &token);
    if (token.type != CASE) {
        parser_show_expected(parser, CASE, token.type);
        return parser_panic_mode(parser);
    }

    expr->case_.node.value = "case";

    expr->case_.node.line = token.line;
    expr->case_.node.col = token.col;

    parser_advance(parser);

    build_expr(parser, expr->case_.expr);

    parser_current(parser, &token);
    if (token.type != OF) {
        parser_show_expected(parser, OF, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    do {
        branch_node branch;

        build_node_branch(parser, &branch);

        ds_dynamic_array_append(&expr->case_.cases, &branch);

        parser_current(parser, &token);
        if (token.type != SEMICOLON) {
            parser_show_expected(parser, SEMICOLON, token.type);
            return parser_panic_mode(parser);
        }
        parser_advance(parser);

        parser_current(parser, &token);
    } while (token.type != ESAC);
    parser_advance(parser);
}

static void build_node_new(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_NEW;

    parser_current(parser, &token);
    if (token.type != NEW) {
        parser_show_expected(parser, NEW, token.type);
        return parser_panic_mode(parser);
    }

    expr->new.node.value = "new";

    expr->new.node.line = token.line;
    expr->new.node.col = token.col;

    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        expr->new.type.value = token.literal;

        expr->new.type.line = token.line;
        expr->new.type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);
}

static void build_node_paren(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_PAREN;
    expr->paren = malloc(sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type != LPAREN) {
        parser_show_expected(parser, LPAREN, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    build_expr(parser, expr->paren);

    parser_current(parser, &token);
    if (token.type != RPAREN) {
        parser_show_expected(parser, RPAREN, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);
}

static void build_node_fcall(struct parser *parser, expr_node *expr) {
    struct token token;

    expr->type = NULL;
    expr->kind = EXPR_DISPATCH;
    ds_dynamic_array_init(&expr->dispatch.args, sizeof(expr_node));

    parser_current(parser, &token);
    if (token.type == IDENT) {
        expr->dispatch.method.value = token.literal;
        expr->dispatch.method.line = token.line;
        expr->dispatch.method.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != LPAREN) {
        parser_show_expected(parser, LPAREN, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != RPAREN) {
        expr_node arg;

        build_expr(parser, &arg);

        ds_dynamic_array_append(&expr->dispatch.args, &arg);

        parser_current(parser, &token);
        while (token.type != RPAREN) {
            if (token.type != COMMA) {
                parser_show_extected_2(parser, COMMA, RPAREN, token.type);
                if (token.type == END) {
                    return;
                }
            }
            parser_advance(parser);

            build_expr(parser, &arg);

            ds_dynamic_array_append(&expr->dispatch.args, &arg);

            parser_current(parser, &token);
        }
    }

    parser_advance(parser);
}

static void build_expr(struct parser *parser, expr_node *expr);

static void build_expr_simple(struct parser *parser, expr_node *expr) {
    struct token token;

    parser_current(parser, &token);
    switch (token.type) {
    case IDENT: {
        struct token next;
        parser_peek(parser, &next);
        if (next.type == LPAREN) {
            build_node_fcall(parser, expr);
        } else {
            expr->type = NULL;
            expr->kind = EXPR_IDENT;
            expr->ident.value = token.literal;
            expr->ident.line = token.line;
            expr->ident.col = token.col;

            parser_advance(parser);
        }
        break;
    }
    case INT_LITERAL:
        expr->type = NULL;
        expr->kind = EXPR_INT;
        expr->integer.value = token.literal;
        expr->integer.line = token.line;
        expr->integer.col = token.col;

        parser_advance(parser);
        break;
    case STRING_LITERAL:
        expr->type = NULL;
        expr->kind = EXPR_STRING;
        expr->string.value = token.literal;
        expr->string.line = token.line;
        expr->string.col = token.col;

        parser_advance(parser);
        break;
    case BOOL_LITERAL:
        expr->type = NULL;
        expr->kind = EXPR_BOOL;
        expr->boolean.value = token.literal;
        expr->boolean.line = token.line;
        expr->boolean.col = token.col;

        parser_advance(parser);
        break;
    case LPAREN:
        build_node_paren(parser, expr);
        break;
    case IF:
        build_node_if(parser, expr);
        break;
    case WHILE:
        build_node_while(parser, expr);
        break;
    case LBRACE:
        build_node_block(parser, expr);
        break;
    case LET:
        build_node_let(parser, expr);
        break;
    case CASE:
        build_node_case(parser, expr);
        break;
    case NEW:
        build_node_new(parser, expr);
        break;
    default: {
        parser_show_errorf(parser, "unexpected token %s",
                           token_type_to_string(token.type));
        parser_panic_mode(parser);
        break;
    }
    }
}

static void build_expr_at(struct parser *parser, expr_node *expr) {
    struct token token;
    struct token next;

    expr_node *root = malloc(sizeof(expr_node));
    build_expr_simple(parser, root);

    parser_current(parser, &token);
    parser_peek(parser, &next);
    while (token.type == AT || token.type == DOT) {
        expr_node *current = malloc(sizeof(expr_node));

        current->type = NULL;
        current->kind = EXPR_DISPATCH_FULL;
        current->dispatch_full.expr = root;
        current->dispatch_full.type.value = NULL;
        current->dispatch_full.dispatch = malloc(sizeof(struct dispatch_node));

        parser_current(parser, &token);
        if (token.type == AT) {
            parser_advance(parser);

            parser_current(parser, &token);
            if (token.type == CLASS_NAME) {
                current->dispatch_full.type.value = token.literal;
                current->dispatch_full.type.line = token.line;
                current->dispatch_full.type.col = token.col;
            } else {
                parser_show_expected(parser, CLASS_NAME, token.type);
                return parser_panic_mode(parser);
            }
            parser_advance(parser);
        }

        parser_current(parser, &token);
        if (token.type != DOT) {
            parser_show_expected(parser, DOT, token.type);
            return parser_panic_mode(parser);
        }
        parser_advance(parser);

        expr_node fcall;

        build_node_fcall(parser, &fcall);

        if (fcall.kind == EXPR_DISPATCH) {
            *current->dispatch_full.dispatch = fcall.dispatch;
        }

        root = current;

        parser_current(parser, &token);
        parser_peek(parser, &next);
    }

    *expr = *root;
}

static void build_expr_neg(struct parser *parser, expr_node *expr) {
    struct token token;

    parser_current(parser, &token);
    if (token.type == TILDE) {
        expr->type = NULL;
        expr->kind = EXPR_NEG;
        expr->neg.expr = malloc(sizeof(expr_node));

        expr->neg.op.value = "~";

        expr->neg.op.line = token.line;
        expr->neg.op.col = token.col;

        parser_advance(parser);

        build_expr_neg(parser, expr->neg.expr);
    } else {
        build_expr_at(parser, expr);
    }
}

static void build_expr_isvoid(struct parser *parser, expr_node *expr) {
    struct token token;

    parser_current(parser, &token);
    if (token.type == ISVOID) {
        expr->type = NULL;
        expr->kind = EXPR_ISVOID;
        expr->isvoid.expr = malloc(sizeof(expr_node));

        expr->isvoid.op.value = "isvoid";

        expr->isvoid.op.line = token.line;
        expr->isvoid.op.col = token.col;

        parser_advance(parser);

        build_expr_isvoid(parser, expr->isvoid.expr);
    } else {
        build_expr_neg(parser, expr);
    }
}

static void build_expr_mul(struct parser *parser, expr_node *expr) {
    struct token token;
    struct token next;

    expr_node *root = malloc(sizeof(expr_node));
    build_expr_isvoid(parser, root);

    parser_current(parser, &token);
    parser_peek(parser, &next);

    while (token.type == MULTIPLY || token.type == DIVIDE) {
        expr_node *current = malloc(sizeof(expr_node));
        expr_binary_node *current_binary;

        current->type = NULL;

        if (token.type == MULTIPLY) {
            current->kind = EXPR_MUL;
            current_binary = &current->mul;
            current_binary->op.value = "*";
        } else {
            current->kind = EXPR_DIV;
            current_binary = &current->div;
            current_binary->op.value = "/";
        }

        current_binary->op.line = token.line;
        current_binary->op.col = token.col;

        current_binary->lhs = root;
        current_binary->rhs = malloc(sizeof(expr_node));

        parser_advance(parser);

        build_expr_isvoid(parser, current_binary->rhs);

        root = current;

        parser_current(parser, &token);
        parser_peek(parser, &next);
    }

    *expr = *root;
}

static void build_expr_add(struct parser *parser, expr_node *expr) {
    struct token token;
    struct token next;

    expr_node *root = malloc(sizeof(expr_node));
    build_expr_mul(parser, root);

    parser_current(parser, &token);
    parser_peek(parser, &next);

    while (token.type == PLUS || token.type == MINUS) {
        expr_node *current = malloc(sizeof(expr_node));
        expr_binary_node *current_binary;

        current->type = NULL;

        if (token.type == PLUS) {
            current->kind = EXPR_ADD;
            current_binary = &current->add;
            current_binary->op.value = "+";
        } else {
            current->kind = EXPR_SUB;
            current_binary = &current->sub;
            current_binary->op.value = "-";
        }

        current_binary->op.line = token.line;
        current_binary->op.col = token.col;

        current_binary->lhs = root;
        current_binary->rhs = malloc(sizeof(expr_node));

        parser_advance(parser);

        build_expr_mul(parser, current_binary->rhs);

        root = current;

        parser_current(parser, &token);
        parser_peek(parser, &next);
    }

    *expr = *root;
}

static void build_expr_cmp(struct parser *parser, expr_node *expr) {
    struct token token;
    struct token next;

    expr_node *root = malloc(sizeof(expr_node));
    build_expr_add(parser, root);

    parser_current(parser, &token);
    parser_peek(parser, &next);

    while (token.type == LESS_THAN_EQ || token.type == LESS_THAN ||
           token.type == EQUAL) {
        expr_node *current = malloc(sizeof(expr_node));
        expr_binary_node *current_binary;

        current->type = NULL;

        if (token.type == LESS_THAN_EQ) {
            current->kind = EXPR_LE;
            current_binary = &current->le;
            current_binary->op.value = "<=";
        } else if (token.type == LESS_THAN) {
            current->kind = EXPR_LT;
            current_binary = &current->lt;
            current_binary->op.value = "<";
        } else {
            current->kind = EXPR_EQ;
            current_binary = &current->eq;
            current_binary->op.value = "=";
        }

        current_binary->op.line = token.line;
        current_binary->op.col = token.col;

        current_binary->lhs = root;
        current_binary->rhs = malloc(sizeof(expr_node));

        parser_advance(parser);

        build_expr_add(parser, current_binary->rhs);

        root = current;

        parser_current(parser, &token);
        parser_peek(parser, &next);
    }

    *expr = *root;
}

static void build_expr_not(struct parser *parser, expr_node *expr) {
    struct token token;
    struct token next;

    parser_current(parser, &token);
    parser_peek(parser, &next);
    if (token.type == NOT) {
        expr->type = NULL;
        expr->kind = EXPR_NOT;
        expr->not_.expr = malloc(sizeof(expr_node));

        expr->not_.op.value = "not";

        expr->not_.op.line = token.line;
        expr->not_.op.col = token.col;

        parser_advance(parser);

        build_expr_not(parser, expr->not_.expr);
    } else {
        build_expr_cmp(parser, expr);
    }
}

static void build_expr(struct parser *parser, expr_node *expr) {
    struct token token;
    struct token next;

    expr->type = NULL;
    expr->kind = EXPR_NONE;

    parser_current(parser, &token);
    parser_peek(parser, &next);
    if (token.type == IDENT && next.type == ASSIGN) {
        expr->kind = EXPR_ASSIGN;
        expr->assign.name.value = token.literal;
        expr->assign.name.line = token.line;
        expr->assign.name.col = token.col;
        expr->assign.value = malloc(sizeof(expr_node));

        parser_advance(parser);
        parser_advance(parser);

        build_expr(parser, expr->assign.value);
    } else {
        build_expr_not(parser, expr);
    }
}

static void build_attribute(struct parser *parser, attribute_node *attribute) {
    struct token token;

    attribute->name.value = NULL;
    attribute->type.value = NULL;
    attribute->value.kind = EXPR_NONE;

    parser_current(parser, &token);
    if (token.type == IDENT) {
        attribute->name.value = token.literal;
        attribute->name.line = token.line;
        attribute->name.col = token.col;
    } else {
        parser_show_expected(parser, IDENT, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        attribute->type.value = token.literal;
        attribute->type.line = token.line;
        attribute->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);

    if (token.type == ASSIGN) {
        parser_advance(parser);

        parser_current(parser, &token);
        if (token.type == EXTERN) {
            attribute->value.type = NULL;
            attribute->value.kind = EXPR_EXTERN;

            parser_advance(parser);
        } else {
            build_expr(parser, &attribute->value);
        }
    } else {
        attribute->value.type = NULL;
        attribute->value.kind = EXPR_NULL;
        attribute->value.null.type.value = attribute->type.value;
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
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != COLON) {
        parser_show_expected(parser, COLON, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        formal->type.value = token.literal;
        formal->type.line = token.line;
        formal->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
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
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type != LPAREN) {
        parser_show_expected(parser, LPAREN, token.type);
        return parser_panic_mode(parser);
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
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        method->type.value = token.literal;
        method->type.line = token.line;
        method->type.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == EXTERN) {
        method->body.type = NULL;
        method->body.kind = EXPR_EXTERN;
    } else {
        if (token.type != LBRACE) {
            parser_show_expected(parser, LBRACE, token.type);
            return parser_panic_mode(parser);
        }
        parser_advance(parser);

        build_expr(parser, &method->body);

        parser_current(parser, &token);
        if (token.type != RBRACE) {
            parser_show_expected(parser, RBRACE, token.type);
            return parser_panic_mode(parser);
        }
    }
    parser_advance(parser);
}

static void build_feature(struct parser *parser, class_node *class) {
    struct token token;

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

    class->filename = parser->filename;
    class->name.value = NULL;
    class->superclass.value = NULL;
    ds_dynamic_array_init(&class->attributes, sizeof(attribute_node));
    ds_dynamic_array_init(&class->methods, sizeof(method_node));

    parser_current(parser, &token);
    if (token.type != CLASS) {
        parser_show_expected(parser, CLASS, token.type);
        return parser_panic_mode(parser);
    }
    parser_advance(parser);

    parser_current(parser, &token);
    if (token.type == CLASS_NAME) {
        class->name.value = token.literal;
        class->name.line = token.line;
        class->name.col = token.col;
    } else {
        parser_show_expected(parser, CLASS_NAME, token.type);
        return parser_panic_mode(parser);
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
            return parser_panic_mode(parser);
        }

        parser_advance(parser);
    }

    parser_current(parser, &token);
    if (token.type != LBRACE) {
        parser_show_expected(parser, LBRACE, token.type);
        return parser_panic_mode(parser);
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
            return parser_panic_mode(parser);
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
            return parser_panic_mode(parser);
        }
        parser_advance(parser);

        parser_current(parser, &token);
    } while (token.type != END);
}

enum parser_result parser_run(const char *filename, ds_dynamic_array *tokens,
                              program_node *program) {

    ds_dynamic_array_init(&program->classes, sizeof(class_node));
    program->filename = filename;

    struct parser parser = {.filename = filename,
                            .tokens = tokens,
                            .index = 0,
                            .result = PARSER_OK,
                            .panicd = 0,
                            .error_fd = stderr};

    build_program(&parser, program);

    return parser.result;
}

void parser_merge(ds_dynamic_array programs, program_node *program,
                  unsigned int index) {
    ds_dynamic_array_init(&program->classes, sizeof(class_node));

    for (unsigned int i = index; i < programs.count; i++) {
        program_node *p = NULL;
        ds_dynamic_array_get_ref(&programs, i, (void **)&p);

        for (unsigned int j = 0; j < p->classes.count; j++) {
            class_node *c = NULL;
            ds_dynamic_array_get_ref(&p->classes, j, (void **)&c);

            ds_dynamic_array_append(&program->classes, c);
        }
    }
}
