// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/buffer.h"
#include "iabb/context.h"
#include "iabb/errors.h"
#include "iabb/lexer.h"
#include "iabb/token.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Transforms a 2-byte opcode into a big-endian list of bytes.
#define IABB_OP2_TO_BYTES(op2) (((op2) >> 8) & 0xFF), ((op2) &0xFF)

// Transforms a dword into a little-endian list of bytes.
#define IABB_DWORD_TO_BYTES(dword)                                      \
    ((dword) &0xFF), (((dword) >> 8) & 0xFF), (((dword) >> 16) & 0xFF), \
        (((dword) >> 24) & 0xFF)

// Transforms a qword into a little-endian list of bytes.
#define IABB_QWORD_TO_BYTES(dword)                                      \
    ((dword) &0xFF), (((dword) >> 8) & 0xFF), (((dword) >> 16) & 0xFF), \
        (((dword) >> 24) & 0xFF), (((dword) >> 32) & 0xFF),             \
        (((dword) >> 40) & 0xFF), (((dword) >> 48) & 0xFF),             \
        (((dword) >> 56) & 0xFF)

// REX prefixes.
enum {
    IABB_REX_W = 0x48,
    IABB_REX_R = 0x44,
    IABB_REX_B = 0x41,
};

// Instruction primary opcodes.
enum {
    IABB_OP_ADD_RM8_IMM8 = 0x80 /* /0 ib */,
    IABB_OP_ADD_RM64_IMM32 = /* REX.W */ 0x81 /* /0 id */,
    IABB_OP_CALL_REL32 = 0xE8 /* cd */,
    IABB_OP_CALL_RM64 = 0xFF /* /2 */,
    IABB_OP_CMP_EAX_IMM32 = 0x3D /* id */,
    IABB_OP_CMP_RAX_IMM32 = /* REX.W */ 0x3D /* id */,
    IABB_OP_CMP_RM8_IMM8 = 0x80 /* /7 ib */,
    IABB_OP_JMP_REL8 = 0xEB /* cb */,
    IABB_OP_JMP_RM64 = 0xFF /* /4 */,
    IABB_OP_LEA_R64_M = /* REX.W */ 0x8D /* /r */,
    IABB_OP_MOV_RM8_R8 = 0x88 /* /r */,
    IABB_OP_MOV_RM64_R64 = /* REX.W */ 0x89 /* /r */,
    IABB_OP_MOV_R64_RM64 = /* REX.W */ 0x8B /* /r */,
    IABB_OP_MOV_R32_IMM32 = 0xB8 /* +rd id */,
    IABB_OP_MOV_R64_IMM64 = /* REX.W */ 0xB8 /* +rq iq */,
    IABB_OP_POP_R64 = 0x58 /* +rq */,
    IABB_OP_POP_RM64 = 0x8F /* /0 */,
    IABB_OP_PUSH_R64 = 0x50 /* +rq */,
    IABB_OP_RET_NEAR = 0xC3,
    IABB_OP_SUB_RM8_IMM8 = 0x80 /* /5 ib */,
    IABB_OP_SUB_RM64_IMM32 = /* REX.W */ 0x81 /* /5 id */,
    IABB_OP_SUB_RM64_R64 = /* REX.W */ 0x29 /* /r */,
    IABB_OP_XOR_RM32_R32 = 0x31 /* /r */,

    IABB_OP2_JAE_REL32 = 0x0F83 /* cd */,
    IABB_OP2_JB_REL32 = 0x0F82 /* cd */,
    IABB_OP2_JE_REL32 = 0x0F84 /* cd */,
    IABB_OP2_JNE_REL8 = 0x0F75 /* cb */,
    IABB_OP2_JNE_REL32 = 0x0F85 /* cd */,
    IABB_OP2_MOVZX_R32_RM8 = 0x0FB6 /* /r */,
};

// Register IDs.
enum {
    IABB_REG_AL = 0x0,

    IABB_REG_EAX = 0x0,
    IABB_REG_EDI = 0x7,

    IABB_REG_RAX = 0x0,
    IABB_REG_RBX = 0x3,
    IABB_REG_RSI = 0x6,
    IABB_REG_RDI = 0x7,
    IABB_REG_R12 = 0x4,
    IABB_REG_R13 = 0x5,
    IABB_REG_R14 = 0x6,
    IABB_REG_R15 = 0x7,
};

