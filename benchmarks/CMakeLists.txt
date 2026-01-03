include (FetchContent)

FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/eddb0241389718a23a42db6af5f0164b6e0139af.zip
    SYSTEM
)

FetchContent_MakeAvailable(googlebenchmark)

add_executable(
    orderbook_benchmark
    orderbook_benchmark.cpp
)

target_link_libraries(orderbook_benchmark PRIVATE
    orderbook_lib
    benchmark::benchmark
    benchmark::benchmark_main
)

target_compile_options(orderbook_benchmark PRIVATE -O3 -march=native)

