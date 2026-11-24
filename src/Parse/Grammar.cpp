#include "zcc/Parse/Grammar.h"

namespace zcc {

// Terminals are the lab's token names (int, id, UINT, UFLOAT, operators, ...);
// nonterminals are the upper-case names. The order must match enum class Rule.
const std::vector<Production> &labGrammar() {
  static const std::vector<Production> grammar = {
      {"START", {"PROG"}},                                        // Start
      {"PROG", {"SUBPROG"}},                                      // Prog
      {"SUBPROG", {"M", "VARIABLES", "STATEMENT"}},               // Subprog
      {"M", {}},                                                  // MEmpty
      {"N", {}},                                                  // NEmpty
      {"VARIABLES", {"VARIABLES", "VARIABLE", ";"}},              // VariablesMany
      {"VARIABLES", {"VARIABLE", ";"}},                           // VariablesOne
      {"T", {"int"}},                                             // TInt
      {"T", {"double"}},                                          // TDouble
      {"ID", {"id"}},                                             // IdFromId
      {"VARIABLE", {"T", "ID"}},                                  // VarDecl
      {"VARIABLE", {"VARIABLE", ",", "ID"}},                      // VarListDecl
      {"STATEMENT", {"ASSIGN"}},                                  // StmtAssign
      {"STATEMENT", {"SCANF"}},                                   // StmtScanf
      {"STATEMENT", {"PRINTF"}},                                  // StmtPrintf
      {"STATEMENT", {}},                                          // StmtEmpty
      {"STATEMENT", {"{", "L", ";", "}"}},                        // StmtBlock
      {"STATEMENT", {"while", "N", "B", "do", "N", "STATEMENT"}}, // StmtWhile
      {"STATEMENT", {"if", "B", "then", "N", "STATEMENT"}},       // StmtIf
      {"ASSIGN", {"ID", "=", "EXPR"}},                            // AssignStmt
      {"L", {"L", ";", "N", "STATEMENT"}},                        // ListMany
      {"L", {"STATEMENT"}},                                       // ListOne
      {"EXPR", {"EXPR", "||", "ORITEM"}},                         // ExprOr
      {"EXPR", {"ORITEM"}},                                       // ExprOne
      {"ORITEM", {"ORITEM", "&&", "ANDITEM"}},                    // OrItemAnd
      {"ORITEM", {"ANDITEM"}},                                    // OrItemOne
      {"ANDITEM", {"NOITEM"}},                                    // AndItemOne
      {"ANDITEM", {"!", "NOITEM"}},                               // AndItemNot
      {"NOITEM", {"NOITEM", "REL", "RELITEM"}},                   // NoItemRel
      {"NOITEM", {"RELITEM"}},                                    // NoItemOne
      {"RELITEM", {"RELITEM", "PLUS_MINUS", "ITEM"}},             // RelItemAdd
      {"RELITEM", {"ITEM"}},                                      // RelItemOne
      {"ITEM", {"FACTOR"}},                                       // ItemOne
      {"ITEM", {"ITEM", "MUL_DIV", "FACTOR"}},                    // ItemMul
      {"FACTOR", {"ID"}},                                         // FactorId
      {"FACTOR", {"UINT"}},                                       // FactorUint
      {"FACTOR", {"UFLOAT"}},                                     // FactorUfloat
      {"FACTOR", {"(", "EXPR", ")"}},                             // FactorParen
      {"FACTOR", {"PLUS_MINUS", "FACTOR"}},                       // FactorUnary
      {"B", {"B", "||", "N", "BORTERM"}},                         // BOr
      {"B", {"BORTERM"}},                                         // BOne
      {"BORTERM", {"BORTERM", "&&", "N", "BANDTERM"}},            // BorAnd
      {"BORTERM", {"BANDTERM"}},                                  // BorOne
      {"BANDTERM", {"(", "B", ")"}},                              // BandParen
      {"BANDTERM", {"!", "BANDTERM"}},                            // BandNot
      {"BANDTERM", {"BFACTOR", "REL", "BFACTOR"}},                // BandRel
      {"BANDTERM", {"BFACTOR"}},                                  // BandOne
      {"BFACTOR", {"UINT"}},                                      // BfUint
      {"BFACTOR", {"UFLOAT"}},                                    // BfUfloat
      {"BFACTOR", {"ID"}},                                        // BfId
      {"PLUS_MINUS", {"+"}},                                      // PlusOp
      {"PLUS_MINUS", {"-"}},                                      // MinusOp
      {"MUL_DIV", {"*"}},                                         // MulOp
      {"MUL_DIV", {"/"}},                                         // DivOp
      {"REL", {"=="}},                                            // EqOp
      {"REL", {"!="}},                                            // NeOp
      {"REL", {"<"}},                                             // LtOp
      {"REL", {"<="}},                                            // LeOp
      {"REL", {">"}},                                             // GtOp
      {"REL", {">="}},                                            // GeOp
      {"SCANF", {"SCANF_BEGIN", ")"}},                            // ScanfEnd
      {"SCANF_BEGIN", {"SCANF_BEGIN", ",", "ID"}},                // ScanfMore
      {"SCANF_BEGIN", {"scanf", "(", "ID"}},                      // ScanfFirst
      {"PRINTF", {"PRINTF_BEGIN", ")"}},                          // PrintfEnd
      {"PRINTF_BEGIN", {"printf", "(", "ID"}},                    // PrintfFirst
      {"PRINTF_BEGIN", {"PRINTF_BEGIN", ",", "ID"}},              // PrintfMore
  };
  return grammar;
}

} // namespace zcc