// ModR/M byte `mod` field values.
enum {
    IABB_MODRM_MOD_DISP0 = 0x0 << 6,
    IABB_MODRM_MOD_DISP8 = 0x1 << 6,
    IABB_MODRM_MOD_DIRECT = 0x3 << 6,
};

// ModR/M byte `reg` field values.
enum {
    IABB_MODRM_REG_OP_ADD_RM_IMM = 0x0 << 3,
    IABB_MODRM_REG_OP_CALL_RM = 0x2 << 3,
    IABB_MODRM_REG_OP_CMP_RM_IMM = 0x7 << 3,
    IABB_MODRM_REG_OP_SUB_RM_IMM = 0x5 << 3,
    IABB_MODRM_REG_OP_JMP_RM = 0x4 << 3,
    IABB_MODRM_REG_OP_POP_RM = 0x0 << 3,

    IABB_MODRM_REG_AL = IABB_REG_AL << 3,

    IABB_MODRM_REG_EAX = IABB_REG_EAX << 3,
    IABB_MODRM_REG_EDI = IABB_REG_EDI << 3,

    IABB_MODRM_REG_RBX = IABB_REG_RBX << 3,
    IABB_MODRM_REG_RSI = IABB_REG_RSI << 3,
    IABB_MODRM_REG_RDI = IABB_REG_RDI << 3,
    IABB_MODRM_REG_R14 = IABB_REG_R14 << 3,
    IABB_MODRM_REG_R15 = IABB_REG_R15 << 3,
};

// ModR/M byte `rm` field values.
enum {
    IABB_MODRM_RM_EAX = IABB_REG_EAX,
    IABB_MODRM_RM_RAX = IABB_REG_RAX,
    IABB_MODRM_RM_RBX = IABB_REG_RBX,
    IABB_MODRM_RM_RDI = IABB_REG_RDI,
    IABB_MODRM_RM_R12 = IABB_REG_R12,
    IABB_MODRM_RM_R13 = IABB_REG_R13,
    IABB_MODRM_RM_R14 = IABB_REG_R14,
};

enum iabb_jit_x86_64_jump_target {
    IABB_JUMP_RET_ERROR_DP_OUT_OF_BOUNDS,
    IABB_JUMP_RET_ERROR_IO,
    IABB_JUMP_HANDLE_FGETC_EOF,

    IABB_NUM_JUMP_TARGETS,
};

struct iabb_jit_x86_64_jump {
    size_t from;
    enum iabb_jit_x86_64_jump_target to;
};

struct iabb_jit_x86_64_compiler {
    struct iabb_lexer lexer;
    struct iabb_token token;
    struct iabb_buffer jumps;
    struct iabb_buffer loop_stack;
    struct iabb_buffer *dst;
};

static enum iabb_error iabb_jit_x86_64_compiler_init(
    struct iabb_jit_x86_64_compiler *compiler,
    FILE *src,
    struct iabb_buffer *dst
) {
    compiler->dst = dst;
    iabb_lexer_init(&compiler->lexer, src);
    compiler->token = iabb_lexer_next_token(&compiler->lexer);
    enum iabb_error error = iabb_buffer_init(&compiler->jumps);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    return iabb_buffer_init(&compiler->loop_stack);
}

static void
iabb_jit_x86_64_compiler_fini(struct iabb_jit_x86_64_compiler *compiler) {
    iabb_buffer_fini(&compiler->jumps);
    iabb_buffer_fini(&compiler->loop_stack);
}

