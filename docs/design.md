# Design Notes

## Objective

The tool will analyze MLIR emitted by LLVM Flang and identify implicit heap allocations that are easy to miss at the Fortran source level.

## Intended Pipeline

1. Accept an HLFIR/FIR MLIR file produced by Flang.
2. Parse the module using MLIR infrastructure.
3. Walk operations and record allocation operations such as `fir.allocmem`.
4. Inspect surrounding HLFIR/FIR context to classify why an allocation was introduced.
5. Use source location metadata to map reports back to Fortran code.
6. Emit beginner-friendly diagnostics and optimization suggestions.

## Allocation Categories

### Array Expression Temporaries

Operations such as `hlfir.expr` and `hlfir.elemental` may represent lowered array expressions. If their lowering introduces `fir.allocmem`, the tool should report a temporary array allocation.

### Array-Valued Function Results

Function calls returning arrays may require result storage. The analysis should distinguish user-visible result allocation from compiler-created temporary storage.

### Allocatable Assignment Reallocation

Assignments to allocatable arrays may trigger automatic reallocation when shape, type parameters, or allocation state require it. HLFIR assignment operations such as `hlfir.assign` are important context.

### Copy-In/Copy-Out Temporaries

Passing non-contiguous array sections to procedures may force temporary buffers. The future pass should inspect array section operations and call argument lowering.

## Future Optimizer Ideas

- Suggest preallocating allocatable arrays when shape is known.
- Suggest replacing complex whole-array expressions with explicit loops for memory-sensitive kernels.
- Warn about non-contiguous sections passed to procedures expecting contiguous data.
- Show paired `fir.allocmem` and `fir.freemem` regions to explain temporary lifetimes.

