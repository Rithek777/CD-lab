add_test(NAME profiler_help COMMAND flang-implicit-alloc-profiler --help)
add_test(NAME profiler_simple_alloc
  COMMAND flang-implicit-alloc-profiler
          ${CMAKE_CURRENT_SOURCE_DIR}/simple_alloc.mlir)
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

add_test(NAME report_text_format
  COMMAND flang-implicit-alloc-profiler
          --format=text
          ${CMAKE_CURRENT_SOURCE_DIR}/array_expression_temporary.mlir)
set_tests_properties(report_text_format PROPERTIES
  PASS_REGULAR_EXPRESSION "classification: PROVABLY_UNNECESSARY")

add_test(NAME report_json_format
  COMMAND flang-implicit-alloc-profiler
          --format=json
          ${CMAKE_CURRENT_SOURCE_DIR}/array_expression_temporary.mlir)
set_tests_properties(report_json_format PROPERTIES
  PASS_REGULAR_EXPRESSION "\"classification\": \"PROVABLY_UNNECESSARY\"")

add_test(NAME report_show_ir
  COMMAND flang-implicit-alloc-profiler
          --show-ir
          ${CMAKE_CURRENT_SOURCE_DIR}/array_expression_temporary.mlir)
set_tests_properties(report_show_ir PROPERTIES
  PASS_REGULAR_EXPRESSION "ir: %tmp = fir.allocmem")

add_test(NAME report_threshold
  COMMAND flang-implicit-alloc-profiler
          --threshold-mb=9
          ${CMAKE_CURRENT_SOURCE_DIR}/array_expression_temporary.mlir)
set_tests_properties(report_threshold PROPERTIES
  PASS_REGULAR_EXPRESSION "No allocations met the reporting threshold")
