// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_TARGETS_BYTECODE_H
#define IABB_TARGETS_BYTECODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../buffer.h"
#include "../context.h"
#include "../errors.h"
#include "../token.h"

#include <stdint.h>
#include <stdio.h>

// I use Arch btw bytecode opcodes.
enum {
    // Return from the program.
    IABB_BYTECODE_OP_RET,
    // Add to the data pointer the following `uint16_t` value.
    IABB_BYTECODE_OP_ADDP,
    // Subtract from the data pointer the following `uint16_t` value.
    IABB_BYTECODE_OP_SUBP,
    // Add to the value pointed to by the data pointer the following `uint8_t`
    // value.
    IABB_BYTECODE_OP_ADDV,
    // Subtract from the value pointed to by the data pointer the following
    // `uint8_t` value.
    IABB_BYTECODE_OP_SUBV,
    // Write the value pointed to by the data pointer as character to the output
    // file.
    IABB_BYTECODE_OP_WRITE,
    // Read a character from the input file into the value pointed to by the
    // data pointer.
    IABB_BYTECODE_OP_READ,
    // Jump if the value pointed to by the data pointer is zero to the following
    // `size_t` program offset.
    IABB_BYTECODE_OP_JMPZ,
    // Jump if the value pointed to by the data pointer is not zero to the
    // following `size_t` program offset.
    IABB_BYTECODE_OP_JMPNZ,
    // Call the debugging event handler.
    IABB_BYTECODE_OP_DEBUG,
};

// Returns the name of the given I use Arch btw bytecode opcode as a string.
const char *iabb_bytecode_op_name(uint8_t op);

// Compiles the source file pointed to by `src` into I use Arch btw bytecode to
// write to the buffer pointed to by `dst` and writes the last token
// processed at the location pointed to by `last_token_dst`. Returns the error
// that occurred in the process.
enum iabb_error iabb_compile_bytecode(
    FILE *src,
    struct iabb_buffer *dst,
    struct iabb_token *last_token_dst
);

// Runs the I use Arch btw bytecode program from the context pointed to by
// `ctx`. Returns the error that occurred in the process.
enum iabb_error iabb_run_bytecode(struct iabb_context *ctx);

#ifdef __cplusplus
}
#endif

#endif // IABB_TARGETS_BYTECODE_H
