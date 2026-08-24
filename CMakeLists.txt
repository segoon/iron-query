cmake_minimum_required(VERSION 3.16)

PROJECT(sql-builder VERSION 0.1.0 LANGUAGES CXX)

enable_testing()
include(GoogleTest)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(sql-builder-pp INTERFACE)
target_include_directories(sql-builder-pp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_compile_features(sql-builder-pp INTERFACE cxx_std_17)
add_library(sql-builder-pp::sql-builder-pp ALIAS sql-builder-pp)

cmake_policy(SET CMP0037 OLD) # allow target name "test" despite CTest reserving it
add_executable(test tests/test.cpp)
target_link_libraries(test gtest_main gtest sql-builder-pp)
gtest_discover_tests(test)

install(TARGETS sql-builder-pp EXPORT sql-builder-pp-targets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT sql-builder-pp-targets
  FILE sql-builder-pp-targets.cmake
  NAMESPACE sql-builder-pp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sql-builder-pp)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sql-builder-pp-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/sql-builder-pp-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sql-builder-pp)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/sql-builder-pp-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  ARCH_INDEPENDENT)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/sql-builder-pp-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/sql-builder-pp-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sql-builder-pp)

