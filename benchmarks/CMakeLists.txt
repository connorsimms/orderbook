add_executable(benchmarks orderbook_benchmark.cpp)

target_link_libraries(benchmarks PRIVATE orderbook_lib)
