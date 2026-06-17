// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/targets.h"

#include "iabb/buffer.h"
#include "iabb/context.h"
#include "iabb/errors.h"
#include "iabb/targets/bytecode.h"
#include "iabb/targets/jit_x86_64.h"
#include "iabb/token.h"

#include <stdio.h>

const char *iabb_target_name(enum iabb_target target) {
    switch (target) {
    case IABB_TARGET_BYTECODE: return "bytecode";
    case IABB_TARGET_JIT_X86_64: return "JIT x86-64";
    default: return "???";
    }
}

bool iabb_target_is_jit(enum iabb_target target) {
    return target == IABB_TARGET_JIT_X86_64;
}

enum iabb_error iabb_compile(
    enum iabb_target target,
    FILE *src,
    struct iabb_buffer *dst,
    struct iabb_token *last_token_dst
) {
    switch (target) {
    case IABB_TARGET_BYTECODE:
        return iabb_compile_bytecode(src, dst, last_token_dst);
    case IABB_TARGET_JIT_X86_64:
        return iabb_compile_jit_x86_64(src, dst, last_token_dst);
    default: return IABB_ERROR_INVALID_TARGET;
    }
}

enum iabb_error iabb_run(enum iabb_target target, struct iabb_context *ctx) {
    switch (target) {
    case IABB_TARGET_BYTECODE: return iabb_run_bytecode(ctx);
    case IABB_TARGET_JIT_X86_64: return iabb_run_jit_x86_64(ctx);
    default: return IABB_ERROR_INVALID_TARGET;
    }
}
