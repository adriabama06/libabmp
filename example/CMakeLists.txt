add_executable(example_libabmp
    main.c
)

target_link_libraries(example_libabmp PRIVATE abmp)
