cmake_minimum_required(VERSION 3.16)

project(iron-query-userver-example CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)


find_package(userver COMPONENTS postgresql QUIET)
if(NOT userver_FOUND)
  set(USERVER_FEATURE_POSTGRESQL ON CACHE BOOL "" FORCE)

  # Tries TRY_DIR first, falls back to downloading userver from GitHub.
  download_userver(TRY_DIR third_party/userver)
endif()
userver_setup_environment()

# Pulls in just the `iron-query` target: the top-level CMakeLists.txt skips
# its GTest-based test binary when included this way.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. iron-query-build)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE userver::postgresql iron-query::iron-query)
