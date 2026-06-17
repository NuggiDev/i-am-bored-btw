// Modified work copyright (C) 2026
#include "iabb/buffer.h"
#include "iabb/context.h"
#include "iabb/errors.h"
#include "iabb/targets.h"
#include "iabb/token.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

struct FileDeleter {
    void operator()(std::FILE *file) {
        std::fclose(file);
    }
};

int compile(iabb_target target, std::FILE *src, iabb_buffer *dst) {
    iabb_token last_token;
    iabb_error error = iabb_compile(target, src, dst, &last_token);

    if (error != IABB_ERROR_SUCCESS) {
        std::cerr << "compile-time error: " << iabb_strerror(error)
                  << " at line " << last_token.line << ", col "
                  << last_token.col << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void debug_handler([[maybe_unused]] iabb_context *ctx) {
    std::cout << "debug handler called\n";
}

int run(iabb_target target, iabb_buffer *program) {
    iabb_context ctx;
    iabb_context_init(&ctx, program->data, stdin, stdout, debug_handler);
    iabb_error error = iabb_run(target, &ctx);

    if (error != IABB_ERROR_SUCCESS) {
        std::cerr << "run-time error: " << iabb_strerror(error) << " at "
                  << std::hex << std::showbase
                  << reinterpret_cast<uintptr_t>(ctx.ip - 1) << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int compile_and_run(const char *filename) {
    std::unique_ptr<std::FILE, FileDeleter> src{std::fopen(filename, "rbe")};

    if (src == nullptr) {
        std::cerr << "failed to open source file: " << std::strerror(errno)
                  << "\n";
        return EXIT_FAILURE;
    }

    iabb_buffer program;
    iabb_error error = iabb_buffer_init(&program);

    if (error != IABB_ERROR_SUCCESS) {
        std::cerr << "failed to init program buffer: " << iabb_strerror(error)
                  << "\n";
        return EXIT_FAILURE;
    }

    iabb_target target = IABB_TARGET_BYTECODE;

    int status = compile(target, src.get(), &program);

    if (status == EXIT_SUCCESS) {
        status = run(target, &program);
    }

    iabb_buffer_fini(&program);
    return status;
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source file>\n";
        return EXIT_FAILURE;
    }

    return compile_and_run(argv[1]);
}
