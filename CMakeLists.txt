cmake_minimum_required(VERSION 4.2)

project(orderbook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
enable_testing()

add_executable( orderbook_test
  orderbook_test.cpp
)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

if(CLANG_TIDY_EXE)
    set(TIDY_CHECKS "-*,google-*,performance-*,bugprone-*, -readability-magic-numbers, -misc-include-cleaner")
    
    set_target_properties(
        orderbook_test PROPERTIES 
        CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=${TIDY_CHECKS}"
    )
endif()

find_package(GTest REQUIRED)
target_link_libraries( orderbook_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(orderbook_test)
