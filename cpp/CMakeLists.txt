cmake_minimum_required(VERSION 3.16)
project(SymbolicGridCpp VERSION 1.0.0 LANGUAGES CXX)

add_library(symbolic_grid STATIC
    src/GridCreator.cpp
    src/GridSegment.cpp
    src/segments/PartialSegmentHandler.cpp
    src/segments/detail/NodePool.cpp
    src/segments/detail/SegmentIndex.cpp
    src/segments/detail/SegmentIndexDiagnostics.cpp
    src/segments/detail/SegmentIndexRendering.cpp
    src/util/Random.cpp
)
add_library(symbolic::grid ALIAS symbolic_grid)
target_include_directories(symbolic_grid
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_compile_features(symbolic_grid PUBLIC cxx_std_17)
set_target_properties(symbolic_grid PROPERTIES POSITION_INDEPENDENT_CODE ON CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(symbolic_grid PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

option(SYMBOLIC_GRID_BUILD_TESTS "Build correctness tests" OFF)
option(SYMBOLIC_GRID_BUILD_EXAMPLES "Build the example" OFF)
option(SYMBOLIC_GRID_BUILD_BENCHMARKS "Build the standalone benchmark" OFF)
option(SYMBOLIC_GRID_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(SYMBOLIC_GRID_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "The sanitizer option requires GCC or Clang")
    endif()
    target_compile_options(symbolic_grid PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(symbolic_grid PUBLIC -fsanitize=address,undefined)
endif()

if(SYMBOLIC_GRID_BUILD_TESTS)
    enable_testing()
    add_executable(symbolic_grid_tests tests/grid_tests.cpp)
    target_link_libraries(symbolic_grid_tests PRIVATE symbolic::grid)
    add_test(NAME symbolic_grid_tests COMMAND symbolic_grid_tests)
endif()

if(SYMBOLIC_GRID_BUILD_EXAMPLES)
    add_executable(symbolic_grid_example examples/basic.cpp)
    target_link_libraries(symbolic_grid_example PRIVATE symbolic::grid)
endif()

if(SYMBOLIC_GRID_BUILD_BENCHMARKS)
    add_executable(symbolic_grid_benchmark benchmarks/grid_benchmark.cpp)
    target_link_libraries(symbolic_grid_benchmark PRIVATE symbolic::grid)
endif()
