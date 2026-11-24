#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "zcc/Parse/Grammar.h"

namespace zcc {

// Any error the lab reports as "Syntax Error": undefined or uninitialized
// variables, redefinition, or an LR(1) failure.
struct SyntaxError {};

enum class ValueType : std::uint8_t { Int, Double };

// Attributes carried by each grammar symbol on the parse stack. Only the
// fields a symbol needs are meaningful.
struct Attr {
  std::string lexeme; // spelling of an id / UINT / UFLOAT token
  std::string name;   // ID.name
  std::string place;  // address of a variable, temporary, or constant
  std::string op;     // operator text for PLUS_MINUS / MUL_DIV / REL
  ValueType type = ValueType::Int;
  int width = 0;
  int quad = 0;
  std::vector<int> nextlist;
  std::vector<int> truelist;
  std::vector<int> falselist;
};

struct Symbol {
  std::string name;
  ValueType type;
  int offset;
  bool initialized; // assigned or read by scanf before this point
};

struct Quad {
  std::string op;
  std::string arg1;
  std::string arg2;
  std::string dest;
};

// Syntax-directed translation: symbol table, temporaries and quadruples.
class Translator {
public:
  // Applies the semantic action of `rule`. rhs holds the attributes of the
  // right-hand side in order; lhs receives the attributes of the left side.
  void reduce(Rule rule, std::vector<Attr> &rhs, Attr &lhs);

  // The full report: symbol table, temporary count, then quadruples.
  std::string render() const;

private:
  int find(const std::string &name) const;
  void enter(const std::string &name, ValueType type, int width);
  std::string lvalue(const std::string &name) const;
  std::string rvalue(const std::string &name, ValueType &type) const;
  void markInitialized(const std::string &name);

  std::string newTemp(ValueType type);
  int nxq() const;
  int gen(const std::string &op, const std::string &arg1,
          const std::string &arg2, const std::string &dest);
  void backpatch(const std::vector<int> &list, int target);

  std::vector<Symbol> symtab_;
  std::vector<Quad> quads_;
  int tempCount_ = 0;
  int offset_ = 0;
};

} // namespace zcc
