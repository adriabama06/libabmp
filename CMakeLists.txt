cmake_minimum_required(VERSION 3.10)
project(libabmp VERSION 1.0.1 LANGUAGES C)

include(CTest)

# Set ABMP_ABITMAP_NOT_USE_PACKED_HEADER to 0 by default
add_compile_definitions(ABMP_ABITMAP_NOT_USE_PACKED_HEADER=0)

# Library target
add_library(libabmp STATIC
    src/abmp.c

    src/read.c
    src/write.c

    src/file_read.c
    src/file_write.c

    src/create.c
)

target_include_directories(libabmp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Remove prefix lib to prevent naming: liblibabmp
set(CMAKE_STATIC_LIBRARY_PREFIX "")

# Example executable
# add_subdirectory(example)

if(BUILD_TESTING)
    add_executable(libabmp_tests
        tests/test_abmp.c
    )
    target_link_libraries(libabmp_tests PRIVATE libabmp)
    target_compile_definitions(libabmp_tests PRIVATE
        ABMP_TEST_OUTPUT_PATH="${CMAKE_CURRENT_BINARY_DIR}/libabmp_test_output.bmp"
    )
    add_test(NAME libabmp_tests COMMAND libabmp_tests)
endif()
