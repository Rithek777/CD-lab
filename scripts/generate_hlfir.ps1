param(
  [string]$Example = "examples\array_expression_temporary.f90",
  [string]$Output = "build\example.hlfir.mlir"
)

New-Item -ItemType Directory -Force -Path (Split-Path $Output) | Out-Null

flang-new -fc1 -emit-hlfir $Example -o $Output

Write-Host "Wrote $Output"