static enum iabb_error iabb_jit_x86_64_emit_header(struct iabb_buffer *dst) {
    uint8_t instrs[] = {
        // push rbx
        IABB_OP_PUSH_R64 + IABB_REG_RBX,
        // push r12
        IABB_REX_B,
        IABB_OP_PUSH_R64 + IABB_REG_R12,
        // push r13
        IABB_REX_B,
        IABB_OP_PUSH_R64 + IABB_REG_R13,
        // push r14
        IABB_REX_B,
        IABB_OP_PUSH_R64 + IABB_REG_R14,
        // push r15
        IABB_REX_B,
        IABB_OP_PUSH_R64 + IABB_REG_R15,
        // mov rbx, rdi
        IABB_REX_W,
        IABB_OP_MOV_RM64_R64,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_RDI | IABB_MODRM_RM_RBX,
        // mov r12, fgetc
        IABB_REX_W | IABB_REX_B,
        IABB_OP_MOV_R64_IMM64 + IABB_REG_R12,
        IABB_QWORD_TO_BYTES((int64_t) fgetc),
        // mov r13, fputc
        IABB_REX_W | IABB_REX_B,
        IABB_OP_MOV_R64_IMM64 + IABB_REG_R13,
        IABB_QWORD_TO_BYTES((int64_t) fputc),
        // mov r14, QWORD PTR [rdi + offsetof(struct iabb_context, dp)]
        IABB_REX_W | IABB_REX_R,
        IABB_OP_MOV_R64_RM64,
        IABB_MODRM_MOD_DISP8 | IABB_MODRM_REG_R14 | IABB_MODRM_RM_RDI,
        offsetof(struct iabb_context, dp),
        // lea r15, QWORD PTR [rdi + offsetof(struct iabb_context, memory)]
        IABB_REX_W | IABB_REX_R,
        IABB_OP_LEA_R64_M,
        IABB_MODRM_MOD_DISP8 | IABB_MODRM_REG_R15 | IABB_MODRM_RM_RDI,
        offsetof(struct iabb_context, memory),
    };
    return IABB_BUFFER_WRITE_JIT(dst, instrs);
}

static enum iabb_error
iabb_jit_x86_64_set_rel8(struct iabb_buffer *dst, size_t from, size_t to) {
    ssize_t rel8 = (ssize_t) (to - from);

    if (rel8 < INT8_MIN || rel8 > INT8_MAX) {
        return IABB_ERROR_JIT_JUMP_TOO_LARGE;
    }

    *(int8_t *) &dst->data[from - sizeof(int8_t)] = (int8_t) rel8;
    return IABB_ERROR_SUCCESS;
}

static enum iabb_error
iabb_jit_x86_64_set_rel32(struct iabb_buffer *dst, size_t from, size_t to) {
    ssize_t rel32 = (ssize_t) (to - from);

    if (rel32 < INT32_MIN || rel32 > INT32_MAX) {
        return IABB_ERROR_JIT_JUMP_TOO_LARGE;
    }

    uint8_t *rel32_dst = &dst->data[from - sizeof(int32_t)];
    rel32_dst[0] = rel32 & 0xFF;
    rel32_dst[1] = (rel32 >> 8) & 0xFF;
    rel32_dst[2] = (rel32 >> 16) & 0xFF;
    rel32_dst[3] = (rel32 >> 24) & 0xFF;
    return IABB_ERROR_SUCCESS;
}

