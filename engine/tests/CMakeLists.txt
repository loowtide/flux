cmake_minimum_required(VERSION 3.23)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)

find_library(MUPDF_LIB NAMES mupdf REQUIRED
)
find_library(MUPDF_THIRD_LIB NAMES mupdf-third)
find_package(Freetype REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
pkg_check_modules(HARFBUZZ harfbuzz)
find_library(OPENJP2_LIB  NAMES openjp2)
find_library(JBIG2DEC_LIB NAMES jbig2dec)
find_library(GUMBO_LIB    NAMES gumbo)
find_library(MUJS_LIB     NAMES mujs)

function(add_unit_test target)

  add_executable(${target}
  ${ARGN}
)

  target_include_directories(
${target}
PRIVATE
../include
)

  target_compile_definitions(
${target}
PRIVATE
TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)


  target_link_libraries(${target}
PRIVATE
GTest::gtest_main
-Wl,--start-group
   ${MUPDF_LIB}
   $<$<BOOL:${MUPDF_THIRD_LIB}>:${MUPDF_THIRD_LIB}>
   -Wl,--end-group
   Freetype::Freetype
   ZLIB::ZLIB
   JPEG::JPEG
   ${HARFBUZZ_LIBRARIES}
   ${OPENJP2_LIB}
   ${JBIG2DEC_LIB}
   ${GUMBO_LIB}
   ${MUJS_LIB}
)

  include(GoogleTest)
  gtest_discover_tests(${target})
endfunction()

add_unit_test(
tokenizer_test
tokenizer_test.cpp
../src/tokenizer.cpp
)

add_unit_test(
indexer_test
indexer_test.cpp
../src/indexer.cpp
#../src/tokenizer.cpp
)
