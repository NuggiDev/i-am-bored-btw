// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/lexer.h"

#include "iabb/token.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define IABB_LEXER_COMMENT_CHAR ';'

static bool iabb_is_space(int ch) {
    return ch == '\t' || ch == '\n' || ch == '\r' || ch == ' ';
}

static bool iabb_is_token_char(int ch) {
    return !iabb_is_space(ch) && ch != IABB_LEXER_COMMENT_CHAR && ch != EOF;
}

void iabb_lexer_init(struct iabb_lexer *lexer, FILE *src) {
    lexer->src = src;
    lexer->line = 1;
    lexer->col = 0;
}

static int iabb_lexer_next_char(struct iabb_lexer *lexer) {
    lexer->col++;
    // FIXME: on I/O error, propagate IABB_ERROR_IO to the library consumer.
    return fgetc(lexer->src);
}

static int iabb_lexer_peek_char(struct iabb_lexer *lexer) {
    int ch = fgetc(lexer->src);
    ungetc(ch, lexer->src);
    return ch;
}

static void iabb_lexer_skip_line(struct iabb_lexer *lexer) {
    int ch;

    do {
        ch = iabb_lexer_next_char(lexer);
    } while (ch != '\n' && ch != EOF);

    ungetc('\n', lexer->src);
}

static void iabb_lexer_consume_new_line(struct iabb_lexer *lexer) {
    lexer->line++;
    lexer->col = 0;
}

static enum iabb_token_type iabb_lexer_match_token(
    struct iabb_lexer *lexer,
    const char *end,
    size_t end_len,
    enum iabb_token_type type
) {
    for (size_t i = 0; i < end_len; i++) {
        if (iabb_lexer_next_char(lexer) != end[i]) {
            return IABB_TOKEN_INVALID;
        }
    }

    if (iabb_is_token_char(iabb_lexer_peek_char(lexer))) {
        return IABB_TOKEN_INVALID;
    }

    return type;
}

#define IABB_LEXER_MATCH_TOKEN(lexer, end, type) \
    iabb_lexer_match_token(lexer, end, strlen(end), type)

static enum iabb_token_type
iabb_lexer_next_token_type(struct iabb_lexer *lexer, int ch) {
    switch (ch) {
    case 'i': return IABB_LEXER_MATCH_TOKEN(lexer, "", IABB_TOKEN_I);
    case 'a': return IABB_LEXER_MATCH_TOKEN(lexer, "m", IABB_TOKEN_AM);
    case 'b':
        switch (iabb_lexer_next_char(lexer)) {
        case 'o': return IABB_LEXER_MATCH_TOKEN(lexer, "red", IABB_TOKEN_BORED);
        case 't': return IABB_LEXER_MATCH_TOKEN(lexer, "w", IABB_TOKEN_BTW);
        case 'y': return IABB_LEXER_MATCH_TOKEN(lexer, "", IABB_TOKEN_BY);
        default: return IABB_TOKEN_INVALID;
        }
    case 't': return IABB_LEXER_MATCH_TOKEN(lexer, "he", IABB_TOKEN_THE);
    case 'w': return IABB_LEXER_MATCH_TOKEN(lexer, "ay", IABB_TOKEN_WAY);
    case 's': return IABB_LEXER_MATCH_TOKEN(lexer, "leep", IABB_TOKEN_SLEEP);
    default: return IABB_TOKEN_INVALID;
    }
}

struct iabb_token iabb_lexer_next_token(struct iabb_lexer *lexer) {
    int ch;

    while ((ch = iabb_lexer_next_char(lexer)) != EOF) {
        switch (ch) {
        case IABB_LEXER_COMMENT_CHAR: iabb_lexer_skip_line(lexer); break;
        case '\n': iabb_lexer_consume_new_line(lexer); break;
        default:
            if (iabb_is_space(ch)) {
                continue;
            }

            size_t col = lexer->col;
            enum iabb_token_type type = iabb_lexer_next_token_type(lexer, ch);
            return (struct iabb_token){ type, lexer->line, col };
        }
    }

    return (struct iabb_token){ IABB_TOKEN_EOF, lexer->line, lexer->col };
}
