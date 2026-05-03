cmake_minimum_required(VERSION 3.20)

project(
  FlangImplicitAllocationProfiler
  VERSION 0.1.0
  DESCRIPTION "LLVM/MLIR-style profiler for implicit allocations in Flang FIR/HLFIR IR"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MINGW)
  set(CMAKE_CXX_LINK_EXECUTABLE
      "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

option(FAP_ENABLE_LLVM "Enable LLVM/MLIR/Flang integration when those packages are installed" OFF)
option(FAP_BUILD_TESTS "Build project tests" ON)

add_library(faprof_core OBJECT
  src/AnalysisDriver.cpp
  src/AllocationClassifier.cpp
  src/AllocationRecord.cpp
  src/Report.cpp)

target_include_directories(faprof_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(faprof_core PUBLIC cxx_std_17)

if(FAP_ENABLE_LLVM)
  find_package(LLVM REQUIRED CONFIG)
  find_package(MLIR REQUIRED CONFIG)

  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  message(STATUS "Using MLIRConfig.cmake in: ${MLIR_DIR}")

  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")

  include(AddLLVM)
  include(AddMLIR)

  target_compile_definitions(faprof_core PUBLIC FAP_ENABLE_LLVM=1)
  target_include_directories(faprof_core SYSTEM PUBLIC
    ${LLVM_INCLUDE_DIRS}
    ${MLIR_INCLUDE_DIRS})
endif()

add_executable(flang-implicit-alloc-profiler
  src/main.cpp
  $<TARGET_OBJECTS:faprof_core>)
target_link_libraries(flang-implicit-alloc-profiler PRIVATE faprof_core)

if(FAP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
