// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_LEXER_H
#define IABB_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "token.h"

#include <stdio.h>

// An I am bored btw source code lexer.
struct iabb_lexer {
    FILE *src;
    size_t line;
    size_t col;
};

// Initializes the given lexer for lexing of the source file pointed to by
// `src`.
void iabb_lexer_init(struct iabb_lexer *lexer, FILE *src);

// Returns the next token from the given lexer.
struct iabb_token iabb_lexer_next_token(struct iabb_lexer *lexer);

#ifdef __cplusplus
}
#endif

#endif // IABB_LEXER_H
