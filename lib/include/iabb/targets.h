// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_TARGETS_H
#define IABB_TARGETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"
#include "context.h"
#include "errors.h"
#include "token.h"

#include <stdbool.h>
#include <stdio.h>

// An I use Arch btw compilation target.
enum iabb_target {
    // I use Arch btw bytecode.
    IABB_TARGET_BYTECODE,
    // JIT-compiled x86-64 code following the System V AMD64/x86-64 ABI's
    // calling convention.
    IABB_TARGET_JIT_X86_64,
};

// Returns the name of the given target as a string.
const char *iabb_target_name(enum iabb_target target);

// Returns true if the given target is a JIT compilation target, otherwise
// false.
bool iabb_target_is_jit(enum iabb_target target);

// Compiles for the given target the source file pointed to by `src` into
// code to write to the buffer pointed to by `dst` and writes the last token
// processed at the location pointed to by `last_token_dst`. Returns the error
// that occurred in the process.
//
// The buffer must have been initialized with `iabb_buffer_init_jit()` if the
// target is a JIT compilation target, otherwise with `iabb_buffer_init()`.
enum iabb_error iabb_compile(
    enum iabb_target target,
    FILE *src,
    struct iabb_buffer *dst,
    struct iabb_token *last_token_dst
);

// Runs the program compiled for the given target from the context pointed to
// by `ctx`.
enum iabb_error iabb_run(enum iabb_target target, struct iabb_context *ctx);

#ifdef __cplusplus
}
#endif

#endif // IABB_TARGETS_H
