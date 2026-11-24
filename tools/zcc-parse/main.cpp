#include <fstream>
#include <iostream>

#include "zcc/Basic/SourceFile.h"
#include "zcc/Parse/Parser.h"
#include "zcc/Parse/Translator.h"

namespace {

// Reads stdin, or the file named by argv[1] if one is given.
zcc::SourceFile readInput(int argc, char **argv) {
  if (argc > 1) {
    std::ifstream in(argv[1], std::ios::binary);
    if (!in) {
      std::cerr << "cannot open " << argv[1] << '\n';
      std::exit(2);
    }
    return zcc::SourceFile::readAll(in, argv[1]);
  }
  return zcc::SourceFile::readAll(std::cin);
}

} // namespace

int main(int argc, char **argv) {
  zcc::SourceFile src = readInput(argc, argv);
  try {
    std::cout << zcc::compileToQuads(src);
  } catch (const zcc::SyntaxError &) {
    // The lab's format: no trailing newline after the error line.
    std::cout << "Syntax Error";
  }
  return 0;
}
