PROJECT(sql-builder)

enable_testing()

include(GoogleTest)

add_executable(test src/test.cpp)
target_link_libraries(test gtest_main gtest)
target_include_directories(test PRIVATE include)
gtest_discover_tests(test)

