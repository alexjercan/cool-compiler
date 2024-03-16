#ifndef LEXER_H
#define LEXER_H

#include "ds.h"

enum lexer_result {
    LEXER_OK = 0,
    LEXER_ERROR,
};

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
    EXTERN,
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

const char *error_type_to_string(enum error_type type);

struct token {
        enum token_type type;
        char *literal;
        unsigned int pos;
        enum error_type error;
        unsigned int line;
        unsigned int col;
};

enum lexer_result lexer_tokenize(char *buffer, int length,
                                 ds_dynamic_array *tokens);
void lexer_print_tokens(ds_dynamic_array *tokens);

#endif // LEXER_H
