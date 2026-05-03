#!/usr/bin/env sh
set -eu

example="${1:-examples/array_expression_temporary.f90}"
output="${2:-build/example.hlfir.mlir}"

mkdir -p "$(dirname "$output")"
flang-new -fc1 -emit-hlfir "$example" -o "$output"

echo "Wrote $output"

