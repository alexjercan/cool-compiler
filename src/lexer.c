#include "lexer.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const char *token_type_to_string(enum token_type type) {
    switch (type) {
    case ARROW:
        return "ARROW";
    case ASSIGN:
        return "ASSIGN";
    case AT:
        return "AT";
    case BOOL_LITERAL:
        return "BOOL_LITERAL";
    case CASE:
        return "CASE";
    case CLASS:
        return "CLASS";
    case CLASS_NAME:
        return "CLASS_NAME";
    case COLON:
        return "COLON";
    case COMMA:
        return "COMMA";
    case DIVIDE:
        return "DIVIDE";
    case DOT:
        return "DOT";
    case ELSE:
        return "ELSE";
    case END:
        return "END";
    case EQUAL:
        return "EQUAL";
    case ESAC:
        return "ESAC";
    case FI:
        return "FI";
    case IDENT:
        return "IDENT";
    case IF:
        return "IF";
    case ILLEGAL:
        return "ILLEGAL";
    case IN:
        return "IN";
    case INHERITS:
        return "INHERITS";
    case INT_LITERAL:
        return "INT_LITERAL";
    case ISVOID:
        return "ISVOID";
    case LBRACE:
        return "LBRACE";
    case LESS_THAN:
        return "LESS_THAN";
    case LESS_THAN_EQ:
        return "LESS_THAN_EQ";
    case LET:
        return "LET";
    case LOOP:
        return "LOOP";
    case LPAREN:
        return "LPAREN";
    case MINUS:
        return "MINUS";
    case MULTIPLY:
        return "MULTIPLY";
    case NEW:
        return "NEW";
    case NOT:
        return "NOT";
    case OF:
        return "OF";
    case PLUS:
        return "PLUS";
    case POOL:
        return "POOL";
    case RBRACE:
        return "RBRACE";
    case RPAREN:
        return "RPAREN";
    case SEMICOLON:
        return "SEMICOLON";
    case STRING_LITERAL:
        return "STRING_LITERAL";
    case THEN:
        return "THEN";
    case TILDE:
        return "TILDE";
    case WHILE:
        return "WHILE";
    default:
        return "UNKNOWN";
    }
}

static const char *error_type_to_string(enum error_type type) {
    switch (type) {
    case NO_ERROR:
        return "";
    case INVALID_CHAR:
        return "Invalid character";
    case STRING_CONSTANT_TOO_LONG:
        return "String constant too long";
    case STRING_CONTAINS_NULL:
        return "String contains null character";
    case STRING_UNTERMINATED:
        return "Unterminated string constant";
    case STRING_CONTAINS_EOF:
        return "EOF in string constant";
    default:
        return "UNKNOWN";
    }
}

static struct token literal_to_token(char *literal) {
    if (strcmp(literal, "class") == 0) {
        DS_FREE(literal);
        return (struct token){.type = CLASS, .literal = NULL};
    } else if (strcmp(literal, "inherits") == 0) {
        DS_FREE(literal);
        return (struct token){.type = INHERITS, .literal = NULL};
    } else if (strcmp(literal, "true") == 0 || strcmp(literal, "false") == 0) {
        return (struct token){.type = BOOL_LITERAL, .literal = literal};
    } else if (strcmp(literal, "not") == 0) {
        DS_FREE(literal);
        return (struct token){.type = NOT, .literal = NULL};
    } else if (strcmp(literal, "isvoid") == 0) {
        DS_FREE(literal);
        return (struct token){.type = ISVOID, .literal = NULL};
    } else if (strcmp(literal, "new") == 0) {
        DS_FREE(literal);
        return (struct token){.type = NEW, .literal = NULL};
    } else if (strcmp(literal, "if") == 0) {
        DS_FREE(literal);
        return (struct token){.type = IF, .literal = NULL};
    } else if (strcmp(literal, "then") == 0) {
        DS_FREE(literal);
        return (struct token){.type = THEN, .literal = NULL};
    } else if (strcmp(literal, "else") == 0) {
        DS_FREE(literal);
        return (struct token){.type = ELSE, .literal = NULL};
    } else if (strcmp(literal, "fi") == 0) {
        DS_FREE(literal);
        return (struct token){.type = FI, .literal = NULL};
    } else if (strcmp(literal, "while") == 0) {
        DS_FREE(literal);
        return (struct token){.type = WHILE, .literal = NULL};
    } else if (strcmp(literal, "loop") == 0) {
        DS_FREE(literal);
        return (struct token){.type = LOOP, .literal = NULL};
    } else if (strcmp(literal, "pool") == 0) {
        DS_FREE(literal);
        return (struct token){.type = POOL, .literal = NULL};
    } else if (strcmp(literal, "let") == 0) {
        DS_FREE(literal);
        return (struct token){.type = LET, .literal = NULL};
    } else if (strcmp(literal, "in") == 0) {
        DS_FREE(literal);
        return (struct token){.type = IN, .literal = NULL};
    } else if (strcmp(literal, "case") == 0) {
        DS_FREE(literal);
        return (struct token){.type = CASE, .literal = NULL};
    } else if (strcmp(literal, "of") == 0) {
        DS_FREE(literal);
        return (struct token){.type = OF, .literal = NULL};
    } else if (strcmp(literal, "esac") == 0) {
        DS_FREE(literal);
        return (struct token){.type = ESAC, .literal = NULL};
    } else {
        return (struct token){.type = IDENT, .literal = literal};
    }
}

