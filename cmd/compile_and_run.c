// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "debug_handler.h"
#include "log.h"

#include "iabb/buffer.h"
#include "iabb/context.h"
#include "iabb/errors.h"
#include "iabb/targets.h"
#include "iabb/token.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__) && defined(IABB_USE_JIT)
    #define COMPILE_AND_RUN_TARGET IABB_TARGET_JIT_X86_64
#else
    #define COMPILE_AND_RUN_TARGET IABB_TARGET_BYTECODE
#endif

int compile(enum iabb_target target, FILE *src, struct iabb_buffer *dst) {
    struct iabb_token last_token;
    enum iabb_error error = iabb_compile(target, src, dst, &last_token);

    if (error != IABB_ERROR_SUCCESS) {
        LOG_ERROR(
            "compiler error: %s at line %zu, col %zu\n",
            iabb_strerror(error),
            last_token.line,
            last_token.col
        );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int run(enum iabb_target target, const struct iabb_buffer *program) {
    struct iabb_context ctx;
    iabb_context_init(&ctx, program->data, stdin, stdout, debug_handler);
    enum iabb_error error = iabb_run(target, &ctx);

    if (error != IABB_ERROR_SUCCESS) {
        LOG_ERROR(
            "run-time error: %s at %p (program + %p)\n",
            iabb_strerror(error),
            (void *) ctx.ip,
            (void *) (ctx.ip - ctx.program)
        );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int compile_and_run(const char *filename) {
    FILE *src = fopen(filename, "rbe");

    if (!src) {
        LOG_ERROR("failed to open source file: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    enum iabb_target target = COMPILE_AND_RUN_TARGET;
    bool is_jit_target = iabb_target_is_jit(target);

    struct iabb_buffer program;
    enum iabb_error error = iabb_buffer_init_maybe_jit(&program, is_jit_target);

    if (error != IABB_ERROR_SUCCESS) {
        LOG_ERROR("failed to init program buffer: %s\n", iabb_strerror(error));
        fclose(src);
        return EXIT_FAILURE;
    }

    int status = compile(target, src, &program);
    fclose(src);

    if (status == EXIT_SUCCESS) {
        status = run(target, &program);
    }

    iabb_buffer_fini_maybe_jit(&program, is_jit_target);
    return status;
}
