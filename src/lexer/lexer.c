#include "lexer.h"
#include "ds.h"
#include "util.h"
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
    case EXTERN:
        return "EXTERN";
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

const char *error_type_to_string(enum error_type type) {
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
    case UNMATCHED_COMMENT:
        return "Unmatched *)";
    case UNTERMINATED_COMMENT:
        return "EOF in comment";
    default:
        return "UNKNOWN";
    }
}

static struct token literal_to_token(char *literal) {
    if (strcmp(literal, "class") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = CLASS, .literal = NULL};
    } else if (strcmp(literal, "inherits") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = INHERITS, .literal = NULL};
    } else if (strcmp(literal, "true") == 0 || strcmp(literal, "false") == 0) {
        return (struct token){.type = BOOL_LITERAL, .literal = literal};
    } else if (strcmp(literal, "not") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = NOT, .literal = NULL};
    } else if (strcmp(literal, "isvoid") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = ISVOID, .literal = NULL};
    } else if (strcmp(literal, "new") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = NEW, .literal = NULL};
    } else if (strcmp(literal, "if") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = IF, .literal = NULL};
    } else if (strcmp(literal, "then") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = THEN, .literal = NULL};
    } else if (strcmp(literal, "else") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = ELSE, .literal = NULL};
    } else if (strcmp(literal, "fi") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = FI, .literal = NULL};
    } else if (strcmp(literal, "while") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = WHILE, .literal = NULL};
    } else if (strcmp(literal, "loop") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = LOOP, .literal = NULL};
    } else if (strcmp(literal, "pool") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = POOL, .literal = NULL};
    } else if (strcmp(literal, "let") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = LET, .literal = NULL};
    } else if (strcmp(literal, "in") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = IN, .literal = NULL};
    } else if (strcmp(literal, "case") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = CASE, .literal = NULL};
    } else if (strcmp(literal, "of") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = OF, .literal = NULL};
    } else if (strcmp(literal, "esac") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = ESAC, .literal = NULL};
    } else if (strcmp(literal, "extern") == 0) {
        DS_FREE(NULL, literal);
        return (struct token){.type = EXTERN, .literal = NULL};
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

static char lexer_peek_char(struct lexer *l) {
    if (l->read_pos >= l->buffer_len) {
        return EOF;
    }

    return l->buffer[l->read_pos];
}

static char lexer_read_char(struct lexer *l) {
    l->ch = lexer_peek_char(l);

    l->pos = l->read_pos;
    l->read_pos += 1;

    return l->ch;
}

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

static void skip_until_newline(struct lexer *l) {
    while (l->ch != '\n' && l->ch != EOF) {
        lexer_read_char(l);
    }
}

static enum error_type skip_comment(struct lexer *l) {
    int stack = 1;
    while (stack > 0) {
        if (stack < 0) {
            return UNMATCHED_COMMENT;
        }
        if (l->ch == EOF) {
            return UNTERMINATED_COMMENT;
        }

        if (l->ch == '(') {
            char next = lexer_peek_char(l);
            if (next == '*') {
                stack += 1;
                lexer_read_char(l);
            }
        } else if (l->ch == '*') {
            char next = lexer_peek_char(l);
            if (next == ')') {
                stack -= 1;
                lexer_read_char(l);
            }
        }

        lexer_read_char(l);
    }

    return NO_ERROR;
}

static void lexer_init(struct lexer *l, char *buffer, unsigned int buffer_len) {
    l->buffer = buffer;
    l->buffer_len = buffer_len;
    l->pos = 0;
    l->read_pos = 0;
    l->ch = 0;

    lexer_read_char(l);
}

static struct token token_string_literal(struct lexer *l) {
    unsigned int position = l->pos;

    lexer_read_char(l);

    ds_string_builder builder;
    ds_string_builder_init(&builder);
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
        DS_FREE(NULL, literal);
        return (struct token){.type = ILLEGAL,
                              .literal = NULL,
                              .pos = position,
                              .error = STRING_CONSTANT_TOO_LONG};
    }

    return (struct token){
        .type = STRING_LITERAL, .literal = literal, .pos = position};
}

static struct token lexer_next_token(struct lexer *l) {
    skip_whitespaces(l);

