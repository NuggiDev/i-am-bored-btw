// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/context.h"

#include <string.h>

void iabb_context_init(
    struct iabb_context *ctx,
    const uint8_t *program,
    FILE *in,
    FILE *out,
    void (*debug_handler)(struct iabb_context *)
) {
    ctx->ip = program;
    ctx->dp = ctx->memory;
    ctx->in = in;
    ctx->out = out;
    ctx->debug_handler = debug_handler;
    ctx->program = program;
    memset(ctx->memory, 0, IABB_CONTEXT_MEMORY_SIZE);
}
