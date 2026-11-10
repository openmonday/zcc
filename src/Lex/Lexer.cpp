#include "zcc/Lex/Lexer.h"

#include <string_view>

namespace zcc {
namespace {

bool isDigit(char c) { return c >= '0' && c <= '9'; }

// The lab grammar says letters only, so '_' is not an identifier character.
bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' ||
         c == '\f';
}

TokenKind keywordKind(std::string_view spelling) {
  struct Entry {
    std::string_view spelling;
    TokenKind kind;
  };
  static const Entry table[] = {
      {"int", TokenKind::KwInt},     {"double", TokenKind::KwDouble},
      {"scanf", TokenKind::KwScanf}, {"printf", TokenKind::KwPrintf},
      {"if", TokenKind::KwIf},       {"then", TokenKind::KwThen},
      {"while", TokenKind::KwWhile}, {"do", TokenKind::KwDo},
  };
  for (const Entry& e : table)
    if (e.spelling == spelling) return e.kind;
  return TokenKind::Identifier;
}

} // namespace

std::optional<DiagID> checkNumber(std::string_view spelling) {
  // The three checks are ordered by the lab's priority, not by position.
  int dots = 0;
  for (char c : spelling)
    if (c == '.') ++dots;

  if (dots > 1) return DiagID::MultipleDecimalPoints;
  if (spelling.front() == '.' || spelling.back() == '.')
    return DiagID::DecimalPointAtEnds;

  std::string_view intPart = spelling.substr(0, spelling.find('.'));
  if (intPart.size() > 1 && intPart.front() == '0')
    return DiagID::LeadingZerosInInteger;

  return std::nullopt;
}

Lexer::Lexer(const SourceFile& src, DiagnosticEngine& diags)
    : text_(src.text()), diags_(diags) {}

char Lexer::charAt(std::uint32_t index) const {
  return index < text_.size() ? text_[index] : '\0';
}

bool Lexer::startsWith(std::string_view s) const {
  return text_.substr(pos_, s.size()) == s;
}

Token Lexer::next() {
  if (peeked_) {
    Token tok = *peeked_;
    peeked_.reset();
    return tok;
  }
  return lexRaw();
}

const Token& Lexer::peek() {
  if (!peeked_) peeked_ = lexRaw();
  return *peeked_;
}

void Lexer::skipTrivia() {
  for (;;) {
    while (isSpace(charAt(pos_))) ++pos_;

    if (startsWith("//")) {
      pos_ += 2;
      while (pos_ < text_.size() && text_[pos_] != '\n') ++pos_;
      continue;
    }

    if (startsWith("/*")) {
      std::size_t end = text_.find("*/", pos_ + 2);
      // No closing */ means the rest of the file is a comment: not an error.
      pos_ = (end == std::string_view::npos)
                 ? static_cast<std::uint32_t>(text_.size())
                 : static_cast<std::uint32_t>(end + 2);
      continue;
    }

    return;
  }
}

Token Lexer::lexRaw() {
  skipTrivia();

  const std::uint32_t start = pos_;
  if (pos_ >= text_.size()) return Token{TokenKind::Eof, start, 0};

  const char c = text_[pos_];

  if (isDigit(c)) return lexNumber();
  // A leading '.' starts a number as well (sample 5: ".0199" is a malformed
  // number, not an unknown character), so a lone '.' is a malformed number too.
  if (c == '.') return lexNumber();
  if (isAlpha(c)) return lexIdentifier();

  // Longest match first: two-character operators, then one-character ones.
  struct TwoChar {
    std::string_view spelling;
    TokenKind kind;
  };
  static const TwoChar twoChar[] = {
      {"==", TokenKind::Equal},     {">=", TokenKind::GreaterEqual},
      {"<=", TokenKind::LessEqual}, {"||", TokenKind::OrOr},
      {"&&", TokenKind::AndAnd},    {"!=", TokenKind::NotEqual},
  };
  for (const TwoChar& op : twoChar) {
    if (startsWith(op.spelling)) {
      pos_ += 2;
      return Token{op.kind, start, 2};
    }
  }

  struct OneChar {
    char spelling;
    TokenKind kind;
  };
  static const OneChar oneChar[] = {
      {'=', TokenKind::Assign}, {'>', TokenKind::Greater},
      {'<', TokenKind::Less},   {'!', TokenKind::Not},
      {'+', TokenKind::Plus},   {'-', TokenKind::Minus},
      {'*', TokenKind::Star},   {'/', TokenKind::Slash},
      {',', TokenKind::Comma},  {'(', TokenKind::LParen},
      {')', TokenKind::RParen}, {'{', TokenKind::LBrace},
      {'}', TokenKind::RBrace}, {';', TokenKind::Semicolon},
  };
  for (const OneChar& op : oneChar) {
    if (c == op.spelling) {
      ++pos_;
      return Token{op.kind, start, 1};
    }
  }

  // Recovery for the compiler: report, skip one character, keep going.
  diags_.report(DiagID::UnrecognizableCharacter, start, 1);
  ++pos_;
  return Token{TokenKind::Invalid, start, 1};
}

Token Lexer::lexNumber() {
  const std::uint32_t start = pos_;
  while (isDigit(charAt(pos_)) || charAt(pos_) == '.') ++pos_;

  const std::uint32_t length = pos_ - start;
  std::string_view spelling = text_.substr(start, length);
  if (auto problem = checkNumber(spelling)) {
    diags_.report(*problem, start, length);
    return Token{TokenKind::Invalid, start, length};
  }

  const bool isFloat = spelling.find('.') != std::string_view::npos;
  return Token{isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral,
               start, length};
}

Token Lexer::lexIdentifier() {
  const std::uint32_t start = pos_;
  while (isAlpha(charAt(pos_)) || isDigit(charAt(pos_))) ++pos_;

  const std::uint32_t length = pos_ - start;
  TokenKind kind = keywordKind(text_.substr(start, length));
  return Token{kind, start, length};
}

} // namespace zcc
