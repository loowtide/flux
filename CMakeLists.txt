cmake_minimum_required(VERSION 3.23)
project(flux)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

enable_testing()
add_subdirectory(engine/tests)
