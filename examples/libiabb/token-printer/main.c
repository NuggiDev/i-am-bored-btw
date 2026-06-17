// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/lexer.h"
#include "iabb/token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int print_tokens(const char *filename) {
    FILE *src = fopen(filename, "rbe");

    if (!src) {
        perror("error: failed to open source file");
        return EXIT_FAILURE;
    }

    struct iabb_lexer lexer;
    iabb_lexer_init(&lexer, src);

    size_t last_line = 1;
    size_t last_end_col = 1;
    struct iabb_token token;

    while ((token = iabb_lexer_next_token(&lexer)).type != IABB_TOKEN_EOF) {
        if (token.type == IABB_TOKEN_INVALID) {
            fprintf(
                stderr,
                "error: invalid token at line %zu, col %zu\n",
                token.line,
                token.col
            );
            fclose(src);
            return EXIT_FAILURE;
        }

        while (token.line > last_line) {
            last_end_col = 1;
            putchar('\n');
            last_line++;
        }

        const char *str = iabb_token_type_name(token.type);
        printf("%*s%s", (int) (token.col - last_end_col), "", str);
        last_end_col = token.col + strlen(str);
    }

    fclose(src);
    putchar('\n');
    return EXIT_SUCCESS;
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    return print_tokens(argv[1]);
}
