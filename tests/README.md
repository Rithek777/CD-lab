# Tests

This directory contains small MLIR fixtures and CTest entries for the allocation profiler. The snippets are intentionally compact, but they use realistic Flang HLFIR/FIR operation names.

## Fixtures

- `simple_alloc.mlir`: minimal `fir.allocmem` / `fir.freemem` example used by the quick start.
- `array_expression_temporary.mlir`: array expression temporary near `hlfir.elemental`.
- `allocatable_reassignment.mlir`: allocatable assignment reallocation near `hlfir.assign`.
- `array_section_copy.mlir`: copy-in/copy-out style temporary for a non-contiguous section.
- `function_result_temporary.mlir`: unknown-size array-valued function result temporary.

## Run Tests

From a Unix-like build directory:

```sh
cmake ..
make
ctest --output-on-failure
```

From Windows with MinGW:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:\MinGW\bin\g++.exe"
& "C:\Program Files\CMake\bin\cmake.exe" --build build-mingw
& "C:\Program Files\CMake\bin\ctest.exe" --test-dir build-mingw --output-on-failure
```

The tests cover help output, allocation detection, classification, text report formatting, JSON report formatting, `--show-ir`, and `--threshold-mb`.
