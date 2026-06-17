# Copyright (C) 2022 OverMighty
# Modified work copyright (C) 2026
# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.23)

project(i-am-bored-btw VERSION 0.1.0)

option(IABB_BUILD_EXAMPLES "Build the libiabb example programs by default.")
option(
    IABB_USE_JIT
    "Use JIT compilation in the interpreter if available for the target system."
    ON
)

# Windows does not follow the System V ABI's calling convention, and Apple
# platforms require passing the `MAP_JIT` flag to `mmap()` and the program to
# have a relevant entitlement.
if(IABB_USE_JIT AND (WIN32 OR APPLE))
    set(IABB_USE_JIT OFF)
endif()

include(GNUInstallDirs)

add_subdirectory(lib)
add_subdirectory(cmd)

if(IABB_BUILD_EXAMPLES)
    add_subdirectory(examples/libiabb)
else()
    add_subdirectory(examples/libiabb EXCLUDE_FROM_ALL)
endif()
