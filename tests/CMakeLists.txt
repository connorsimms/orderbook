include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
  SYSTEM
)

FetchContent_MakeAvailable(googletest)

add_executable(
    orderbook_test 
    orderbook_test.cpp
)

target_link_libraries(orderbook_test PRIVATE
  orderbook_lib
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(orderbook_test)