struct lexer {
        char *buffer;
        unsigned int buffer_len;
        unsigned int pos;
        unsigned int read_pos;
        char ch;
};

static void lexer_init(struct lexer *l, char *buffer, unsigned int buffer_len);
static char lexer_read_char(struct lexer *l);
static char lexer_peek_char(struct lexer *l);
static struct token lexer_next_token(struct lexer *l);

static void skip_whitespaces(struct lexer *l) {
    while (isspace(l->ch)) {
        lexer_read_char(l);
    }
}

static void skip_until_semi(struct lexer *l) {
    while (l->ch != ';' && l->ch != EOF) {
        lexer_read_char(l);
    }
}

static void lexer_init(struct lexer *l, char *buffer, unsigned int buffer_len) {
    l->buffer = buffer;
    l->buffer_len = buffer_len;
    l->pos = 0;
    l->read_pos = 0;
    l->ch = 0;

    lexer_read_char(l);
}

static char lexer_read_char(struct lexer *l) {
    l->ch = lexer_peek_char(l);

    l->pos = l->read_pos;
    l->read_pos += 1;

    return l->ch;
}

static char lexer_peek_char(struct lexer *l) {
    if (l->read_pos >= l->buffer_len) {
        return EOF;
    }

    return l->buffer[l->read_pos];
}

static struct token token_string_literal(struct lexer *l) {
    unsigned int position = l->pos;

    lexer_read_char(l);
    ds_string_builder builder = {.items = NULL, .count = 0, .capacity = 0};
    while (l->ch != '"') {
        char ch = l->ch;
        if (ch == EOF) {
            skip_until_semi(l);
            return (struct token){.type = ILLEGAL,
                                  .literal = NULL,
                                  .pos = position,
                                  .error = STRING_CONTAINS_EOF};
        }
        if (ch == '\0') {
            skip_until_semi(l);
            return (struct token){.type = ILLEGAL,
                                  .literal = NULL,
                                  .pos = position,
                                  .error = STRING_CONTAINS_NULL};
        }
        if (ch == '\n') {
            skip_until_semi(l);
            return (struct token){.type = ILLEGAL,
                                  .literal = NULL,
                                  .pos = position,
                                  .error = STRING_UNTERMINATED};
        }

        if (ch == '\\') {
            lexer_read_char(l);
            if (l->ch == 'n') {
                ch = '\n';
            } else if (l->ch == 't') {
                ch = '\t';
            } else if (l->ch == 'b') {
                ch = '\b';
            } else if (l->ch == 'f') {
                ch = '\f';
            } else {
                ch = l->ch;
            }
        }

        ds_string_builder_appendc(&builder, ch);

        lexer_read_char(l);
    }
    lexer_read_char(l);

    char *literal = NULL;
    ds_string_builder_build(&builder, &literal);

    if (strlen(literal) > 1024) {
        DS_FREE(literal);
        return (struct token){.type = ILLEGAL,
                              .literal = NULL,
                              .pos = position,
                              .error = STRING_CONSTANT_TOO_LONG};
    }

    return (struct token){.type = STRING_LITERAL, .literal = literal};
}

static struct token lexer_next_token(struct lexer *l) {
    skip_whitespaces(l);

