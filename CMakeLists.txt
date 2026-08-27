cmake_minimum_required(VERSION 3.16)

PROJECT(iron-query VERSION 0.1.0 LANGUAGES CXX)

# Only build the test binary when this is the top-level project: examples/
# and downstream consumers pull in just the `iron-query` target via
# add_subdirectory() and should not need GTest as a transitive dependency.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(IRON_QUERY_IS_TOP_LEVEL TRUE)
else()
  set(IRON_QUERY_IS_TOP_LEVEL FALSE)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Warnings are errors: a silently-reordered member initialiser once shipped
# as a real bug here.
set(IRON_QUERY_WARNINGS
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall;-Wextra;-Werror>)

file(GLOB IRON_QUERY_SOURCES CONFIGURE_DEPENDS src/*.cpp src/detail/*.cpp)
add_library(iron-query STATIC ${IRON_QUERY_SOURCES})
target_compile_options(iron-query PRIVATE ${IRON_QUERY_WARNINGS})
target_include_directories(iron-query PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(iron-query PUBLIC cxx_std_17)
add_library(iron-query::iron-query ALIAS iron-query)

if(IRON_QUERY_IS_TOP_LEVEL)
  enable_testing()
  include(GoogleTest)

  add_executable(iron-query-gtest tests/test.cpp)
  target_compile_options(iron-query-gtest PRIVATE ${IRON_QUERY_WARNINGS})
  target_link_libraries(iron-query-gtest gtest_main gtest iron-query)
  gtest_discover_tests(iron-query-gtest)
endif()

install(TARGETS iron-query EXPORT iron-query-targets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT iron-query-targets
  FILE iron-query-targets.cmake
  NAMESPACE iron-query::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iron-query)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/iron-query-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/iron-query-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iron-query)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/iron-query-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  ARCH_INDEPENDENT)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/iron-query-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/iron-query-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iron-query)

