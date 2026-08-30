cmake_minimum_required(VERSION 3.16)

project(iron-query-libpq-example CXX)

find_package(PostgreSQL REQUIRED)

# Pulls in just the `iron-query` target: the top-level CMakeLists.txt skips
# its GTest-based test binary when included this way.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. iron-query-build)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE PostgreSQL::PostgreSQL iron-query::iron-query)
