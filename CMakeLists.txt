cmake_minimum_required(VERSION 3.10)
project(libabmp VERSION 1.0.0 LANGUAGES C)

# Set ABMP_ABITMAP_NOT_USE_PACKED_HEADER to 0 by default
add_compile_definitions(ABMP_ABITMAP_NOT_USE_PACKED_HEADER=0)

# Library target
add_library(abmp STATIC
    src/abmp.c
    src/read.c
    src/write.c
    src/get.c
)

target_include_directories(abmp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Example executable
add_subdirectory(example)
