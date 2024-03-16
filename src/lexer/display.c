#include "lexer.h"
#include <stdio.h>

void lexer_print_tokens(ds_dynamic_array *tokens) {
    for (unsigned int i = 0; i < tokens->count; i++) {
        struct token tok;
        ds_dynamic_array_get(tokens, i, &tok);

        printf("%s", token_type_to_string(tok.type));
        if (tok.type == ILLEGAL) {
            printf("(%s", error_type_to_string(tok.error));
            if (tok.literal != NULL) {
                printf(" %s", tok.literal);
            }
            printf(")");
        } else if (tok.literal != NULL) {
            printf("(%s)", tok.literal);
        }
        printf("\n");
    }
}
