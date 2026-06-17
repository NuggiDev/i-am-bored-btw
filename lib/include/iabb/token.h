// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_TOKEN_H
#define IABB_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// A type of I am bored btw source code token.
enum iabb_token_type {
    // End of file.
    IABB_TOKEN_EOF = -1,

    // The `i` keyword.
    IABB_TOKEN_I,
    // The `am` keyword.
    IABB_TOKEN_AM,
    // The `bored` keyword.
    IABB_TOKEN_BORED,
    // The `btw` keyword.
    IABB_TOKEN_BTW,
    // The `by` keyword.
    IABB_TOKEN_BY,
    // The `the` keyword.
    IABB_TOKEN_THE,
    // The `way` keyword.
    IABB_TOKEN_WAY,
    // The `sleep` keyword.
    IABB_TOKEN_SLEEP,

    // Invalid token type.
    IABB_TOKEN_INVALID,
};

// Returns the name of the given token type as a string.
const char *iabb_token_type_name(enum iabb_token_type type);

// An I am bored btw source code token.
struct iabb_token {
    enum iabb_token_type type;
    size_t line;
    size_t col;
};

#ifdef __cplusplus
}
#endif

#endif
