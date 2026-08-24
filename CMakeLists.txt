cmake_minimum_required(VERSION 3.16)

PROJECT(iron-query VERSION 0.1.0 LANGUAGES CXX)

enable_testing()
include(GoogleTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(iron-query INTERFACE)
target_include_directories(iron-query INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(iron-query INTERFACE cxx_std_17)
add_library(iron-query::iron-query ALIAS iron-query)

cmake_policy(SET CMP0037 OLD) # allow target name "test" despite CTest reserving it
add_executable(test tests/test.cpp)
target_link_libraries(test gtest_main gtest iron-query)
gtest_discover_tests(test)

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

