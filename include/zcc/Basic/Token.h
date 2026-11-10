#pragma once

#include <cstdint>

namespace zcc {

// One kind per distinct spelling the grammar cares about. The lab's coarse
// names (RO, LO, BRACE) are a printing concern, not a lexer concern.
enum class TokenKind : std::uint8_t {
  Eof,
  Invalid, // produced after a lexical error, only as recovery

  Identifier,
  IntegerLiteral,
  FloatLiteral,

  KwInt,
  KwDouble,
  KwScanf,
  KwPrintf,
  KwIf,
  KwThen,
  KwWhile,
  KwDo,

  Assign,       // =
  Equal,        // ==
  Greater,      // >
  GreaterEqual, // >=
  Less,         // <
  LessEqual,    // <=
  OrOr,         // ||
  AndAnd,       // &&
  Not,          // !
  NotEqual,     // !=
  Plus,
  Minus,
  Star,
  Slash,
  Comma,
  LParen,
  RParen,
  LBrace,
  RBrace,
  Semicolon,
};

// A view of the source text: kind + where it is. The text itself stays in
// SourceFile; use SourceFile::slice(tok.offset, tok.length) to get it.
struct Token {
  TokenKind kind;
  std::uint32_t offset;
  std::uint32_t length;
};

} // namespace zcc
