cmake_minimum_required(VERSION 3.10)
project(libabmp VERSION 1.0.0 LANGUAGES C)

# Library target
add_library(abmp STATIC
    src/abmp.c
    src/read.c
)

target_include_directories(abmp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Example executable
add_subdirectory(example)
