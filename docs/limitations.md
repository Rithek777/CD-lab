# Limitations

This repository is submission-ready as a compiler design lab prototype, but it is not a production Flang pass.

## Analyzer Limitations

- The current analyzer scans textual MLIR instead of using MLIR parser APIs.
- Def-use analysis is approximate and based on SSA value text matching.
- Classification is heuristic and depends on nearby operation names and markers.
- Static size estimation only handles simple visible `!fir.array<...>` shapes such as `!fir.array<1024x1024xf64>`.
- Dynamic shapes, descriptors, type parameters, and polymorphic values are reported with unknown size.
- Source locations are captured only from simple `loc("file":line:column)` metadata.

## Transformer Limitations

- `scripts/transform_fortran.py` is not a full Fortran parser.
- It handles only simple one-dimensional assignments:
  - `A = B + C`
  - `A = B * C`
  - `A = B + scalar`
  - `A(:) = B(:) + C(:)`
- It intentionally skips calls, derived-type components, complex indexing, reductions, and unclear scalar assignments.
- It assumes loop index `i` is acceptable in the local scope.

## Evaluation Limitations

- Timing results are machine-dependent.
- Windows process startup overhead can dominate small benchmarks.
- `/usr/bin/time`, Valgrind Massif, and `LD_PRELOAD` allocation counting are generally Linux-oriented and may be unavailable on Windows.
- The generated transformed benchmark files in `build/evaluation` are build artifacts, not source files to edit by hand.

## Future Integration Path

A production version should use MLIR APIs to parse modules, walk operations, inspect operands, perform real def-use analysis, and attach diagnostics to exact source locations.
