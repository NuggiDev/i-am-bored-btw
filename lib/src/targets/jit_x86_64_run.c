// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/context.h"
#include "iabb/errors.h"

enum iabb_error iabb_run_jit_x86_64(struct iabb_context *ctx) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    return ((enum iabb_error(*)(struct iabb_context *)) ctx->ip)(ctx);
#pragma GCC diagnostic pop
}
