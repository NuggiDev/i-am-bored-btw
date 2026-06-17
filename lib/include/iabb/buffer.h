// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#ifndef IABB_BUFFER_H
#define IABB_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "errors.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// A growable byte buffer.
struct iabb_buffer {
    size_t size;
    size_t cap;
    uint8_t *data;
};

// Initializes the given buffer to store non-executable data. Returns the error
// that occurred in the process.
enum iabb_error iabb_buffer_init(struct iabb_buffer *buffer);

// Initializes the given buffer to store executable code. Returns the error that
// occurred in the process.
enum iabb_error iabb_buffer_init_jit(struct iabb_buffer *buffer);

// Initializes the given buffer to store executable code if `is_jit` is true,
// or to store non-executable data otherwise.
static inline enum iabb_error
iabb_buffer_init_maybe_jit(struct iabb_buffer *buffer, bool is_jit) {
    if (is_jit) {
        return iabb_buffer_init_jit(buffer);
    } else {
        return iabb_buffer_init(buffer);
    }
}

// Writes `n` bytes from `data` at the end of the given buffer. Returns the
// error that occurred in the process.
//
// The buffer must have been initialized with `iabb_buffer_init()`.
enum iabb_error
iabb_buffer_write(struct iabb_buffer *buffer, const void *data, size_t n);

// Writes the given `uint8_t` value at the end of the given buffer. Returns the
// error that occurred in the process.
static inline enum iabb_error
iabb_buffer_write_u8(struct iabb_buffer *buffer, uint8_t value) {
    return iabb_buffer_write(buffer, &value, sizeof(value));
}

// Writes the given `size_t` value at the end of the given buffer. Returns the
// error that occurred in the process.
static inline enum iabb_error
iabb_buffer_write_size(struct iabb_buffer *buffer, size_t value) {
    return iabb_buffer_write(buffer, &value, sizeof(value));
}

// Transforms to a call to `iabb_buffer_write()` with `sizeof(data)` as argument
// for the `n` parameter.
#define IABB_BUFFER_WRITE(buffer, data) \
    iabb_buffer_write((buffer), (data), sizeof(data))

// Writes `n` bytes from `data` at the end of the given buffer. Returns the
// error that occurred in the process.
//
// The buffer must have been initialized with `iabb_buffer_init_jit()`.
enum iabb_error
iabb_buffer_write_jit(struct iabb_buffer *buffer, const void *data, size_t n);

// Transforms to a call to `iabb_buffer_write_jit()` with `sizeof(data)` as
// argument for the `n` parameter.
#define IABB_BUFFER_WRITE_JIT(buffer, data) \
    iabb_buffer_write_jit((buffer), (data), sizeof(data))

// Returns and removes the `size_t` value at the end of the given buffer.
size_t iabb_buffer_pop_size(struct iabb_buffer *buffer);

// Finalizes the given buffer initialized with `iabb_buffer_init()`. Frees the
// allocated storage capacity.
void iabb_buffer_fini(struct iabb_buffer *buffer);

// Finalizes the given buffer initialized with `iabb_buffer_init_jit()`. Frees
// the allocated storage capacity.
void iabb_buffer_fini_jit(struct iabb_buffer *buffer);

// Finalizes the given buffer initialized with `iabb_buffer_init_jit()` if
// `is_jit` is true, otherwise with `iabb_buffer_init()`. Frees the allocated
// storage capacity.
static inline void
iabb_buffer_fini_maybe_jit(struct iabb_buffer *buffer, bool is_jit) {
    if (is_jit) {
        iabb_buffer_fini_jit(buffer);
    } else {
        iabb_buffer_fini(buffer);
    }
}

#ifdef __cplusplus
}
#endif

#endif // IABB_BUFFER_H
