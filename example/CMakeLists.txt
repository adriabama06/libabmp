cmake_minimum_required(VERSION 3.22)

project(example_libabmp
  LANGUAGES C
  VERSION 1.0.0
)

add_executable(example_libabmp
    main.c
)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

# Add an option to choose between local and remote
option(USE_LOCAL_LIBABMP "Use local libabmp from ../libabmp" ON)

if(USE_LOCAL_LIBABMP)
    add_subdirectory(.. libabmp EXCLUDE_FROM_ALL)
else()
    FetchContent_Declare(libabmp
        GIT_REPOSITORY https://github.com/adriabama06/libabmp.git
        GIT_TAG        main
        GIT_PROGRESS   TRUE
        GIT_SHALLOW    TRUE
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(libabmp)
endif()

target_link_libraries(example_libabmp
    PRIVATE libabmp
)
