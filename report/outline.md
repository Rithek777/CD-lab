# Report Outline

## Title

Flang Implicit Allocation Profiler and Optimizer

## Problem Statement

Fortran array syntax is expressive, but compilers may introduce hidden heap allocations while lowering array expressions, array-valued function results, allocatable assignments, and non-contiguous array sections.

## Proposed Tool

The project proposes an LLVM/MLIR-style analysis tool that scans Flang HLFIR/FIR IR and reports likely implicit allocation sites.

## Methodology

1. Generate HLFIR/FIR from representative Fortran examples.
2. Parse MLIR and walk operations.
3. Detect `fir.allocmem` and related deallocation operations.
4. Classify allocation causes using nearby HLFIR/FIR operations.
5. Report source locations and optimization suggestions.

## Expected Outcome

A beginner-friendly profiler that helps students and developers understand where Fortran code may allocate temporary heap memory after Flang lowering.

