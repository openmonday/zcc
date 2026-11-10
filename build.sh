#!/bin/bash
# Builds the executable the grader runs: ./Main
# Keep the source list in sync with CMakeLists.txt when files are added.
set -eu

g++ -std=c++17 -O1 -Wall -Wextra -Iinclude -c src/Basic/SourceFile.cpp -o source_file.o
g++ -std=c++17 -O1 -Wall -Wextra -Iinclude -c src/Lex/Lexer.cpp -o lexer.o
g++ -std=c++17 -O1 -Wall -Wextra -Iinclude -c tools/zcc-lex/main.cpp -o main.o

g++ source_file.o lexer.o main.o -o Main -lm
