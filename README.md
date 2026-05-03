# Flang Implicit Allocation Profiler and Optimizer

A compiler design lab project that detects and explains implicit heap allocations in textual LLVM Flang HLFIR/FIR MLIR. The project includes a C++ profiler, JSON/text reporting, a conservative Fortran source transformation prototype, benchmark scripts, documentation, and a final report.

## Quick Start

On Linux/macOS or a Unix-like shell with CMake and a C++ compiler:

```sh
mkdir build
cd build
cmake ..
make
./flang-implicit-alloc-profiler ../tests/simple_alloc.mlir
```

Expected output:

```text
Flang Implicit Allocation Report
Input: ../tests/simple_alloc.mlir
Allocations reported: 1
Threshold: 0.00 MB

line 13: array expression temporary generates 8.00 MB temporary array allocation
source file: examples/simple_array_expr.f90
source line: 13
source column: 3
IR operation: fir.allocmem
estimated bytes: 8388608
estimated MB: 8.00
classification: PROVABLY_UNNECESSARY
reason: temporary is local, has static shape, does not appear to escape, and is freed locally
confidence: 0.78
suggestion: replace the whole-array expression with an explicit loop or stack-sized local buffer
```

Run the source transformation prototype:

```sh
cd ..
python3 scripts/transform_fortran.py examples/simple_array_expr.f90
```

Expected transformed snippet:

```fortran
! transformed by transform_fortran.py: explicit loop avoids an array temporary
do i = 1, size(A)
  A(i) = B(i) + C(i)
end do
```

On Windows with MinGW:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER="C:\MinGW\bin\g++.exe"
& "C:\Program Files\CMake\bin\cmake.exe" --build build-mingw
.\build-mingw\flang-implicit-alloc-profiler.exe tests\simple_alloc.mlir
python scripts\transform_fortran.py examples\simple_array_expr.f90
```

## Features

- Detects `fir.allocmem` in textual Flang FIR/HLFIR MLIR.
- Matches `fir.freemem` for local lifetime evidence.
- Captures simple MLIR source locations like `loc("file":line:column)`.
- Estimates static allocation sizes from visible FIR array types.
- Classifies allocations as:
  - `PROVABLY_UNNECESSARY`
  - `POSSIBLY_UNNECESSARY`
  - `NECESSARY`
- Emits human-readable text reports and JSON reports.
- Provides a conservative Python source-to-source transformer for simple Fortran array assignments.
- Includes benchmark and evaluation scripts that produce Markdown and JSON results.
- Includes a local browser UI for demos.

## Project Layout

```text
.
+-- CMakeLists.txt
+-- include/FAP/
+-- src/
+-- tests/
+-- examples/
+-- scripts/
+-- docs/
+-- report/
```

## CLI Usage

```sh
flang-implicit-alloc-profiler [options] input.mlir
```

Options:

- `--format=text`: print the default human-readable report.
- `--format=json`: print a JSON report.
- `--show-ir`: include the matched `fir.allocmem` line.
- `--threshold-mb=1`: only report statically-sized allocations at least 1 MB; unknown sizes remain visible.

Sample commands:

```sh
./flang-implicit-alloc-profiler ../tests/simple_alloc.mlir
./flang-implicit-alloc-profiler --format=json ../tests/simple_alloc.mlir
./flang-implicit-alloc-profiler --show-ir ../tests/array_expression_temporary.mlir
./flang-implicit-alloc-profiler --threshold-mb=9 ../tests/array_expression_temporary.mlir
```

JSON output example:

```json
{
  "source_file": "examples/simple_array_expr.f90",
  "source_line": 13,
  "source_column": 3,
  "ir_operation_name": "fir.allocmem",
  "estimated_bytes": 8388608,
  "estimated_mb": 8.00,
  "classification": "PROVABLY_UNNECESSARY",
  "suggested_transformation": "replace the whole-array expression with an explicit loop or stack-sized local buffer"
}
```

## Source Transformation

The transformer is intentionally conservative. It handles only simple one-dimensional assignments:

```fortran
A = B + C
A = B * C
A = B + scalar
A(:) = B(:) + C(:)
```

Run it and write output to a file:

```sh
python3 scripts/transform_fortran.py examples/simple_array_expr.f90 -o examples/transformed_simple_array_expr.f90
```

Complex cases are skipped with reasons printed to stderr, for example:

```text
line 19: skipped 'A = sin(B) + C': contains calls, components, or complex indexing
```

## Local Web UI

Start the demo UI from the repository root:

```sh
python3 scripts/serve_ui.py
```

On Windows:

```powershell
python scripts\serve_ui.py
```

Open:

```text
http://127.0.0.1:8765
```

The UI loads sample MLIR files from `tests/`, sends pasted input to the existing `flang-implicit-alloc-profiler` executable, and renders allocation classifications, sizes, source locations, reasons, and suggestions.

## Evaluation

Run the benchmark framework:

```sh
python3 scripts/run_evaluation.py --runs 3
```

Outputs:

- `report/evaluation_results.md`
- `report/evaluation_results.json`

The script compiles and runs original/transformed Fortran programs when `gfortran`, `flang-new`, or `flang` is installed. Optional allocation tools such as `/usr/bin/time`, Valgrind Massif, and the provided `LD_PRELOAD` malloc counter are used when available.

## Tests

From a configured build directory:

```sh
ctest --output-on-failure
```

The CTest suite checks help output, simple allocation detection, classification behavior, text formatting, JSON formatting, `--show-ir`, and threshold filtering.

## Documentation

- `docs/design.md`: architecture and design notes.
- `docs/flang_ir_notes.md`: short HLFIR/FIR terminology notes.
- `docs/limitations.md`: known limitations and future integration path.
- `report/final_report.md`: complete final lab report.

## Notes on LLVM/MLIR/Flang

This project currently uses a standalone textual scanner so it is easy to build for a lab submission. The code is structured around LLVM/MLIR/Flang concepts and can later be upgraded to use MLIR parser and operation-walking APIs directly.
