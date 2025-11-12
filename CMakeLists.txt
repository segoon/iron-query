PROJECT(sql-builder)

enable_testing()
include(GoogleTest)

add_library(sql-builder-pp INTERFACE)
target_include_directories(sql-builder-pp INTERFACE include)
# set_target_properties(sql-builder-pp PROPERTIES LANGUAGE CXX)

add_executable(test tests/test.cpp)
target_link_libraries(test gtest_main gtest sql-builder-pp)
gtest_discover_tests(test)