    if (l->ch == EOF) {
        lexer_read_char(l);
        return (struct token){.type = END, .literal = NULL};
    } else if (l->ch == '{') {
        lexer_read_char(l);
        return (struct token){.type = LBRACE, .literal = NULL};
    } else if (l->ch == '}') {
        lexer_read_char(l);
        return (struct token){.type = RBRACE, .literal = NULL};
    } else if (l->ch == ';') {
        lexer_read_char(l);
        return (struct token){.type = SEMICOLON, .literal = NULL};
    } else if (l->ch == ':') {
        lexer_read_char(l);
        return (struct token){.type = COLON, .literal = NULL};
    } else if (l->ch == '<') {
        char next = lexer_peek_char(l);
        if (next == '-') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){.type = ASSIGN, .literal = NULL};
        } else if (next == '=') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){.type = LESS_THAN_EQ, .literal = NULL};
        } else {
            lexer_read_char(l);
            return (struct token){.type = LESS_THAN, .literal = NULL};
        }
    } else if (l->ch == '(') {
        lexer_read_char(l);
        return (struct token){.type = LPAREN, .literal = NULL};
    } else if (l->ch == ')') {
        lexer_read_char(l);
        return (struct token){.type = RPAREN, .literal = NULL};
    } else if (l->ch == ',') {
        lexer_read_char(l);
        return (struct token){.type = COMMA, .literal = NULL};
    } else if (l->ch == '+') {
        lexer_read_char(l);
        return (struct token){.type = PLUS, .literal = NULL};
    } else if (l->ch == '-') {
        lexer_read_char(l);
        return (struct token){.type = MINUS, .literal = NULL};
    } else if (l->ch == '*') {
        lexer_read_char(l);
        return (struct token){.type = MULTIPLY, .literal = NULL};
    } else if (l->ch == '/') {
        lexer_read_char(l);
        return (struct token){.type = DIVIDE, .literal = NULL};
    } else if (l->ch == '~') {
        lexer_read_char(l);
        return (struct token){.type = TILDE, .literal = NULL};
    } else if (l->ch == '=') {
        char next = lexer_peek_char(l);
        if (next == '>') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){.type = ARROW, .literal = NULL};
        } else {
            lexer_read_char(l);
            return (struct token){.type = EQUAL, .literal = NULL};
        }
    } else if (l->ch == '.') {
        lexer_read_char(l);
        return (struct token){.type = DOT, .literal = NULL};
    } else if (l->ch == '@') {
        lexer_read_char(l);
        return (struct token){.type = AT, .literal = NULL};
    } else if (l->ch == '"') {
        return token_string_literal(l);
    } else if (islower(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isalnum(l->ch) || l->ch == '_') {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return literal_to_token(literal);
    } else if (isupper(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isalnum(l->ch) || l->ch == '_') {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){.type = CLASS_NAME, .literal = literal};
    } else if (isdigit(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isdigit(l->ch)) {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){.type = INT_LITERAL, .literal = literal};
    } else {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 1};
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        struct token il = {.type = ILLEGAL,
                           .literal = literal,
                           .pos = l->pos,
                           .error = INVALID_CHAR};

        lexer_read_char(l);
        return il;
    }
};

void pos_to_lc(char *buffer, unsigned int pos, unsigned int *line,
               unsigned int *col) {
    *line = 1;
    *col = 1;
    for (unsigned int i = 0; i < pos; i++) {
        if (buffer[i] == '\n') {
            *line += 1;
            *col = 1;
        } else {
            *col += 1;
        }
    }
}

int run_lexer(char *buffer, int length, const char *filename,
              ds_dynamic_array *tokens) {
    int result = 0;

    struct lexer l;
    lexer_init(&l, (char *)buffer, length);

    struct token tok;

    do {
        tok = lexer_next_token(&l);
        if (tok.type == ILLEGAL) {
            unsigned int line, col;
            pos_to_lc(buffer, tok.pos, &line, &col);
            if (filename == NULL) {
                if (tok.literal != NULL) {
                    printf("line %d:%d, Lexical error: %s: %s\n", line, col,
                           error_type_to_string(tok.error), tok.literal);
                } else {
                    printf("line %d:%d, Lexical error: %s\n", line, col,
                           error_type_to_string(tok.error));
                }
            } else {
                if (tok.literal != NULL) {
                    printf("\"%s\", %d:%d: Lexical error: %s: %s\n", filename,
                           line, col, error_type_to_string(tok.error),
                           tok.literal);
                } else {
                    printf("\"%s\", %d:%d: Lexical error: %s\n", filename, line,
                           col, error_type_to_string(tok.error));
                }
            }

            result = 1;

            if (tok.literal != NULL) {
                DS_FREE(tok.literal);
            }
        } else {
            ds_dynamic_array_append(tokens, &tok);
        }
    } while (tok.type != END);

    return result;
}
