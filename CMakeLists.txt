cmake_minimum_required(VERSION 3.23)
project(flux)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
execute_process(
COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
OUTPUT_VARIABLE pybind11_DIR
OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_package(pybind11 REQUIRED)

pybind11_add_module(flux
engine/src/bindings.cpp
engine/src/indexer.cpp
engine/src/tokenizer.cpp
)

find_library(MUPDF_LIB mupdf REQUIRED)
target_link_libraries(flux PRIVATE ${MUPDF_LIB})

target_include_directories(flux PRIVATE engine/include)

enable_testing()
add_subdirectory(engine/tests)
