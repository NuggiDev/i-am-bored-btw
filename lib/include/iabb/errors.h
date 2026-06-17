// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_ERRORS_H
#define IABB_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

// I use Arch btw errors.
enum iabb_error {
    // Success.
    IABB_ERROR_SUCCESS,

    // Memory allocation error.
    IABB_ERROR_MALLOC,
    // Input/output error.
    IABB_ERROR_IO,

    // Invalid target.
    IABB_ERROR_INVALID_TARGET,

    // Invalid token.
    IABB_ERROR_COMPILER_INVALID_TOKEN,
    // Unexpected loop end.
    IABB_ERROR_COMPILER_UNEXPECTED_LOOP_END,
    // Unclosed loops.
    IABB_ERROR_COMPILER_UNCLOSED_LOOPS,
    // Internal compiler error.
    IABB_ERROR_COMPILER_INTERNAL,

    // Data pointer out of bounds.
    IABB_ERROR_DP_OUT_OF_BOUNDS,

    // End of input file.
    IABB_ERROR_RUNTIME_END_OF_INPUT_FILE,

    // Invalid bytecode opcode.
    IABB_ERROR_BYTECODE_INVALID_OP,

    // Jump too large.
    IABB_ERROR_JIT_JUMP_TOO_LARGE,
};

// Returns a description of the given error as a string.
const char *iabb_strerror(enum iabb_error error);

#ifdef __cplusplus
}
#endif

#endif // IABB_ERRORS_H
