# Copyright (C) 2022 OverMighty
# Modified work copyright (C) 2026
# SPDX-License-Identifier: GPL-3.0-only

add_executable(i-am-bored-btw compile_and_run.c main.c debug_handler.c)

target_compile_features(i-am-bored-btw PUBLIC c_std_99)
target_compile_options(
    i-am-bored-btw PRIVATE
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall -Wextra -pedantic>
)

if(IABB_USE_JIT)
    target_compile_definitions(i-am-bored-btw PUBLIC IABB_USE_JIT)
endif()

configure_file(version.h.in version.h)
target_include_directories(i-am-bored-btw PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(i-am-bored-btw PRIVATE iabb)

set_target_properties(
    i-am-bored-btw
    PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}
)

install(
    TARGETS i-am-bored-btw
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
)
