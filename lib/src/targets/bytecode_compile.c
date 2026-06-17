// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/buffer.h"
#include "iabb/errors.h"
#include "iabb/lexer.h"
#include "iabb/targets/bytecode.h"
#include "iabb/token.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DEFINE_IABB_BYTECODE_INSTR_TYPE_WITH_OPERAND(name, operand_type) \
    typedef uint8_t name[1 + sizeof(operand_type)];                      \
                                                                         \
    void name##_init(name instr, uint8_t op, operand_type operand) {     \
        instr[0] = op;                                                   \
        memcpy(&instr[1], &operand, sizeof(operand));                    \
    }

DEFINE_IABB_BYTECODE_INSTR_TYPE_WITH_OPERAND(iabb_bytecode_instr_u8, uint8_t)
DEFINE_IABB_BYTECODE_INSTR_TYPE_WITH_OPERAND(iabb_bytecode_instr_u16, uint16_t)
DEFINE_IABB_BYTECODE_INSTR_TYPE_WITH_OPERAND(iabb_bytecode_instr_size, size_t)

struct iabb_bytecode_compiler {
    struct iabb_lexer lexer;
    struct iabb_token token;
    struct iabb_buffer loop_stack;
    struct iabb_buffer *dst;
};

static enum iabb_error iabb_bytecode_compiler_init(
    struct iabb_bytecode_compiler *compiler,
    FILE *src,
    struct iabb_buffer *dst
) {
    compiler->dst = dst;
    iabb_lexer_init(&compiler->lexer, src);
    compiler->token = iabb_lexer_next_token(&compiler->lexer);
    return iabb_buffer_init(&compiler->loop_stack);
}

static void iabb_bytecode_compiler_fini(struct iabb_bytecode_compiler *compiler
) {
    iabb_buffer_fini(&compiler->loop_stack);
}

static enum iabb_error
iabb_bytecode_emit_additive_p(struct iabb_bytecode_compiler *compiler) {
    enum iabb_token_type token_type = compiler->token.type;
    struct iabb_lexer *lexer = &compiler->lexer;

    uint8_t op;

    switch (token_type) {
    case IABB_TOKEN_I: op = IABB_BYTECODE_OP_ADDP; break;
    case IABB_TOKEN_AM: op = IABB_BYTECODE_OP_SUBP; break;
    default: return IABB_ERROR_COMPILER_INTERNAL;
    }

    uint16_t operand = 1;
    struct iabb_token next_token;

    while ((next_token = iabb_lexer_next_token(lexer)).type == token_type) {
        if (operand == UINT16_MAX) {
            compiler->token = next_token;
            return IABB_ERROR_DP_OUT_OF_BOUNDS;
        }

        operand++;
    }

    compiler->token = next_token;

    iabb_bytecode_instr_u16 instr;
    iabb_bytecode_instr_u16_init(instr, op, operand);
    return IABB_BUFFER_WRITE(compiler->dst, instr);
}

static enum iabb_error
iabb_bytecode_emit_additive_v(struct iabb_bytecode_compiler *compiler) {
    enum iabb_token_type token_type = compiler->token.type;
    struct iabb_lexer *lexer = &compiler->lexer;

    uint8_t op;

    switch (token_type) {
    case IABB_TOKEN_BORED: op = IABB_BYTECODE_OP_ADDV; break;
    case IABB_TOKEN_BTW: op = IABB_BYTECODE_OP_SUBV; break;
    default: return IABB_ERROR_COMPILER_INTERNAL;
    }

    uint8_t operand = 1;
    struct iabb_token next_token;

    while ((next_token = iabb_lexer_next_token(lexer)).type == token_type) {
        operand++;
    }

    compiler->token = next_token;

    if (operand == 0) {
        return IABB_ERROR_SUCCESS;
    }

    iabb_bytecode_instr_u8 instr;
    iabb_bytecode_instr_u8_init(instr, op, operand);
    return IABB_BUFFER_WRITE(compiler->dst, instr);
}

