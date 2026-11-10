cmake_minimum_required(VERSION 3.20)
project(zcc CXX)

# Clangd reads this to resolve includes and compile flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(zcc_basic STATIC
  src/Basic/SourceFile.cpp
)
target_include_directories(zcc_basic PUBLIC include)

add_library(zcc_lex STATIC
  src/Lex/Lexer.cpp
)
target_link_libraries(zcc_lex PUBLIC zcc_basic)

add_executable(zcc-lex tools/zcc-lex/main.cpp)
target_link_libraries(zcc-lex PRIVATE zcc_lex)

enable_testing()
add_test(NAME lexer-samples
         COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/run-samples.sh
                 $<TARGET_FILE:zcc-lex>)
