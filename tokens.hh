#pragma once
#include <stdlib.h>

using namespace std;

namespace syntax {
typedef uint64_t token;

// type token
// #define _ 0
#define _EOF 1
// names and literals
#define _Name 2
#define _Literal 3
// operators and operations
// _Operator is excluding '*' (_Star)
#define _Operator 4
#define _AssignOp 5
#define _IncOp 6
#define _Assign 7
#define _Define 8
#define _Arrow 9
#define _Star 10
// delimiters
#define _Lparen 11
#define _Lbrack 12
#define _Lbrace 13
#define _Rparen 14
#define _Rbrack 15
#define _Rbrace 16
#define _Comma 17
#define _Semi 18
#define _Colon 19
#define _Dot 20
#define _DotDotDot 21
// keywords
#define _Break 22
#define _Case 23
#define _Chan 24
#define _Const 25
#define _Continue 26
#define _Default 27
#define _Defer 28
#define _Else 29
#define _Fallthrough 30
#define _For 31
#define _Func 32
#define _Go 33
#define _Goto 34
#define _If 35
#define _Import 36
#define _Interface 37
#define _Map 38
#define _Package 39
#define _Range 40
#define _Return 41
#define _Select 42
#define _Struct 43
#define _Switch 44
#define _Type 45
#define _Var 46
#define tokenCount 47

// for BranchStmt
#define Break _Break
#define Continue _Continue
#define Fallthrough _Fallthrough
#define Goto _Goto

// for CallStmt
#define Go _Go
#define Defer _Defer

// Make sure we have at most 64 tokens so we can use them in a set.
#define _xx (1 << (tokenCount - 1))

// contains reports whether tok is in tokset.
inline bool contains(uint64_t tokset, token tok) { return tokset & (1 << tok) != 0; }

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