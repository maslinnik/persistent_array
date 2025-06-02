enable_testing()
find_package(GTest REQUIRED)
include_directories(.. ${GTEST_INCLUDE_DIR})

add_executable(run_tests unit.cpp stress.cpp)

target_link_libraries(run_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(run_tests)
