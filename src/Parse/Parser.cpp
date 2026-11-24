#include "zcc/Parse/Parser.h"

#include <utility>
#include <vector>

#include "zcc/Basic/Diagnostic.h"
#include "zcc/Lex/Lexer.h"
#include "zcc/Parse/Grammar.h"
#include "zcc/Parse/LR1Table.h"
#include "zcc/Parse/Translator.h"

namespace zcc {
namespace {

// The grammar's name for a token. Invalid tokens have no terminal.
std::string terminalName(TokenKind kind) {
  switch (kind) {
  case TokenKind::Eof:
    return "$";
  case TokenKind::Identifier:
    return "id";
  case TokenKind::IntegerLiteral:
    return "UINT";
  case TokenKind::FloatLiteral:
    return "UFLOAT";
  case TokenKind::KwInt:
    return "int";
  case TokenKind::KwDouble:
    return "double";
  case TokenKind::KwScanf:
    return "scanf";
  case TokenKind::KwPrintf:
    return "printf";
  case TokenKind::KwIf:
    return "if";
  case TokenKind::KwThen:
    return "then";
  case TokenKind::KwWhile:
    return "while";
  case TokenKind::KwDo:
    return "do";
  case TokenKind::Assign:
    return "=";
  case TokenKind::Equal:
    return "==";
  case TokenKind::Greater:
    return ">";
  case TokenKind::GreaterEqual:
    return ">=";
  case TokenKind::Less:
    return "<";
  case TokenKind::LessEqual:
    return "<=";
  case TokenKind::OrOr:
    return "||";
  case TokenKind::AndAnd:
    return "&&";
  case TokenKind::Not:
    return "!";
  case TokenKind::NotEqual:
    return "!=";
  case TokenKind::Plus:
    return "+";
  case TokenKind::Minus:
    return "-";
  case TokenKind::Star:
    return "*";
  case TokenKind::Slash:
    return "/";
  case TokenKind::Comma:
    return ",";
  case TokenKind::LParen:
    return "(";
  case TokenKind::RParen:
    return ")";
  case TokenKind::LBrace:
    return "{";
  case TokenKind::RBrace:
    return "}";
  case TokenKind::Semicolon:
    return ";";
  case TokenKind::Invalid:
    break;
  }
  throw SyntaxError{};
}

struct InputSymbol {
  int terminal;
  Attr attr;
};

} // namespace

std::string compileToQuads(const SourceFile &src) {
  static const LR1Table table(labGrammar());

  // Tokenize everything up front; any lexical problem is a syntax error here.
  DiagnosticEngine diags;
  Lexer lexer(src, diags);
  std::vector<InputSymbol> input;
  for (;;) {
    Token tok = lexer.next();
    if (!diags.empty()) throw SyntaxError{};
    int terminal = table.terminal(terminalName(tok.kind));
    if (terminal < 0) throw SyntaxError{};
    Attr attr;
    attr.lexeme = std::string(src.slice(tok.offset, tok.length));
    input.push_back(InputSymbol{terminal, std::move(attr)});
    if (tok.kind == TokenKind::Eof) break;
  }

  Translator translator;
  std::vector<int> states{0};
  std::vector<Attr> values{Attr{}}; // parallel to states
  std::size_t pos = 0;

  for (;;) {
    const int state = states.back();
    const LR1Table::Action act = table.action(state, input[pos].terminal);
    switch (act.kind) {
    case LR1Table::ActionKind::Shift:
      states.push_back(act.value);
      values.push_back(input[pos].attr);
      ++pos;
      break;

    case LR1Table::ActionKind::Reduce: {
      const int production = act.value;
      const std::size_t n = table.rhsSize(production);
      std::vector<Attr> rhs(values.end() - static_cast<std::ptrdiff_t>(n),
                            values.end());
      states.resize(states.size() - n);
      values.resize(values.size() - n);

      Attr lhs;
      translator.reduce(static_cast<Rule>(production), rhs, lhs);

      const int next = table.gotoAfterReduce(states.back(), production);
      if (next < 0) throw SyntaxError{};
      states.push_back(next);
      values.push_back(std::move(lhs));
      break;
    }

    case LR1Table::ActionKind::Accept:
      return translator.render();

    case LR1Table::ActionKind::Error:
      throw SyntaxError{};
    }
  }
}

} // namespace zcc
