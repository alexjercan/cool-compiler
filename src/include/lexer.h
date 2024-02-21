#ifndef LEXER_H
#define LEXER_H

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

const char *token_type_to_string(enum token_type type);

enum error_type {
    NO_ERROR,
    INVALID_CHAR,
    STRING_CONSTANT_TOO_LONG,
    STRING_CONTAINS_NULL,
    STRING_UNTERMINATED,
    STRING_CONTAINS_EOF
};

const char *error_type_to_string(enum error_type type);

struct token {
        enum token_type type;
        char *literal;
        unsigned int pos;
        enum error_type error;
};

struct lexer {
        const char *filename;
        char *buffer;
        unsigned int buffer_len;
        unsigned int pos;
        unsigned int read_pos;
        char ch;
};

void lexer_init(struct lexer *l, const char *filename, char *buffer,
                unsigned int buffer_len);
struct token lexer_next_token(struct lexer *l);
void lexer_print_error(struct lexer *l, struct token *t);

#endif // LEXER_H
