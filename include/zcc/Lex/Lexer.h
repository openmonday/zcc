#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "zcc/Basic/Diagnostic.h"
#include "zcc/Basic/SourceFile.h"
#include "zcc/Basic/Token.h"

namespace zcc {

// Checks one complete numeric literal and returns the highest-priority problem,
// or nothing if it is well formed. Exposed separately so it can be unit tested.
std::optional<DiagID> checkNumber(std::string_view spelling);

// Turns characters into tokens. Pull interface: next() consumes one token,
// peek() looks at the next one without consuming it. Never exits the process;
// problems go to the DiagnosticEngine.
class Lexer {
public:
  Lexer(const SourceFile &src, DiagnosticEngine &diags);

  Token next();
  const Token &peek();

private:
  Token lexRaw();
  void skipTrivia();
  Token lexNumber();
  Token lexIdentifier();

  char charAt(std::uint32_t index) const;
  bool startsWith(std::string_view s) const;

  std::string_view text_;
  DiagnosticEngine &diags_;
  std::uint32_t pos_ = 0;
  std::optional<Token> peeked_;
};

} // namespace zcc
