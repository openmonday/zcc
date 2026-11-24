#!/bin/bash
# Builds the executable the grader runs: ./Main (the experiment-2 parser:
# lexer + LR(1) parser + translator).
# Keep the source list in sync with CMakeLists.txt when files are added.
set -eu

g++ -std=c++17 -O1 -Wall -Wextra -Iinclude \
  src/Basic/SourceFile.cpp \
  src/Lex/Lexer.cpp \
  src/Parse/Grammar.cpp \
  src/Parse/LR1Table.cpp \
  src/Parse/Translator.cpp \
  src/Parse/Parser.cpp \
  tools/zcc-parse/main.cpp \
  -o Main
