# Flang Implicit Allocation Profiler and Optimizer

This repository is a beginner-friendly CD lab project focused on implicit heap allocations in LLVM Flang IR.

The tool scans Flang HLFIR/FIR IR text and reports allocation sites that may come from:

- array expression temporaries
- array-valued function results
- automatic reallocation on allocatable assignment
- copy-in/copy-out or array section temporaries

The current first working version scans textual MLIR/FIR/HLFIR input and reports `fir.allocmem` operations, matching `fir.freemem` operations, source locations, static size estimates when visible, and a simple reason classification.

It also classifies each allocation as:

- `PROVABLY_UNNECESSARY`
- `POSSIBLY_UNNECESSARY`
- `NECESSARY`

Each report includes a confidence score and suggested fix.

## Project Layout

```text
.
+-- CMakeLists.txt
+-- include/
|   +-- FAP/
+-- src/
+-- tests/
+-- examples/
+-- scripts/
+-- docs/
+-- report/
```

## Current IR Focus

The analysis scans MLIR text for operations and patterns such as:

- `fir.allocmem`
- `fir.freemem`
- `hlfir.expr`
- `hlfir.assign`
- `hlfir.elemental`
- temporary arrays created for expression evaluation
- source location metadata attached to MLIR operations

## Build Instructions

From the repository root:

```powershell
cmake -S . -B build
cmake --build build
```

Run the profiler:

```powershell
.\build\Debug\flang-implicit-alloc-profiler.exe --help
```

On single-config generators such as Ninja or Makefiles, the executable may be here instead:

```powershell
.\build\flang-implicit-alloc-profiler.exe --help
```

## CLI Usage

```powershell
flang-implicit-alloc-profiler [options] input.mlir
```

Options:

- `--format=text` prints the default human-readable report
- `--format=json` prints a JSON report
- `--show-ir` includes the matched `fir.allocmem` line
- `--threshold-mb=1` only reports allocations at least 1 MB; unknown sizes are kept visible

Examples:

```powershell
flang-implicit-alloc-profiler --format=text tests\array_expression_temporary.mlir
flang-implicit-alloc-profiler --format=json --show-ir tests\array_expression_temporary.mlir
flang-implicit-alloc-profiler --threshold-mb=1 tests\array_expression_temporary.mlir
```

Example from the build directory on Windows:

```powershell
.\build-mingw\flang-implicit-alloc-profiler.exe --format=text tests\array_expression_temporary.mlir
```

Example report:

```text
line 12: array expression temporary generates 8.00 MB temporary array allocation
classification: PROVABLY_UNNECESSARY
suggestion: replace the whole-array expression with an explicit loop or stack-sized local buffer
```

JSON reports include the same fields using stable keys:

```json
{
  "source_file": "examples/array_expression_temporary.f90",
  "source_line": 12,
  "source_column": 7,
  "ir_operation_name": "fir.allocmem",
  "estimated_bytes": 8388608,
  "estimated_mb": 8.00,
  "classification": "PROVABLY_UNNECESSARY",
  "suggested_transformation": "replace the whole-array expression with an explicit loop or stack-sized local buffer"
}
```

## Optional LLVM/MLIR Mode

If LLVM, MLIR, and Flang development packages are available on your system, configure with:

```powershell
cmake -S . -B build -DFAP_ENABLE_LLVM=ON -DMLIR_DIR="path\to\mlir\cmake" -DLLVM_DIR="path\to\llvm\cmake"
cmake --build build
```

LLVM mode currently prepares the build for future integration. This version uses a standalone textual MLIR scanner so the project can be built easily on a beginner machine.

## Example Flang Commands

When Flang is installed, you can generate HLFIR/FIR-like MLIR for the examples with commands similar to:

```powershell
flang-new -fc1 -emit-hlfir examples\array_expression_temporary.f90
flang-new -fc1 -emit-fir examples\allocatable_reassignment.f90
```

Exact flags can vary across LLVM/Flang versions.

## Next Steps

1. Replace the textual scanner with MLIR parser APIs when LLVM/MLIR/Flang libraries are available.
2. Improve classification using real operation parents, operands, and dialect interfaces.
3. Add optimizer suggestions, such as rewriting expressions, avoiding unnecessary array sections, or preallocating allocatables.
4. Add JSON or CSV output for profiling reports.
