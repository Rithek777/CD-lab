# CMake generated Testfile for 
# Source directory: C:/Users/jprit/OneDrive/Desktop/CD lab/tests
# Build directory: C:/Users/jprit/OneDrive/Desktop/CD lab/build-mingw/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[profiler_help]=] "C:/Users/jprit/OneDrive/Desktop/CD lab/build-mingw/flang-implicit-alloc-profiler.exe" "--help")
set_tests_properties([=[profiler_help]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;1;add_test;C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;0;")
add_test([=[profiler_array_expr]=] "C:/Users/jprit/OneDrive/Desktop/CD lab/build-mingw/flang-implicit-alloc-profiler.exe" "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/array_expression_temporary.mlir")
set_tests_properties([=[profiler_array_expr]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;2;add_test;C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;0;")
add_test([=[profiler_allocatable_assign]=] "C:/Users/jprit/OneDrive/Desktop/CD lab/build-mingw/flang-implicit-alloc-profiler.exe" "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/allocatable_reassignment.mlir")
set_tests_properties([=[profiler_allocatable_assign]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;5;add_test;C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;0;")
add_test([=[profiler_section_copy]=] "C:/Users/jprit/OneDrive/Desktop/CD lab/build-mingw/flang-implicit-alloc-profiler.exe" "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/array_section_copy.mlir")
set_tests_properties([=[profiler_section_copy]=] PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;8;add_test;C:/Users/jprit/OneDrive/Desktop/CD lab/tests/CMakeLists.txt;0;")
