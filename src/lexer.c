#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#define DS_SS_IMPLEMENTATION
#include "ds.h"
#include <ctype.h>

enum token_type {
    ARROW,
    ASSIGN,
    AT,
    BOOL_LITERAL,
    CASE,
    CLASS,
    CLASS_NAME,
    COLON,
    COMMA,
    DIVIDE,
    DOT,
    ELSE,
    END,
    EQUAL,
    ESAC,
    FI,
    IDENT,
    IF,
    ILLEGAL,
    IN,
    INHERITS,
    INT_LITERAL,
    ISVOID,
    LBRACE,
    LESS_THAN,
    LESS_THAN_EQ,
    LET,
    LOOP,
    LPAREN,
    MINUS,
    MULTIPLY,
    NEW,
    NOT,
    OF,
    PLUS,
    POOL,
    RBRACE,
    RPAREN,
    SEMICOLON,
    STRING_LITERAL,
    THEN,
    TILDE,
    WHILE,
};

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

struct token {
        enum token_type type;
        char *literal;
};

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

static void lexer_init(struct lexer *l, char *buffer);
static char lexer_read_char(struct lexer *l);
static char lexer_peek_char(struct lexer *l);
static struct token lexer_next_token(struct lexer *l);

static void skip_whitespaces(struct lexer *l) {
    while (isspace(l->ch)) {
        lexer_read_char(l);
    }
}

static void lexer_init(struct lexer *l, char *buffer) {
    l->buffer = buffer;
    l->buffer_len = strlen(buffer);
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
        return '\0';
    }

    return l->buffer[l->read_pos];
}

static struct token lexer_next_token(struct lexer *l) {
    skip_whitespaces(l);

    if (l->ch == '\0') {
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
        lexer_read_char(l);
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (l->ch != '"') {
            slice.len += 1;
            lexer_read_char(l);
        }
        lexer_read_char(l);

        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){.type = STRING_LITERAL, .literal = literal};
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
        lexer_read_char(l);
        return (struct token){.type = ILLEGAL, .literal = NULL};
    }
};

int run_lexer(char *buffer) {
    struct lexer l;
    lexer_init(&l, (char *)buffer);

    struct token tok;

    do {
        tok = lexer_next_token(&l);
        printf("%s", token_type_to_string(tok.type));
        if (tok.literal != NULL) {
            printf("(%s)", tok.literal);
        }
        printf("\n");

        DS_FREE(tok.literal);
    } while (tok.type != END);

    return 0;
}
