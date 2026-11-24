#include "zcc/Parse/Translator.h"

#include <string>
#include <utility>

namespace zcc {
namespace {

std::string suffix(ValueType type) {
  return type == ValueType::Int ? "i" : "d";
}

int widthOf(ValueType type) { return type == ValueType::Int ? 4 : 8; }

// Lab output: floating constants printed like std::to_string(double).
std::string floatText(const std::string &lexeme) {
  return std::to_string(std::stod(lexeme));
}

std::vector<int> makeList(int quad) { return {quad}; }

std::vector<int> merge(std::vector<int> a, const std::vector<int> &b) {
  a.insert(a.end(), b.begin(), b.end());
  return a;
}

// Copies the operand part (place and type) of a unary-chain nonterminal.
void copyOperand(Attr &to, const Attr &from) {
  to.place = from.place;
  to.type = from.type;
}

} // namespace

int Translator::find(const std::string &name) const {
  for (std::size_t i = 0; i < symtab_.size(); ++i)
    if (symtab_[i].name == name) return static_cast<int>(i);
  return -1;
}

void Translator::enter(const std::string &name, ValueType type, int width) {
  if (find(name) >= 0) throw SyntaxError{}; // redefinition
  symtab_.push_back(Symbol{name, type, offset_, false});
  offset_ += width;
}

std::string Translator::lvalue(const std::string &name) const {
  int i = find(name);
  if (i < 0) throw SyntaxError{}; // undefined variable
  return "TB" + std::to_string(i);
}

std::string Translator::rvalue(const std::string &name, ValueType &type) const {
  int i = find(name);
  if (i < 0 || !symtab_[i].initialized) throw SyntaxError{};
  type = symtab_[i].type;
  return "TB" + std::to_string(i);
}

void Translator::markInitialized(const std::string &name) {
  symtab_[find(name)].initialized = true;
}

std::string Translator::newTemp(ValueType type) {
  return "T" + std::to_string(tempCount_++) + "_" + suffix(type);
}

int Translator::nxq() const { return static_cast<int>(quads_.size()); }

int Translator::gen(const std::string &op, const std::string &arg1,
                    const std::string &arg2, const std::string &dest) {
  quads_.push_back(Quad{op, arg1, arg2, dest});
  return nxq() - 1;
}

void Translator::backpatch(const std::vector<int> &list, int target) {
  for (int q : list) quads_[q].dest = std::to_string(target);
}

void Translator::reduce(Rule rule, std::vector<Attr> &g, Attr &x) {
  switch (rule) {
  case Rule::Start:
    throw SyntaxError{}; // never reduced: the parser accepts instead

  case Rule::Prog:
  case Rule::VariablesMany:
  case Rule::VariablesOne:
  case Rule::ScanfEnd:
  case Rule::PrintfEnd:
    break;

  case Rule::Subprog:
    backpatch(g[2].nextlist, nxq());
    gen("End", "-", "-", "-");
    break;

  case Rule::MEmpty:
    offset_ = 0;
    break;

  case Rule::NEmpty:
    x.quad = nxq();
    break;

  case Rule::TInt:
    x.type = ValueType::Int;
    x.width = widthOf(ValueType::Int);
    break;

  case Rule::TDouble:
    x.type = ValueType::Double;
    x.width = widthOf(ValueType::Double);
    break;

  case Rule::IdFromId:
    x.name = g[0].lexeme;
    break;

  case Rule::VarDecl:
    enter(g[1].name, g[0].type, g[0].width);
    x.type = g[0].type;
    x.width = g[0].width;
    break;

  case Rule::VarListDecl:
    enter(g[2].name, g[0].type, g[0].width);
    x.type = g[0].type;
    x.width = g[0].width;
    break;

  case Rule::StmtAssign:
  case Rule::StmtScanf:
  case Rule::StmtPrintf:
  case Rule::StmtEmpty:
    break; // nextlist stays empty

  case Rule::StmtBlock:
    x.nextlist = g[1].nextlist;
    break;

  case Rule::StmtWhile: // while N1 B do N2 STATEMENT1
    backpatch(g[5].nextlist, g[1].quad);
    backpatch(g[2].truelist, g[4].quad);
    x.nextlist = g[2].falselist;
    gen("j", "-", "-", std::to_string(g[1].quad));
    break;

  case Rule::StmtIf: // if B then N STATEMENT1
    backpatch(g[1].truelist, g[3].quad);
    x.nextlist = merge(g[1].falselist, g[4].nextlist);
    break;

  case Rule::AssignStmt: { // ID = EXPR
    std::string p = lvalue(g[0].name);
    gen("=", g[2].place, "-", p);
    markInitialized(g[0].name);
    break;
  }

  case Rule::ListMany: // L1 ; N STATEMENT
    backpatch(g[0].nextlist, g[2].quad);
    x.nextlist = g[3].nextlist;
    break;

  case Rule::ListOne:
    x.nextlist = g[0].nextlist;
    break;

  case Rule::ExprOr:
    x.type = ValueType::Int;
    x.place = newTemp(ValueType::Int);
    gen("||", g[0].place, g[2].place, x.place);
    break;

  case Rule::ExprOne:
  case Rule::OrItemOne:
  case Rule::AndItemOne:
  case Rule::NoItemOne:
  case Rule::RelItemOne:
  case Rule::ItemOne:
    copyOperand(x, g[0]);
    break;

  case Rule::FactorParen: // ( EXPR )
    copyOperand(x, g[1]);
    break;

  case Rule::OrItemAnd:
    x.type = ValueType::Int;
    x.place = newTemp(ValueType::Int);
    gen("&&", g[0].place, g[2].place, x.place);
    break;

  case Rule::AndItemNot:
    x.type = ValueType::Int;
    x.place = newTemp(ValueType::Int);
    gen("!", g[1].place, "-", x.place);
    break;

  case Rule::NoItemRel:
    x.type = ValueType::Int;
    x.place = newTemp(ValueType::Int);
    gen(g[1].op, g[0].place, g[2].place, x.place);
    break;

  case Rule::RelItemAdd:
    x.type = g[0].type;
    x.place = newTemp(x.type);
    gen(g[1].op, g[0].place, g[2].place, x.place);
    break;

  case Rule::ItemMul:
    x.type = g[2].type;
    x.place = newTemp(x.type);
    gen(g[1].op, g[0].place, g[2].place, x.place);
    break;

  case Rule::FactorId:
    x.place = rvalue(g[0].name, x.type);
    break;

  case Rule::FactorUint:
    x.type = ValueType::Int;
    x.place = newTemp(x.type);
    gen("=", g[0].lexeme, "-", x.place);
    break;

  case Rule::FactorUfloat:
    x.type = ValueType::Double;
    x.place = newTemp(x.type);
    gen("=", floatText(g[0].lexeme), "-", x.place);
    break;

  case Rule::FactorUnary: // PLUS_MINUS FACTOR
    x.type = g[1].type;
    x.place = newTemp(x.type);
    gen(g[0].op, "0", g[1].place, x.place);
    break;

  case Rule::BOr: // B1 || N BORTERM
    backpatch(g[0].falselist, g[2].quad);
    x.truelist = merge(g[0].truelist, g[3].truelist);
    x.falselist = g[3].falselist;
    break;

  case Rule::BOne:
  case Rule::BorOne:
    x.truelist = g[0].truelist;
    x.falselist = g[0].falselist;
    break;

  case Rule::BorAnd: // BORTERM1 && N BANDTERM
    backpatch(g[0].truelist, g[2].quad);
    x.falselist = merge(g[0].falselist, g[3].falselist);
    x.truelist = g[3].truelist;
    break;

  case Rule::BandParen:
    x.truelist = g[1].truelist;
    x.falselist = g[1].falselist;
    break;

  case Rule::BandNot:
    x.truelist = g[1].falselist;
    x.falselist = g[1].truelist;
    break;

  case Rule::BandRel: { // BFACTOR1 REL BFACTOR2
    const int q = nxq();
    x.truelist = makeList(q);
    x.falselist = makeList(q + 1);
    gen("j" + g[1].op, g[0].place, g[2].place, "0");
    gen("j", "-", "-", "0");
    break;
  }

  case Rule::BandOne: { // BFACTOR
    const int q = nxq();
    x.truelist = makeList(q);
    x.falselist = makeList(q + 1);
    gen("jnz", g[0].place, "-", "0");
    gen("j", "-", "-", "0");
    break;
  }

  case Rule::BfUint:
    x.type = ValueType::Int;
    x.place = newTemp(x.type);
    gen("=", g[0].lexeme, "-", x.place);
    break;

  case Rule::BfUfloat:
    x.type = ValueType::Double;
    x.place = newTemp(x.type);
    gen("=", floatText(g[0].lexeme), "-", x.place);
    break;

  case Rule::BfId:
    x.place = rvalue(g[0].name, x.type);
    break;

  case Rule::PlusOp:
    x.op = "+";
    break;
  case Rule::MinusOp:
    x.op = "-";
    break;
  case Rule::MulOp:
    x.op = "*";
    break;
  case Rule::DivOp:
    x.op = "/";
    break;
  case Rule::EqOp:
    x.op = "==";
    break;
  case Rule::NeOp:
    x.op = "!=";
    break;
  case Rule::LtOp:
    x.op = "<";
    break;
  case Rule::LeOp:
    x.op = "<=";
    break;
  case Rule::GtOp:
    x.op = ">";
    break;
  case Rule::GeOp:
    x.op = ">=";
    break;

  case Rule::ScanfMore: // SCANF_BEGIN , ID
  case Rule::ScanfFirst: // scanf ( ID
    gen("R", "-", "-", lvalue(g[2].name));
    markInitialized(g[2].name);
    break;

  case Rule::PrintfFirst: // printf ( ID
  case Rule::PrintfMore:  // PRINTF_BEGIN , ID
    gen("W", "-", "-", lvalue(g[2].name));
    break;
  }
}

std::string Translator::render() const {
  std::string out;
  out += std::to_string(symtab_.size()) + "\n";
  for (const Symbol &s : symtab_) {
    out += s.name + " " + (s.type == ValueType::Int ? "0" : "1") + " null " +
           std::to_string(s.offset) + "\n";
  }
  out += std::to_string(tempCount_) + "\n";
  out += std::to_string(quads_.size()) + "\n";
  for (std::size_t i = 0; i < quads_.size(); ++i) {
    const Quad &q = quads_[i];
    out += std::to_string(i) + ": (" + q.op + "," + q.arg1 + "," + q.arg2 + "," +
           q.dest + ")\n";
  }
  return out;
}

} // namespace zcc
