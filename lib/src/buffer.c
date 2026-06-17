// Copyright (C) 2022 OverMighty
// Modified work copyright (C) 2026
// SPDX-License-Identifier: GPL-3.0-only

#include "iabb/buffer.h"

#include "iabb/errors.h"

#include <sys/mman.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IABB_BUFFER_DEFAULT_CAP 64
#define IABB_BUFFER_GROWTH_FACTOR 2

enum iabb_error iabb_buffer_init(struct iabb_buffer *buffer) {
    buffer->size = 0;
    buffer->cap = IABB_BUFFER_DEFAULT_CAP;
    buffer->data = malloc(buffer->cap);

    if (!buffer->data) {
        return IABB_ERROR_MALLOC;
    }

    return IABB_ERROR_SUCCESS;
}

enum iabb_error iabb_buffer_init_jit(struct iabb_buffer *buffer) {
    buffer->size = 0;
    buffer->cap = sysconf(_SC_PAGESIZE);
    buffer->data = mmap(
        NULL,
        buffer->cap,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANON,
        -1,
        0
    );

    if (buffer->data == MAP_FAILED) {
        return IABB_ERROR_MALLOC;
    }

    return IABB_ERROR_SUCCESS;
}

enum iabb_error
iabb_buffer_write(struct iabb_buffer *buffer, const void *data, size_t n) {
    if (buffer->size + n > buffer->cap) {
        buffer->cap *=
            (buffer->cap + n - 1) / buffer->cap * IABB_BUFFER_GROWTH_FACTOR;
        uint8_t *new_data = realloc(buffer->data, buffer->cap);

        if (!new_data) {
            return IABB_ERROR_MALLOC;
        }

        buffer->data = new_data;
    }

    memcpy(&buffer->data[buffer->size], data, n);
    buffer->size += n;
    return IABB_ERROR_SUCCESS;
}

enum iabb_error
iabb_buffer_write_jit(struct iabb_buffer *buffer, const void *data, size_t n) {
    if (buffer->size + n > buffer->cap) {
        size_t prev_cap = buffer->cap;
        buffer->cap *=
            (buffer->cap + n - 1) / buffer->cap * IABB_BUFFER_GROWTH_FACTOR;
        uint8_t *new_data = mmap(
            NULL,
            buffer->cap,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_ANON,
            -1,
            0
        );

        if (new_data == MAP_FAILED) {
            return IABB_ERROR_MALLOC;
        }

        memcpy(new_data, buffer->data, buffer->size);
        munmap(buffer->data, prev_cap);
        buffer->data = new_data;
    }

    memcpy(&buffer->data[buffer->size], data, n);
    buffer->size += n;
    return IABB_ERROR_SUCCESS;
}

size_t iabb_buffer_pop_size(struct iabb_buffer *buffer) {
    size_t top = ((size_t *) &buffer->data[buffer->size])[-1];
    buffer->size -= sizeof(top);
    return top;
}

void iabb_buffer_fini(struct iabb_buffer *buffer) {
    free(buffer->data);
}

void iabb_buffer_fini_jit(struct iabb_buffer *buffer) {
    munmap(buffer->data, buffer->cap);
}
