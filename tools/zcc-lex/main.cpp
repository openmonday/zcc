#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include "zcc/Basic/Diagnostic.h"
#include "zcc/Basic/SourceFile.h"
#include "zcc/Lex/Lexer.h"

namespace {

// The lab's exact wording lives here, not in the compiler core.
const char *labMessage(zcc::DiagID id) {
  switch (id) {
  case zcc::DiagID::MultipleDecimalPoints:
    return "Malformed number: More than one decimal point in a floating point "
           "number.";
  case zcc::DiagID::DecimalPointAtEnds:
    return "Malformed number: Decimal point at the beginning or end of a "
           "floating point number.";
  case zcc::DiagID::LeadingZerosInInteger:
    return "Malformed number: Leading zeros in an integer.";
  case zcc::DiagID::UnrecognizableCharacter:
    return "Unrecognizable characters.";
  }
  std::abort();
}

const char *labTypeName(zcc::TokenKind kind) {
  using zcc::TokenKind;
  switch (kind) {
  case TokenKind::Identifier:
    return "IDENT";
  case TokenKind::IntegerLiteral:
    return "INT";
  case TokenKind::FloatLiteral:
    return "DOUBLE";
  case TokenKind::KwInt:
    return "INTSYM";
  case TokenKind::KwDouble:
    return "DOUBLESYM";
  case TokenKind::KwScanf:
    return "SCANFSYM";
  case TokenKind::KwPrintf:
    return "PRINTFSYM";
  case TokenKind::KwIf:
    return "IFSYM";
  case TokenKind::KwThen:
    return "THENSYM";
  case TokenKind::KwWhile:
    return "WHILESYM";
  case TokenKind::KwDo:
    return "DOSYM";
  case TokenKind::Assign:
    return "AO";
  case TokenKind::Equal:
  case TokenKind::Greater:
  case TokenKind::GreaterEqual:
  case TokenKind::Less:
  case TokenKind::LessEqual:
  case TokenKind::NotEqual:
    return "RO";
  case TokenKind::OrOr:
  case TokenKind::AndAnd:
  case TokenKind::Not:
    return "LO";
  case TokenKind::Plus:
    return "PLUS";
  case TokenKind::Minus:
    return "MINUS";
  case TokenKind::Star:
    return "TIMES";
  case TokenKind::Slash:
    return "DIVISION";
  case TokenKind::Comma:
    return "COMMA";
  case TokenKind::LParen:
  case TokenKind::RParen:
  case TokenKind::LBrace:
  case TokenKind::RBrace:
    return "BRACE";
  case TokenKind::Semicolon:
    return "SEMICOLON";
  case TokenKind::Eof:
  case TokenKind::Invalid:
    break;
  }
  std::abort();
}

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
  zcc::DiagnosticEngine diags;
  zcc::Lexer lexer(src, diags);

  // Buffered: a lexical error prints only the error, never the tokens before
  // it.
  std::string out;
  for (;;) {
    zcc::Token tok = lexer.next();

    // Only the earliest error is printed, and only the error: the token lines
    // buffered so far are dropped. Exit status is always 0; the grader reads
    // stdout, and a non-zero status can be mistaken for a crash.
    if (!diags.empty()) {
      std::cout << labMessage(diags.first().id) << '\n';
      return 0;
    }
    if (tok.kind == zcc::TokenKind::Eof)
      break;

    std::string_view spelling = src.slice(tok.offset, tok.length);
    out.append(spelling.data(), spelling.size());
    out.push_back(' ');
    out.append(labTypeName(tok.kind));
    out.push_back('\n');
  }

  std::cout << out;
  return 0;
}
