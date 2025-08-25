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

FetchContent_Declare(libabmp
  GIT_REPOSITORY https://github.com/adriabama06/libabmp.git
  GIT_TAG        v1.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(libabmp)

target_link_libraries(example_libabmp
    PRIVATE libabmp
)
