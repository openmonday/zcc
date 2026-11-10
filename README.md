# zcc

`zcc` is a personal C/C++ compiler project built for learning.

## Purpose

This project exists to understand how compilers work from the inside out — not to
replace GCC or Clang. The goal is to implement the pieces of a real compiler
pipeline step by step:

- **Lexing** — turning source text into tokens
- **Parsing** — building an abstract syntax tree from tokens
- **Semantic analysis** — resolving names, checking types, and enforcing rules
- **Code generation** — lowering the AST into executable output
- **Optimization** — improving the generated code (eventually)

## Status

Early days. The design and implementation will evolve as I learn, so expect
rough edges, rewrites, and experiments.

## License

This is a learning project. Feel free to read the code, but it is not intended
for production use.
