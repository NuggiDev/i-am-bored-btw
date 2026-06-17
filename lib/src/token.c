// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/token.h"

const char *iabb_token_type_name(enum iabb_token_type type) {
    switch (type) {
    case IABB_TOKEN_EOF: return "EOF";
    case IABB_TOKEN_I: return "i";
    case IABB_TOKEN_AM: return "am";
    case IABB_TOKEN_BORED: return "bored";
    case IABB_TOKEN_BTW: return "btw";
    case IABB_TOKEN_BY: return "by";
    case IABB_TOKEN_THE: return "the";
    case IABB_TOKEN_WAY: return "way";
    case IABB_TOKEN_SLEEP: return "sleep";
    default: return "???";
    }
}
