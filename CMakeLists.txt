cmake_minimum_required(VERSION 3.10)
project(libabmp VERSION 1.0.1 LANGUAGES C)

# Set ABMP_ABITMAP_NOT_USE_PACKED_HEADER to 0 by default
add_compile_definitions(ABMP_ABITMAP_NOT_USE_PACKED_HEADER=0)

# Library target
add_library(libabmp STATIC
    src/abmp.c

    src/read.c
    src/write.c

    src/file_read.c
    src/file_write.c

    src/get.c
    src/create.c
)

target_include_directories(libabmp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Remove prefix lib to prevent naming: liblibabmp
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Example executable
# add_subdirectory(example)
