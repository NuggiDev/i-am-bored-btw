// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/errors.h"

const char *iabb_strerror(enum iabb_error error) {
    switch (error) {
    case IABB_ERROR_SUCCESS: return "success";
    case IABB_ERROR_MALLOC: return "memory allocation error";
    case IABB_ERROR_IO: return "input/output error";
    case IABB_ERROR_COMPILER_INVALID_TOKEN: return "invalid token";
    case IABB_ERROR_COMPILER_UNEXPECTED_LOOP_END: return "unexpected loop end";
    case IABB_ERROR_COMPILER_UNCLOSED_LOOPS: return "unclosed loops";
    case IABB_ERROR_COMPILER_INTERNAL: return "internal compiler error";
    case IABB_ERROR_DP_OUT_OF_BOUNDS: return "data pointer out of bounds";
    case IABB_ERROR_RUNTIME_END_OF_INPUT_FILE: return "end of input file";
    case IABB_ERROR_BYTECODE_INVALID_OP: return "invalid bytecode opcode";
    case IABB_ERROR_JIT_JUMP_TOO_LARGE: return "jump too large";
    default: return "???";
    }
}
