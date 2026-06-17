// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/context.h"
#include "iabb/errors.h"
#include "iabb/targets/bytecode.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static enum iabb_error iabb_bytecode_run_addp(struct iabb_context *ctx) {
    uint16_t operand;
    memcpy(&operand, ctx->ip, sizeof(operand));

    if (ctx->dp - ctx->memory >= IABB_CONTEXT_MEMORY_SIZE - operand) {
        return IABB_ERROR_DP_OUT_OF_BOUNDS;
    }

    ctx->ip += sizeof(operand);
    ctx->dp += operand;
    return IABB_ERROR_SUCCESS;
}

static enum iabb_error iabb_bytecode_run_subp(struct iabb_context *ctx) {
    uint16_t operand;
    memcpy(&operand, ctx->ip, sizeof(operand));

    if (ctx->dp - ctx->memory < operand) {
        return IABB_ERROR_DP_OUT_OF_BOUNDS;
    }

    ctx->ip += sizeof(operand);
    ctx->dp -= operand;
    return IABB_ERROR_SUCCESS;
}

static enum iabb_error iabb_bytecode_run_write(struct iabb_context *ctx) {
    int result = fputc(*ctx->dp, ctx->out);

    if (result == EOF) {
        return IABB_ERROR_IO;
    }

    return IABB_ERROR_SUCCESS;
}

static enum iabb_error iabb_bytecode_run_read(struct iabb_context *ctx) {
    int result = fgetc(ctx->in);

    if (result == EOF) {
        if (ferror(ctx->in)) {
            return IABB_ERROR_IO;
        }

        return IABB_ERROR_RUNTIME_END_OF_INPUT_FILE;
    }

    *ctx->dp = result;
    return IABB_ERROR_SUCCESS;
}

static void iabb_bytecode_run_jmpz(struct iabb_context *ctx) {
    size_t offset;

    if (*ctx->dp != 0) {
        ctx->ip += sizeof(offset);
        return;
    }

    memcpy(&offset, ctx->ip, sizeof(offset));
    ctx->ip = ctx->program + offset;
}

static void iabb_bytecode_run_jmpnz(struct iabb_context *ctx) {
    size_t offset;

    if (*ctx->dp == 0) {
        ctx->ip += sizeof(offset);
        return;
    }

    memcpy(&offset, ctx->ip, sizeof(offset));
    ctx->ip = ctx->program + offset;
}

enum iabb_error iabb_run_bytecode(struct iabb_context *ctx) {
    uint8_t op;

    while ((op = *ctx->ip++) != IABB_BYTECODE_OP_RET) {
        enum iabb_error err = IABB_ERROR_SUCCESS;

        switch (op) {
        case IABB_BYTECODE_OP_ADDP: err = iabb_bytecode_run_addp(ctx); break;
        case IABB_BYTECODE_OP_SUBP: err = iabb_bytecode_run_subp(ctx); break;
        case IABB_BYTECODE_OP_ADDV: *ctx->dp += *ctx->ip++; break;
        case IABB_BYTECODE_OP_SUBV: *ctx->dp -= *ctx->ip++; break;
        case IABB_BYTECODE_OP_WRITE: err = iabb_bytecode_run_write(ctx); break;
        case IABB_BYTECODE_OP_READ: err = iabb_bytecode_run_read(ctx); break;
        case IABB_BYTECODE_OP_JMPZ: iabb_bytecode_run_jmpz(ctx); break;
        case IABB_BYTECODE_OP_JMPNZ: iabb_bytecode_run_jmpnz(ctx); break;
        case IABB_BYTECODE_OP_DEBUG: ctx->debug_handler(ctx); break;
        default: return IABB_ERROR_BYTECODE_INVALID_OP;
        }

        if (err != IABB_ERROR_SUCCESS) {
            return err;
        }
    }

    return IABB_ERROR_SUCCESS;
}
