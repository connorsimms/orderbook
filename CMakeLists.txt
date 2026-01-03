cmake_minimum_required(VERSION 4.2)
project(orderbook VERSION 0.0.1 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(orderbook_lib INTERFACE)

target_include_directories(orderbook_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(orderbook_lib INTERFACE -Wall -Wextra -Wpedantic)

add_subdirectory(tests)
add_subdirectory(benchmarks)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(TIDY_CHECKS "-*,google-*,performance-*,bugprone-*,-readability-magic-numbers,-misc-include-cleaner")
    
    set_target_properties(
        orderbook_test PROPERTIES 
        CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${TIDY_CHECKS}"
    )
endif()

