// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_TARGETS_JIT_X86_64_H
#define IABB_TARGETS_JIT_X86_64_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../buffer.h"
#include "../context.h"
#include "../errors.h"
#include "../token.h"

#include <stdint.h>
#include <stdio.h>

// JIT-compiles the source file pointed to by `src` into x86-64 code
// following the System V AMD64/x86-64 ABI's calling convention to write to the
// buffer pointed to by `dst` and writes the last token processed at the
// location pointed to by `last_token_dst`. Returns the error that occurred in
// the process.
enum iabb_error iabb_compile_jit_x86_64(
    FILE *src,
    struct iabb_buffer *dst,
    struct iabb_token *last_token_dst
);

// Runs the JIT-compiled x86-64 program from the context pointed to by `ctx`.
// Returns the error that occurred in the process.
//
// The `ip` and `dp` members of the context are only updated when calling the
// debugging event handler.
enum iabb_error iabb_run_jit_x86_64(struct iabb_context *ctx);

#ifdef __cplusplus
}
#endif

#endif // IABB_TARGETS_JIT_X86_64_H