    unsigned int position = l->pos;
    if (l->ch == EOF) {
        lexer_read_char(l);
        return (struct token){.type = END, .literal = NULL, .pos = position};
    } else if (l->ch == '{') {
        lexer_read_char(l);
        return (struct token){.type = LBRACE, .literal = NULL, .pos = position};
    } else if (l->ch == '}') {
        lexer_read_char(l);
        return (struct token){.type = RBRACE, .literal = NULL, .pos = position};
    } else if (l->ch == ';') {
        lexer_read_char(l);
        return (struct token){
            .type = SEMICOLON, .literal = NULL, .pos = position};
    } else if (l->ch == ':') {
        lexer_read_char(l);
        return (struct token){.type = COLON, .literal = NULL, .pos = position};
    } else if (l->ch == '<') {
        char next = lexer_peek_char(l);
        if (next == '-') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){
                .type = ASSIGN, .literal = NULL, .pos = position};
        } else if (next == '=') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){
                .type = LESS_THAN_EQ, .literal = NULL, .pos = position};
        } else {
            lexer_read_char(l);
            return (struct token){
                .type = LESS_THAN, .literal = NULL, .pos = position};
        }
    } else if (l->ch == '(') {
        char ch = lexer_peek_char(l);
        if (ch == '*') {
            lexer_read_char(l);
            lexer_read_char(l);
            enum error_type t = skip_comment(l);
            if (t != NO_ERROR) {
                return (struct token){.type = ILLEGAL,
                                      .literal = NULL,
                                      .pos = position,
                                      .error = t};
            }
            return lexer_next_token(l);
        } else {
            lexer_read_char(l);
            return (struct token){
                .type = LPAREN, .literal = NULL, .pos = position};
        }
    } else if (l->ch == ')') {
        lexer_read_char(l);
        return (struct token){.type = RPAREN, .literal = NULL, .pos = position};
    } else if (l->ch == ',') {
        lexer_read_char(l);
        return (struct token){.type = COMMA, .literal = NULL, .pos = position};
    } else if (l->ch == '+') {
        lexer_read_char(l);
        return (struct token){.type = PLUS, .literal = NULL, .pos = position};
    } else if (l->ch == '-') {
        char next = lexer_peek_char(l);
        if (next == '-') {
            skip_until_newline(l);
            return lexer_next_token(l);
        } else {
            lexer_read_char(l);
            return (struct token){
                .type = MINUS, .literal = NULL, .pos = position};
        }
    } else if (l->ch == '*') {
        char next = lexer_peek_char(l);
        if (next == ')') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){.type = ILLEGAL,
                                  .literal = NULL,
                                  .pos = position,
                                  .error = UNMATCHED_COMMENT};
        } else {
            lexer_read_char(l);
            return (struct token){
                .type = MULTIPLY, .literal = NULL, .pos = position};
        }
    } else if (l->ch == '/') {
        lexer_read_char(l);
        return (struct token){.type = DIVIDE, .literal = NULL, .pos = position};
    } else if (l->ch == '~') {
        lexer_read_char(l);
        return (struct token){.type = TILDE, .literal = NULL, .pos = position};
    } else if (l->ch == '=') {
        char next = lexer_peek_char(l);
        if (next == '>') {
            lexer_read_char(l);
            lexer_read_char(l);
            return (struct token){
                .type = ARROW, .literal = NULL, .pos = position};
        } else {
            lexer_read_char(l);
            return (struct token){
                .type = EQUAL, .literal = NULL, .pos = position};
        }
    } else if (l->ch == '.') {
        lexer_read_char(l);
        return (struct token){.type = DOT, .literal = NULL, .pos = position};
    } else if (l->ch == '@') {
        lexer_read_char(l);
        return (struct token){.type = AT, .literal = NULL, .pos = position};
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
        struct token t = literal_to_token(literal);
        t.pos = position;
        return t;
    } else if (isupper(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isalnum(l->ch) || l->ch == '_') {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){
            .type = CLASS_NAME, .literal = literal, .pos = position};
    } else if (isdigit(l->ch)) {
        ds_string_slice slice = {.str = l->buffer + l->pos, .len = 0};
        while (isdigit(l->ch)) {
            slice.len += 1;
            lexer_read_char(l);
        }
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){
            .type = INT_LITERAL, .literal = literal, .pos = position};
    } else {
        lexer_read_char(l);
        ds_string_slice slice = {.str = l->buffer + position, .len = 1};
        char *literal = NULL;
        ds_string_slice_to_owned(&slice, &literal);
        return (struct token){.type = ILLEGAL,
                              .literal = literal,
                              .pos = position,
                              .error = INVALID_CHAR};
    }
};

enum lexer_result lexer_tokenize(char *buffer, int length,
                                 ds_dynamic_array *tokens) {
    enum lexer_result result = LEXER_OK;

    struct lexer lexer;
    lexer_init(&lexer, (char *)buffer, length);

    unsigned int line, col;

    struct token tok;
    do {
        tok = lexer_next_token(&lexer);
        util_pos_to_lc(buffer, tok.pos, &line, &col);
        tok.line = line;
        tok.col = col;
        if (ds_dynamic_array_append(tokens, &tok) != 0) {
            DS_LOG_ERROR("Failed to append token to array");
            return_defer(LEXER_ERROR);
        }
    } while (tok.type != END);

defer:
    return result;
}
