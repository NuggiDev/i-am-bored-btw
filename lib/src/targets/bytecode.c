// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/targets/bytecode.h"

#include <stdint.h>

const char *iabb_bytecode_op_name(uint8_t op) {
    switch (op) {
    case IABB_BYTECODE_OP_RET: return "ret";
    case IABB_BYTECODE_OP_ADDP: return "addp";
    case IABB_BYTECODE_OP_SUBP: return "subp";
    case IABB_BYTECODE_OP_ADDV: return "addv";
    case IABB_BYTECODE_OP_SUBV: return "subv";
    case IABB_BYTECODE_OP_WRITE: return "write";
    case IABB_BYTECODE_OP_READ: return "read";
    case IABB_BYTECODE_OP_JMPZ: return "jmpz";
    case IABB_BYTECODE_OP_JMPNZ: return "jmpnz";
    case IABB_BYTECODE_OP_DEBUG: return "debug";
    default: return "???";
    }
}
