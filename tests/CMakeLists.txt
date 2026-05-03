add_test(NAME profiler_help COMMAND flang-implicit-alloc-profiler --help)
add_test(NAME profiler_array_expr
  COMMAND flang-implicit-alloc-profiler
          ${CMAKE_CURRENT_SOURCE_DIR}/array_expression_temporary.mlir)
add_test(NAME profiler_allocatable_assign
  COMMAND flang-implicit-alloc-profiler
          ${CMAKE_CURRENT_SOURCE_DIR}/allocatable_reassignment.mlir)
add_test(NAME profiler_section_copy
  COMMAND flang-implicit-alloc-profiler
          ${CMAKE_CURRENT_SOURCE_DIR}/array_section_copy.mlir)
add_test(NAME profiler_function_result
  COMMAND flang-implicit-alloc-profiler
          ${CMAKE_CURRENT_SOURCE_DIR}/function_result_temporary.mlir)
