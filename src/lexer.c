#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#define DSH_STATIC
#define DS_IMPLEMENTATION
#include "ds.h"

static int is_lower_letter(char ch) { return 'a' <= ch && ch <= 'z'; }

static int is_upper_letter(char ch) { return 'A' <= ch && ch <= 'Z'; }

static int is_digit(char ch) { return '0' <= ch && ch <= '9'; }

static int is_word_char(char ch) {
    return is_lower_letter(ch) || is_upper_letter(ch) || is_digit(ch) ||
           ch == '_';
}

static int is_whitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

enum token_type {
    CLASS,
    CLASS_NAME,
    END,
    ILLEGAL,
    INHERITS,
    LBRACE,
    RBRACE,
    SEMICOLON,
};

const char *token_type_to_string(enum token_type type) {
    switch (type) {
    case CLASS:
        return "CLASS";
    case CLASS_NAME:
        return "CLASS_NAME";
    case END:
        return "END";
    case ILLEGAL:
        return "ILLEGAL";
    case INHERITS:
        return "INHERITS";
    case LBRACE:
        return "LBRACE";
    case RBRACE:
        return "RBRACE";
    case SEMICOLON:
        return "SEMICOLON";
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
    } else {
        return (struct token){.type = ILLEGAL, .literal = literal};
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
    while (is_whitespace(l->ch)) {
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
    } else if (is_lower_letter(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (is_word_char(l->ch)) {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return literal_to_token(literal);
    } else if (is_upper_letter(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (is_word_char(l->ch)) {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){.type = CLASS_NAME, .literal = literal};
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
