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
    set(TEST_SUITES
        test_create
        test_memory
        test_file
        test_errors
        test_errors_leak
        test_fuzz
    )

    foreach(test_suite ${TEST_SUITES})
        add_executable(${test_suite} tests/${test_suite}.c)
        target_link_libraries(${test_suite} PRIVATE libabmp)
        target_compile_definitions(${test_suite} PRIVATE
            ABMP_TEST_OUTPUT_PATH="${CMAKE_CURRENT_BINARY_DIR}/${test_suite}_output.bmp"
            ABMP_TEST_SAMPLE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/samples/twoofpadding.bmp"
        )
        add_test(NAME ${test_suite} COMMAND ${test_suite})
    endforeach()
endif()