static enum iabb_error iabb_jit_x86_64_emit_ret_error_dp_out_of_bounds(
    size_t exit_offset,
    struct iabb_buffer *dst
) {
    uint8_t instrs[] = {
        // mov eax, IABB_ERROR_DP_OUT_OF_BOUNDS
        IABB_OP_MOV_R32_IMM32 + IABB_REG_EAX,
        IABB_DWORD_TO_BYTES(IABB_ERROR_DP_OUT_OF_BOUNDS),
        // jmp .exit ; Offset written later.
        IABB_OP_JMP_REL8,
        0,
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(dst, instrs);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    return iabb_jit_x86_64_set_rel8(dst, dst->size, exit_offset);
}

static enum iabb_error
iabb_jit_x86_64_emit_ret_error_io(size_t exit_offset, struct iabb_buffer *dst) {
    uint8_t instrs[] = {
        // mov eax, IABB_ERROR_IO
        IABB_OP_MOV_R32_IMM32 + IABB_REG_EAX,
        IABB_DWORD_TO_BYTES(IABB_ERROR_IO),
        // jmp .exit ; Offset written later.
        IABB_OP_JMP_REL8,
        0,
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(dst, instrs);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    return iabb_jit_x86_64_set_rel8(dst, dst->size, exit_offset);
}

static enum iabb_error iabb_jit_x86_64_emit_handle_fgetc_error(
    size_t exit_offset,
    struct iabb_buffer *dst
) {
    uint8_t ret_error_io_if_ferror[] = {
        // mov rdi, QWORD PTR [rbx + offsetof(struct iabb_context, in)]
        IABB_REX_W,
        IABB_OP_MOV_R64_RM64,
        IABB_MODRM_MOD_DISP8 | IABB_MODRM_REG_RDI | IABB_MODRM_RM_RBX,
        offsetof(struct iabb_context, in),
        // mov rax, ferror
        IABB_REX_W,
        IABB_OP_MOV_R64_IMM64 + IABB_REG_EAX,
        IABB_QWORD_TO_BYTES((int64_t) ferror),
        // call rax
        IABB_OP_CALL_RM64,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_OP_CALL_RM | IABB_MODRM_RM_RAX,
        // cmp eax, 0
        IABB_OP_CMP_EAX_IMM32,
        IABB_DWORD_TO_BYTES(0),
        // mov eax, IABB_ERROR_IO
        IABB_OP_MOV_R32_IMM32 + IABB_REG_EAX,
        IABB_DWORD_TO_BYTES(IABB_ERROR_IO),
        // jne .exit ; Offset written later.
        IABB_OP2_TO_BYTES(IABB_OP2_JNE_REL8),
        0,
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(dst, ret_error_io_if_ferror);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    error = iabb_jit_x86_64_set_rel8(dst, dst->size, exit_offset);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    uint8_t ret_error_runtime_end_of_input_file[] = {
        // mov eax, IABB_ERROR_RUNTIME_END_OF_INPUT_FILE
        IABB_OP_MOV_R32_IMM32 + IABB_REG_EAX,
        IABB_DWORD_TO_BYTES(IABB_ERROR_RUNTIME_END_OF_INPUT_FILE),
        // jmp .exit
        IABB_OP_JMP_REL8,
        0,
    };
    error = IABB_BUFFER_WRITE_JIT(dst, ret_error_runtime_end_of_input_file);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    return iabb_jit_x86_64_set_rel8(dst, dst->size, exit_offset);
}

static enum iabb_error iabb_jit_x86_64_emit_jump_target(
    enum iabb_jit_x86_64_jump_target target,
    size_t exit_offset,
    struct iabb_buffer *dst
) {
    switch (target) {
    case IABB_JUMP_RET_ERROR_DP_OUT_OF_BOUNDS:
        return iabb_jit_x86_64_emit_ret_error_dp_out_of_bounds(
            exit_offset,
            dst
        );
    case IABB_JUMP_RET_ERROR_IO:
        return iabb_jit_x86_64_emit_ret_error_io(exit_offset, dst);
    case IABB_JUMP_HANDLE_FGETC_EOF:
        return iabb_jit_x86_64_emit_handle_fgetc_error(exit_offset, dst);
    default: return IABB_ERROR_COMPILER_INTERNAL;
    }
}

static enum iabb_error iabb_jit_x86_64_emit_footer(
    struct iabb_buffer *jumps,
    struct iabb_buffer *dst
) {
    uint8_t set_error_success[] = {
        // xor eax, eax
        IABB_OP_XOR_RM32_R32,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_EAX | IABB_MODRM_RM_EAX,
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(dst, set_error_success);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    size_t exit_offset = dst->size;
    uint8_t exit[] = {
        // pop r15
        IABB_REX_B,
        IABB_OP_POP_R64 + IABB_REG_R15,
        // pop r14
        IABB_REX_B,
        IABB_OP_POP_R64 + IABB_REG_R14,
        // pop r13
        IABB_REX_B,
        IABB_OP_POP_R64 + IABB_REG_R13,
        // pop r12
        IABB_REX_B,
        IABB_OP_POP_R64 + IABB_REG_R12,
        // pop rbx
        IABB_OP_POP_R64 + IABB_REG_RBX,
        // ret
        IABB_OP_RET_NEAR,
    };
    error = IABB_BUFFER_WRITE_JIT(dst, exit);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    size_t jump_target_offsets[IABB_NUM_JUMP_TARGETS] = { 0 };
    size_t jump_struct_size = sizeof(struct iabb_jit_x86_64_jump);

    for (size_t i = 0; i < jumps->size; i += jump_struct_size) {
        struct iabb_jit_x86_64_jump *jump =
            (struct iabb_jit_x86_64_jump *) &jumps->data[i];

        if (jump_target_offsets[jump->to] == 0) {
            jump_target_offsets[jump->to] = dst->size;
            iabb_jit_x86_64_emit_jump_target(jump->to, exit_offset, dst);
        }

        error = iabb_jit_x86_64_set_rel32(
            dst,
            jump->from,
            jump_target_offsets[jump->to]
        );

        if (error != IABB_ERROR_SUCCESS) {
            return error;
        }
    }

    return IABB_ERROR_SUCCESS;
}

static enum iabb_error
iabb_jit_x86_64_emit_additive_p(struct iabb_jit_x86_64_compiler *compiler) {
    struct iabb_lexer *lexer = &compiler->lexer;
    enum iabb_token_type token_type = compiler->token.type;

    uint16_t operand = 1;
    struct iabb_token next_token;

    while ((next_token = iabb_lexer_next_token(lexer)).type == token_type) {
        if (operand == UINT16_MAX) {
            compiler->token = next_token;
            return IABB_ERROR_DP_OUT_OF_BOUNDS;
        }

        operand++;
    }

    uint16_t jcc_op;
    uint8_t op;
    uint8_t modrm_reg;
    int32_t cmp_bound;

    switch (token_type) {
    case IABB_TOKEN_I:
        jcc_op = IABB_OP2_JAE_REL32;
        op = IABB_OP_ADD_RM64_IMM32;
        modrm_reg = IABB_MODRM_REG_OP_ADD_RM_IMM;
        cmp_bound = IABB_CONTEXT_MEMORY_SIZE - operand;
        break;
    case IABB_TOKEN_AM:
        jcc_op = IABB_OP2_JB_REL32;
        op = IABB_OP_SUB_RM64_IMM32;
        modrm_reg = IABB_MODRM_REG_OP_SUB_RM_IMM;
        cmp_bound = operand;
        break;
    default: return IABB_ERROR_COMPILER_INTERNAL;
    }

    compiler->token = next_token;

    uint8_t bounds_check[] = {
        // mov rax, r14
        IABB_REX_W | IABB_REX_R,
        IABB_OP_MOV_RM64_R64,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_R14 | IABB_MODRM_RM_RAX,
        // sub rax, r15
        IABB_REX_W | IABB_REX_R,
        IABB_OP_SUB_RM64_R64,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_R15 | IABB_MODRM_RM_RAX,
        // cmp rax, cmp_bound
        IABB_REX_W,
        IABB_OP_CMP_RAX_IMM32,
        IABB_DWORD_TO_BYTES(cmp_bound),
        // (jae|jb) .ret_dp_out_of_bounds ; Offset written later.
        IABB_OP2_TO_BYTES(jcc_op),
        IABB_DWORD_TO_BYTES(0),
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(compiler->dst, bounds_check);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    struct iabb_jit_x86_64_jump jump_on_error = {
        .from = compiler->dst->size,
        .to = IABB_JUMP_RET_ERROR_DP_OUT_OF_BOUNDS,
    };
    error = iabb_buffer_write(
        &compiler->jumps,
        &jump_on_error,
        sizeof(jump_on_error)
    );

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    uint8_t instr[] = {
        // (add|sub) r14, operand
        IABB_REX_W | IABB_REX_B,
        op,
        IABB_MODRM_MOD_DIRECT | modrm_reg | IABB_MODRM_RM_R14,
        IABB_DWORD_TO_BYTES(operand),
    };
    return IABB_BUFFER_WRITE_JIT(compiler->dst, instr);
}

static enum iabb_error
iabb_jit_x86_64_emit_additive_v(struct iabb_jit_x86_64_compiler *compiler) {
    enum iabb_token_type token_type = compiler->token.type;
    struct iabb_lexer *lexer = &compiler->lexer;

    uint8_t op;
    uint8_t modrm_reg;

    switch (token_type) {
    case IABB_TOKEN_BORED:
        op = IABB_OP_ADD_RM8_IMM8;
        modrm_reg = IABB_MODRM_REG_OP_ADD_RM_IMM;
        break;
    case IABB_TOKEN_BTW:
        op = IABB_OP_SUB_RM8_IMM8;
        modrm_reg = IABB_MODRM_REG_OP_SUB_RM_IMM;
        break;
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

    uint8_t instr[] = {
        // (add|sub) BYTE PTR [r14], operand
        IABB_REX_B,
        op,
        IABB_MODRM_MOD_DISP0 | modrm_reg | IABB_MODRM_RM_R14,
        operand,
    };
    return IABB_BUFFER_WRITE_JIT(compiler->dst, instr);
}

static enum iabb_error
iabb_jit_x86_64_emit_write(struct iabb_jit_x86_64_compiler *compiler) {
    uint8_t instrs[] = {
        // movzx edi, BYTE PTR [r14]
        IABB_REX_B,
        IABB_OP2_TO_BYTES(IABB_OP2_MOVZX_R32_RM8),
        IABB_MODRM_MOD_DISP0 | IABB_MODRM_REG_EDI | IABB_MODRM_RM_R14,
        // mov rsi, QWORD PTR [rbx + offsetof(struct iabb_context, out)]
        IABB_REX_W,
        IABB_OP_MOV_R64_RM64,
        IABB_MODRM_MOD_DISP8 | IABB_MODRM_REG_RSI | IABB_MODRM_RM_RBX,
        offsetof(struct iabb_context, out),
        // call r13
        IABB_REX_B,
        IABB_OP_CALL_RM64,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_OP_CALL_RM | IABB_MODRM_RM_R13,
        // cmp eax, -1
        IABB_OP_CMP_EAX_IMM32,
        IABB_DWORD_TO_BYTES((int32_t) -1),
        // je .ret_error_io ; Offset written later.
        IABB_OP2_TO_BYTES(IABB_OP2_JE_REL32),
        IABB_DWORD_TO_BYTES(0),
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(compiler->dst, instrs);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    struct iabb_jit_x86_64_jump jump = {
        .from = compiler->dst->size,
        .to = IABB_JUMP_RET_ERROR_IO,
    };
    return iabb_buffer_write(&compiler->jumps, &jump, sizeof(jump));
}

static enum iabb_error
iabb_jit_x86_64_emit_read(struct iabb_jit_x86_64_compiler *compiler) {
    uint8_t call_fgetc[] = {
        // mov rdi, QWORD PTR [rbx + offsetof(struct iabb_context, in)]
        IABB_REX_W,
        IABB_OP_MOV_R64_RM64,
        IABB_MODRM_MOD_DISP8 | IABB_MODRM_REG_RDI | IABB_MODRM_RM_RBX,
        offsetof(struct iabb_context, in),
        // call r12
        IABB_REX_B,
        IABB_OP_CALL_RM64,
        IABB_MODRM_MOD_DIRECT | IABB_MODRM_REG_OP_CALL_RM | IABB_MODRM_RM_R12,
        // cmp eax, -1
        IABB_OP_CMP_EAX_IMM32,
        IABB_DWORD_TO_BYTES((int32_t) -1),
        // je .handle_fgetc_eof ; Offset written later.
        IABB_OP2_TO_BYTES(IABB_OP2_JE_REL32),
        IABB_DWORD_TO_BYTES(0),
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(compiler->dst, call_fgetc);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    struct iabb_jit_x86_64_jump jump = {
        .from = compiler->dst->size,
        .to = IABB_JUMP_HANDLE_FGETC_EOF,
    };
    error = iabb_buffer_write(&compiler->jumps, &jump, sizeof(jump));

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    uint8_t write_returned_char_to_memory[] = {
        // mov BYTE PTR [r14], al
        IABB_REX_B,
        IABB_OP_MOV_RM8_R8,
        IABB_MODRM_MOD_DISP0 | IABB_MODRM_REG_AL | IABB_MODRM_RM_R14,
    };
    return IABB_BUFFER_WRITE_JIT(compiler->dst, write_returned_char_to_memory);
}

static enum iabb_error
iabb_jit_x86_64_begin_loop(struct iabb_jit_x86_64_compiler *compiler) {
    uint8_t instrs[] = {
        // cmp BYTE PTR [r14], 0
        IABB_REX_B,
        IABB_OP_CMP_RM8_IMM8,
        IABB_MODRM_MOD_DISP0 | IABB_MODRM_REG_OP_CMP_RM_IMM | IABB_MODRM_RM_R14,
        0,
        // je loop_end ; Offset written later.
        IABB_OP2_TO_BYTES(IABB_OP2_JE_REL32),
        IABB_DWORD_TO_BYTES(0),
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(compiler->dst, instrs);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    return iabb_buffer_write_size(&compiler->loop_stack, compiler->dst->size);
}

static enum iabb_error
iabb_jit_x86_64_end_loop(struct iabb_jit_x86_64_compiler *compiler) {
    if (compiler->loop_stack.size == 0) {
        return IABB_ERROR_COMPILER_UNEXPECTED_LOOP_END;
    }

    uint8_t instrs[] = {
        // cmp BYTE PTR [r14], 0
        IABB_REX_B,
        IABB_OP_CMP_RM8_IMM8,
        IABB_MODRM_MOD_DISP0 | IABB_MODRM_REG_OP_CMP_RM_IMM | IABB_MODRM_RM_R14,
        0,
        // jne loop_start ; Offset written later.
        IABB_OP2_TO_BYTES(IABB_OP2_JNE_REL32),
        IABB_DWORD_TO_BYTES(0),
    };
    enum iabb_error error = IABB_BUFFER_WRITE_JIT(compiler->dst, instrs);

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    size_t loop_start = iabb_buffer_pop_size(&compiler->loop_stack);
    error = iabb_jit_x86_64_set_rel32(
        compiler->dst,
        compiler->dst->size,
        loop_start
    );

    if (error != IABB_ERROR_SUCCESS) {
        return error;
    }

    return iabb_jit_x86_64_set_rel32(
        compiler->dst,
        loop_start,
        compiler->dst->size
    );
}

static enum iabb_error
iabb_jit_x86_64_emit(struct iabb_jit_x86_64_compiler *compiler) {
    enum iabb_error error;

    switch (compiler->token.type) {
    case IABB_TOKEN_I:
    case IABB_TOKEN_AM: return iabb_jit_x86_64_emit_additive_p(compiler);
    case IABB_TOKEN_BORED:
    case IABB_TOKEN_BTW: return iabb_jit_x86_64_emit_additive_v(compiler);
    case IABB_TOKEN_BY: error = iabb_jit_x86_64_emit_write(compiler); break;
    case IABB_TOKEN_THE: error = iabb_jit_x86_64_emit_read(compiler); break;
    case IABB_TOKEN_WAY: error = iabb_jit_x86_64_begin_loop(compiler); break;
    case IABB_TOKEN_SLEEP: error = iabb_jit_x86_64_end_loop(compiler); break;
    default: return IABB_ERROR_COMPILER_INVALID_TOKEN;
    }

    if (error == IABB_ERROR_SUCCESS) {
        compiler->token = iabb_lexer_next_token(&compiler->lexer);
    }

    return error;
}

enum iabb_error iabb_compile_jit_x86_64(
    FILE *src,
    struct iabb_buffer *dst,
    struct iabb_token *last_token_dst
) {
    struct iabb_jit_x86_64_compiler compiler;
    enum iabb_error error = iabb_jit_x86_64_compiler_init(&compiler, src, dst);

    if (error != IABB_ERROR_SUCCESS) {
        *last_token_dst = compiler.token;
        return error;
    }

    error = iabb_jit_x86_64_emit_header(dst);

    if (error != IABB_ERROR_SUCCESS) {
        *last_token_dst = compiler.token;
        iabb_jit_x86_64_compiler_fini(&compiler);
        return error;
    }

    while (compiler.token.type != IABB_TOKEN_EOF) {
        error = iabb_jit_x86_64_emit(&compiler);

        if (error != IABB_ERROR_SUCCESS) {
            *last_token_dst = compiler.token;
            iabb_jit_x86_64_compiler_fini(&compiler);
            return error;
        }
    }

    *last_token_dst = compiler.token;

    if (compiler.loop_stack.size != 0) {
        iabb_jit_x86_64_compiler_fini(&compiler);
        return IABB_ERROR_COMPILER_UNCLOSED_LOOPS;
    }

    error = iabb_jit_x86_64_emit_footer(&compiler.jumps, dst);
    iabb_jit_x86_64_compiler_fini(&compiler);
    return error;
}
