# Flang Implicit Allocation Profiler and Optimizer

This repository is a starter skeleton for a CD lab assignment focused on implicit heap allocations in LLVM Flang IR.

The final tool will scan Flang HLFIR/FIR IR and report allocation sites that may come from:

- array expression temporaries
- array-valued function results
- automatic reallocation on allocatable assignment
- copy-in/copy-out or array section temporaries

The first version intentionally does not implement the full analysis yet. It provides the repository layout, CMake build system, placeholder C++ entry points, documentation, and example Fortran programs.

## Project Layout

```text
.
├── CMakeLists.txt
├── include/
│   └── FAP/
├── src/
├── tests/
├── examples/
├── scripts/
├── docs/
└── report/
```

## Planned IR Focus

The analysis will later inspect Flang-generated HLFIR/FIR and look for operations and patterns such as:

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

Run the placeholder executable:

```powershell
.\build\Debug\faprof.exe --help
```

On single-config generators such as Ninja or Makefiles, the executable may be here instead:

```powershell
.\build\faprof.exe --help
```

## Optional LLVM/MLIR Mode

If LLVM, MLIR, and Flang development packages are available on your system, configure with:

```powershell
cmake -S . -B build -DFAP_ENABLE_LLVM=ON -DMLIR_DIR="path\to\mlir\cmake" -DLLVM_DIR="path\to\llvm\cmake"
cmake --build build
```

For this first skeleton, LLVM mode only prepares the build for future integration. The real HLFIR/FIR pass logic will be added later.

## Example Flang Commands

When Flang is installed, you can generate HLFIR/FIR-like MLIR for the examples with commands similar to:

```powershell
flang-new -fc1 -emit-hlfir examples\array_expression_temporary.f90
flang-new -fc1 -emit-fir examples\allocatable_reassignment.f90
```

Exact flags can vary across LLVM/Flang versions.

## Next Steps

1. Add an MLIR parser or pass entry point.
2. Walk operations in a module and identify `fir.allocmem` / `fir.freemem` pairs.
3. Classify likely causes using nearby HLFIR/FIR operations such as `hlfir.expr`, `hlfir.assign`, and `hlfir.elemental`.
4. Use source location metadata to report file, line, and column.
5. Add optimizer suggestions, such as rewriting expressions, avoiding unnecessary array sections, or preallocating allocatables.

