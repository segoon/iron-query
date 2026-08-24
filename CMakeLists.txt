cmake_minimum_required(VERSION 3.16)

PROJECT(iron-query VERSION 0.1.0 LANGUAGES CXX)

enable_testing()
include(GoogleTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(iron-query STATIC src/iron_query.cpp)
target_include_directories(iron-query PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(iron-query PUBLIC cxx_std_17)
add_library(iron-query::iron-query ALIAS iron-query)

add_executable(iron-query-gtest tests/test.cpp)
target_link_libraries(iron-query-gtest gtest_main gtest iron-query)
gtest_discover_tests(iron-query-gtest)

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

