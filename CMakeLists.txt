cmake_minimum_required(VERSION 3.16)
project(fibers C ASM)

set (CMAKE_C_STANDARD 99)

file(GLOB fibers_src
    src/main.c
    src/stack.c
    src/coro.c
    src/execucion_context_switch.S
)

include_directories(inc)

# add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options (-g)
endif()

add_executable(fibers_bin ${fibers_src})
