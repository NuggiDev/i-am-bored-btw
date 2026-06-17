# Copyright (C) 2022 OverMighty
# Modified work copyright (C) 2026
# SPDX-License-Identifier: GPL-3.0-only

add_library(
    iabb
    src/buffer.c
    src/context.c
    src/errors.c
    src/lexer.c
    src/targets/bytecode.c
    src/targets/bytecode_compile.c
    src/targets/bytecode_run.c
    src/targets.c
    src/targets/jit_x86_64_compile.c
    src/targets/jit_x86_64_run.c
    src/token.c
)

target_compile_features(iabb PUBLIC c_std_99)
target_compile_options(
    iabb PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall -Wextra -pedantic>
)

configure_file(include/iabb/version.h.in include/iabb/version.h)
target_sources(
    iabb PUBLIC
    FILE_SET HEADERS
    BASE_DIRS include ${CMAKE_CURRENT_BINARY_DIR}/include
    FILES
    include/iabb/buffer.h
    include/iabb/context.h
    include/iabb/errors.h
    include/iabb/lexer.h
    include/iabb/targets/bytecode.h
    include/iabb/targets.h
    include/iabb/targets/jit_x86_64.h
    include/iabb/token.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/iabb/version.h
)

set_target_properties(
    iabb
    PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}
)

install(
    TARGETS iabb
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}
)
