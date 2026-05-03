# Flang HLFIR/FIR Notes

## HLFIR

HLFIR is a higher-level Fortran-oriented MLIR dialect used in Flang lowering. It preserves useful Fortran semantics before lowering into lower-level FIR operations.

Important operation names for this project include:

- `hlfir.expr`
- `hlfir.assign`
- `hlfir.elemental`

## FIR

FIR is Flang's Fortran IR dialect. It represents Fortran-specific memory, descriptors, arrays, and procedure interfaces.

Important operation names for this project include:

- `fir.allocmem`
- `fir.freemem`
- `fir.array_load`
- `fir.array_merge_store`
- `fir.call`

## Source Locations

MLIR operations can carry source locations. The profiler should use those locations to report the Fortran file, line, and column related to each allocation.

