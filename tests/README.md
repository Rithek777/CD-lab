# Tests

This directory contains sample MLIR fixtures and CTest entries for the allocation profiler.

Current fixtures:

- `array_expression_temporary.mlir`
- `allocatable_reassignment.mlir`
- `array_section_copy.mlir`
- `function_result_temporary.mlir`

They are fake but realistic HLFIR/FIR snippets designed to exercise `fir.allocmem`, `fir.freemem`, source locations, static array type size estimates, and allocation classification.
