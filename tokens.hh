#pragma once
#include <stdlib.h>

using namespace std;

namespace syntax {
typedef uint64_t token;

// type token
// #define _ 0
#define Token_EOF 1
// names and literals
#define Token_Name 2
#define Token_Literal 3
// operators and operations
// Token_Operator is excluding '*' (Token_Star)
#define Token_Operator 4
#define Token_AssignOp 5
#define Token_IncOp 6
#define Token_Assign 7
#define Token_Define 8
#define Token_Arrow 9
#define Token_Star 10
// delimiters
#define Token_Lparen 11
#define Token_Lbrack 12
#define Token_Lbrace 13
#define Token_Rparen 14
#define Token_Rbrack 15
#define Token_Rbrace 16
#define Token_Comma 17
#define Token_Semi 18
#define Token_Colon 19
#define Token_Dot 20
#define Token_DotDotDot 21
// keywords
#define Token_Break 22
#define Token_Case 23
#define Token_Chan 24
#define Token_Const 25
#define Token_Continue 26
#define Token_Default 27
#define Token_Defer 28
#define Token_Else 29
#define Token_Fallthrough 30
#define Token_For 31
#define Token_Func 32
#define Token_Go 33
#define Token_Goto 34
#define Token_If 35
#define Token_Import 36
#define Token_Interface 37
#define Token_Map 38
#define Token_Package 39
#define Token_Range 40
#define Token_Return 41
#define Token_Select 42
#define Token_Struct 43
#define Token_Switch 44
#define Token_Type 45
#define Token_Var 46
#define tokenCount 47

// for BranchStmt
#define Break Token_Break
#define Continue Token_Continue
#define Fallthrough Token_Fallthrough
#define Goto Token_Goto

// for CallStmt
#define Go Token_Go
#define Defer Token_Defer

// Make sure we have at most 64 tokens so we can use them in a set.
#define _xx (1 << (tokenCount - 1))

// contains reports whether tok is in tokset.
// inline bool contains(uint64_t tokset, uint64_t tok) { return (tokset & (1 << tok)) != 0; }

typedef uint8_t LitKind;

#define IntLit 0
#define FloatLit 1
#define ImagLit 2
#define RuneLit 3
#define StringLit 4

typedef uint32_t Operator;

// #define _ 0
// Def is the : in :=
#define Def 1       // :
#define Not 2       // !
#define Recv 3      // <-
#define Tilde 4     // ~
// precOrOr
#define OrOr 5      // ||
// precAndAnd
#define AndAnd 6    // &&
// precCmp
#define Eql 7       // ==
#define Neq 8       // !=
#define Lss 9       // <
#define Leq 10      // <=
#define Gtr 11      // >
#define Geq 12      // >=
// precAdd
#define Add 13      // +
#define Sub 14      // -
#define Or 15       // |
#define Xor 16      // ^
// precMul
#define Mul 17      // *
#define Div 18      // /
#define Rem 19      // %
#define And 20      // &
#define AndNot 21   // &^
#define Shl 22      // <<
#define Shr 23      // >>

// Operator precedences
// #define _ 0
#define precOrOr 1
#define precAndAnd 2
#define precCmp 3
#define precAdd 4
#define precMul 5

} // namespace syntax