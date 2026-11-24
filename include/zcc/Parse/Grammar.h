#pragma once

#include <string>
#include <vector>

namespace zcc {

// A production LHS -> RHS. An empty RHS is the lab's epsilon (^).
struct Production {
  std::string lhs;
  std::vector<std::string> rhs;
};

// Identifies a production. Values are indices into labGrammar(), so the
// order of the enumerators must match the order of the productions there.
enum class Rule : int {
  Start,         // START -> PROG (augmented; reaching it means accept)
  Prog,          // PROG -> SUBPROG
  Subprog,       // SUBPROG -> M VARIABLES STATEMENT
  MEmpty,        // M -> ^
  NEmpty,        // N -> ^
  VariablesMany, // VARIABLES -> VARIABLES VARIABLE ;
  VariablesOne,  // VARIABLES -> VARIABLE ;
  TInt,          // T -> int
  TDouble,       // T -> double
  IdFromId,      // ID -> id
  VarDecl,       // VARIABLE -> T ID
  VarListDecl,   // VARIABLE -> VARIABLE , ID
  StmtAssign,    // STATEMENT -> ASSIGN
  StmtScanf,     // STATEMENT -> SCANF
  StmtPrintf,    // STATEMENT -> PRINTF
  StmtEmpty,     // STATEMENT -> ^
  StmtBlock,     // STATEMENT -> { L ; }
  StmtWhile,     // STATEMENT -> while N B do N STATEMENT
  StmtIf,        // STATEMENT -> if B then N STATEMENT
  AssignStmt,    // ASSIGN -> ID = EXPR
  ListMany,      // L -> L ; N STATEMENT
  ListOne,       // L -> STATEMENT
  ExprOr,        // EXPR -> EXPR || ORITEM
  ExprOne,       // EXPR -> ORITEM
  OrItemAnd,     // ORITEM -> ORITEM && ANDITEM
  OrItemOne,     // ORITEM -> ANDITEM
  AndItemOne,    // ANDITEM -> NOITEM
  AndItemNot,    // ANDITEM -> ! NOITEM
  NoItemRel,     // NOITEM -> NOITEM REL RELITEM
  NoItemOne,     // NOITEM -> RELITEM
  RelItemAdd,    // RELITEM -> RELITEM PLUS_MINUS ITEM
  RelItemOne,    // RELITEM -> ITEM
  ItemOne,       // ITEM -> FACTOR
  ItemMul,       // ITEM -> ITEM MUL_DIV FACTOR
  FactorId,      // FACTOR -> ID
  FactorUint,    // FACTOR -> UINT
  FactorUfloat,  // FACTOR -> UFLOAT
  FactorParen,   // FACTOR -> ( EXPR )
  FactorUnary,   // FACTOR -> PLUS_MINUS FACTOR
  BOr,           // B -> B || N BORTERM
  BOne,          // B -> BORTERM
  BorAnd,        // BORTERM -> BORTERM && N BANDTERM
  BorOne,        // BORTERM -> BANDTERM
  BandParen,     // BANDTERM -> ( B )
  BandNot,       // BANDTERM -> ! BANDTERM
  BandRel,       // BANDTERM -> BFACTOR REL BFACTOR
  BandOne,       // BANDTERM -> BFACTOR
  BfUint,        // BFACTOR -> UINT
  BfUfloat,      // BFACTOR -> UFLOAT
  BfId,          // BFACTOR -> ID
  PlusOp,        // PLUS_MINUS -> +
  MinusOp,       // PLUS_MINUS -> -
  MulOp,         // MUL_DIV -> *
  DivOp,         // MUL_DIV -> /
  EqOp,          // REL -> ==
  NeOp,          // REL -> !=
  LtOp,          // REL -> <
  LeOp,          // REL -> <=
  GtOp,          // REL -> >
  GeOp,          // REL -> >=
  ScanfEnd,      // SCANF -> SCANF_BEGIN )
  ScanfMore,     // SCANF_BEGIN -> SCANF_BEGIN , ID
  ScanfFirst,    // SCANF_BEGIN -> scanf ( ID
  PrintfEnd,     // PRINTF -> PRINTF_BEGIN )
  PrintfFirst,   // PRINTF_BEGIN -> printf ( ID
  PrintfMore,    // PRINTF_BEGIN -> PRINTF_BEGIN , ID
};

// The lab grammar, with production i being Rule(i).
const std::vector<Production> &labGrammar();

} // namespace zcc
