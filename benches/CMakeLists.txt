set(BENCHMARK_ENABLE_TESTING OFF)

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG origin/main
)

FetchContent_MakeAvailable(benchmark)

include_directories(..)

add_executable(run_benches main.cpp)

target_link_libraries(run_benches benchmark::benchmark)
