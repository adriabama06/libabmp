#!/usr/bin/env sh
# Configure, build, and run libabmp's CTest suite from any working directory.
set -eu

repository_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${BUILD_DIR:-"$repository_root/build"}

valgrind=0
if [ "${1:-}" = "--valgrind" ]; then
    valgrind=1
fi

cmake -S "$repository_root" -B "$build_dir" -DBUILD_TESTING=ON
cmake --build "$build_dir"

if [ "$valgrind" -eq 1 ]; then
    valgrind --error-exitcode=1 --leak-check=full --error-limit=no ctest --test-dir "$build_dir" --output-on-failure
else
    ctest --test-dir "$build_dir" --output-on-failure
fi
