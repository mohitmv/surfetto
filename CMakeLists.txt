cmake_minimum_required(VERSION 3.1)

project(
  surfetto
  VERSION 1.0
  LANGUAGES CXX)

add_library(surfetto STATIC surfetto.cpp surfetto.h)
target_compile_options(surfetto PRIVATE -std=c++17 -O3)

if (${BUILD_SURFETTO_TESTS})
  add_executable(surfetto_test1 tests/surfetto_test1.cpp)
  target_compile_options(surfetto_test1 PRIVATE -std=c++17 -O3)
  target_include_directories(surfetto_test1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(surfetto_test1 PRIVATE surfetto)

  add_executable(surfetto_example tests/surfetto_example.cpp)
  target_compile_options(surfetto_example PRIVATE -std=c++17 -O3)
  target_include_directories(surfetto_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(surfetto_example PRIVATE surfetto)
endif()
