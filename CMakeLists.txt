cmake_minimum_required(VERSION 3.27)
project(Project)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(tests)
add_subdirectory(benches)

add_executable(example example.cpp)