#pragma once

#include <string>

#include "zcc/Basic/SourceFile.h"

namespace zcc {

// Lexes, parses with the LR(1) table and translates the program. Returns the
// report (symbol table, temporary count, quadruples), newline-terminated.
// Throws SyntaxError on any error.
std::string compileToQuads(const SourceFile &src);

} // namespace zcc