static enum iabb_error
iabb_bytecode_emit_no_operand(struct iabb_bytecode_compiler *compiler) {
    uint8_t op;

    switch (compiler->token.type) {
    case IABB_TOKEN_BY: op = IABB_BYTECODE_OP_WRITE; break;
    case IABB_TOKEN_THE: op = IABB_BYTECODE_OP_READ; break;
    default: return IABB_ERROR_COMPILER_INTERNAL;
    }

    return iabb_buffer_write_u8(compiler->dst, op);
}

static enum iabb_error
iabb_bytecode_begin_loop(struct iabb_bytecode_compiler *compiler) {
    compiler->dst->size += sizeof(iabb_bytecode_instr_size);
    return iabb_buffer_write_size(&compiler->loop_stack, compiler->dst->size);
}

static enum iabb_error
iabb_bytecode_end_loop(struct iabb_bytecode_compiler *compiler) {
    if (compiler->loop_stack.size == 0) {
        return IABB_ERROR_COMPILER_UNEXPECTED_LOOP_END;
    }

    size_t loop_start = iabb_buffer_pop_size(&compiler->loop_stack);
    iabb_bytecode_instr_size instr;
    iabb_bytecode_instr_size_init(instr, IABB_BYTECODE_OP_JMPNZ, loop_start);
    enum iabb_error error = IABB_BUFFER_WRITE(compiler->dst, instr);

    size_t loop_end = compiler->dst->size;
    iabb_bytecode_instr_size_init(instr, IABB_BYTECODE_OP_JMPZ, loop_end);
    size_t jmpz_instr_offset = loop_start - sizeof(iabb_bytecode_instr_size);
    memcpy(&compiler->dst->data[jmpz_instr_offset], instr, sizeof(instr));

    return error;
}

static enum iabb_error
iabb_bytecode_emit(struct iabb_bytecode_compiler *compiler) {
    enum iabb_error error;

    switch (compiler->token.type) {
    case IABB_TOKEN_I:
    case IABB_TOKEN_AM: return iabb_bytecode_emit_additive_p(compiler);
    case IABB_TOKEN_BORED:
    case IABB_TOKEN_BTW: return iabb_bytecode_emit_additive_v(compiler);
    case IABB_TOKEN_BY:
    case IABB_TOKEN_THE:
        error = iabb_bytecode_emit_no_operand(compiler);
        break;
    case IABB_TOKEN_WAY: error = iabb_bytecode_begin_loop(compiler); break;
    case IABB_TOKEN_SLEEP: error = iabb_bytecode_end_loop(compiler); break;
    default: return IABB_ERROR_COMPILER_INVALID_TOKEN;
    }

    if (error == IABB_ERROR_SUCCESS) {
        compiler->token = iabb_lexer_next_token(&compiler->lexer);
    }

    return error;
}

enum iabb_error iabb_compile_bytecode(
    FILE *src,
    struct iabb_buffer *dst,
    struct iabb_token *last_token_dst
) {
    struct iabb_bytecode_compiler compiler;
    enum iabb_error error = iabb_bytecode_compiler_init(&compiler, src, dst);

    if (error != IABB_ERROR_SUCCESS) {
        *last_token_dst = compiler.token;
        return error;
    }

    while (compiler.token.type != IABB_TOKEN_EOF) {
        error = iabb_bytecode_emit(&compiler);

        if (error != IABB_ERROR_SUCCESS) {
            *last_token_dst = compiler.token;
            iabb_bytecode_compiler_fini(&compiler);
            return error;
        }
    }

    *last_token_dst = compiler.token;
    size_t loop_stack_size = compiler.loop_stack.size;
    iabb_bytecode_compiler_fini(&compiler);

    if (loop_stack_size != 0) {
        return IABB_ERROR_COMPILER_UNCLOSED_LOOPS;
    }

    uint8_t ret[] = { IABB_BYTECODE_OP_RET };
    return IABB_BUFFER_WRITE(dst, ret);
}
