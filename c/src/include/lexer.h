#ifndef LEXER_H
#define LEXER_H

#include "ds.h"

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
    STRING_CONTAINS_EOF,
    UNMATCHED_COMMENT,
    UNTERMINATED_COMMENT,
};

struct token {
        enum token_type type;
        char *literal;
        unsigned int pos;
        enum error_type error;
};

int lexer_tokenize(const char *filename, char *buffer, int length,
                   ds_dynamic_array *tokens);

#endif // LEXER_H
