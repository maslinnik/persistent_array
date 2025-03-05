cmake_minimum_required(VERSION 3.27)
project(Project)

add_compile_definitions(LOCAL)

set(CMAKE_CXX_STANDARD 20)

enable_testing()
find_package(GTest REQUIRED)
include_directories(. ${GTEST_INCLUDE_DIR})

add_executable(run_tests tests.cpp)

target_link_libraries(run_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)
