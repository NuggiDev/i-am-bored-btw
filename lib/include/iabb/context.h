// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_CONTEXT_H
#define IABB_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

// The size of the working memory stored in an I use Arch btw program context.
#define IABB_CONTEXT_MEMORY_SIZE (1U << 16U)

// An I use Arch btw program context.
struct iabb_context {
    const uint8_t *ip;
    uint8_t *dp;
    FILE *in;
    FILE *out;
    void (*debug_handler)(struct iabb_context *);
    const uint8_t *program;
    uint8_t memory[IABB_CONTEXT_MEMORY_SIZE];
};

// Initializes the given context for execution of the code stored at `program`,
// with the files pointed to by `in` and `out` as input and output files
// respectively, and the function pointed to by `debug_handler` as debugging
// event handler.
void iabb_context_init(
    struct iabb_context *ctx,
    const uint8_t *program,
    FILE *in,
    FILE *out,
    void (*debug_handler)(struct iabb_context *)
);

#ifdef __cplusplus
}
#endif

#endif // IABB_CONTEXT_H
