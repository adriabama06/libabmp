#!/usr/bin/env sh
# Configure, build, and run libabmp's CTest suite from any working directory.
set -eu

repository_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir=${BUILD_DIR:-"$repository_root/build"}

cmake -S "$repository_root" -B "$build_dir" -DBUILD_TESTING=ON
cmake --build "$build_dir"
ctest --test-dir "$build_dir" --output-on-failure
