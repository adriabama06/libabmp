# AGENTS.md

## Build & Test

```bash
# Build only
cmake -B build && cmake --build build

# Full test suite (configure + build + run)
./scripts/run_tests.sh

# Run with valgrind leak checks
./scripts/run_tests.sh --valgrind

# Run a single test (after initial build)
cmake -B build -DBUILD_TESTING=ON && cmake --build build
ctest --test-dir build -R test_create --output-on-failure
```

The script uses `BUILD_DIR` env var if set, defaults to `./build`.

## Architecture

- **Language:** C (C11-style, no external dependencies)
- **Build:** CMake 3.10+, static library `libabmp`
- **Output:** Always `libabmp.a` (CMAKE_STATIC_LIBRARY_PREFIX cleared)
- **Compile definition:** `ABMP_ABITMAP_NOT_USE_PACKED_HEADER=0` — controls packed struct attribute in `abitmap.h`. Changing this value affects binary layout and memory reads.

### Key files

| File | Role |
|------|------|
| `include/abmp.h` | Public API — all inline helpers live here |
| `include/abitmap.h` | Core structs: `ABMP_BITMAP_HEADER`, `ABMP_BITMAP` |
| `src/create.c` | `abmp_create_bitmap`, `abmp_free` |
| `src/read.c` / `src/write.c` | Memory-to-memory serialization |
| `src/file_read.c` / `src/file_write.c` | FILE*-based and filepath-based APIs |
| `tests/test_common.h` | Shared helpers: `make_bitmap()`, `check_bitmap_equal()` |

### API naming conventions

Two flavors for file I/O:
- `abmp_*_using_memory()` — reads entire file into a buffer, then parses
- `abmp_*_using_direct()` — operates directly on FILE* / filepath

### Known issue

`abmp_makebuffer()` in `abmp.h:94` has a bug: the `buffer` parameter is passed by value, so the caller's pointer is never updated. Also `abmp_allocate_filedata` returns NULL on failure but line 101 checks `if(buffer)` (inverted logic).

## Testing conventions

- **Framework:** None — plain `assert()` + `puts()`. Each test file is a standalone executable.
- **Pattern:** `static int test_xxx(void)` returning 0 on success, called from `main()` via `assert(test_xxx() == 0)`.
- **Headers:** Always `#include "test_common.h"` (which pulls in `abmp.h`, `<assert.h>`, `<stdio.h>`, `<stdlib.h>`, `<string.h>`).
- **New tests:** Add entry to `TEST_SUITES` list in `CMakeLists.txt`. Each test gets compile definitions `ABMP_TEST_OUTPUT_PATH` and `ABMP_TEST_SAMPLE_PATH`.
- **Fixtures:** `make_bitmap()` creates a 3x2 bitmap with deterministic pixel values (`i * 11 + 3`). `check_bitmap_equal()` does memcmp on header + full pixel payload.
- **Cleanup:** Tests call `abmp_free()` and `remove()` on temp files. Memory leak tests (`test_errors_leak.c`) exercise create/free stress cycles.
- **CI:** GitHub Actions runs `run_tests.sh` then `run_tests.sh --valgrind` on every push/PR.
