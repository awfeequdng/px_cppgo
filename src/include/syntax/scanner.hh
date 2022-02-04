#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fmt/format.h>

#include "syntax/tokens.hh"
#include "common/types.hh"
#include "syntax/token_string.hh"
#include "common/utf8/rune.hh"
#include "syntax/source.hh"

namespace syntax
{
template <typename T>
std::vector<T> slice(std::vector<T>& v, std::size_t low, std::size_t high = -1) {
    std::vector<T> vec;
    if (high == -1) {
        std::copy(v.begin() + low, v.end(), std::back_inserter(vec));
    } else {
        std::copy(v.begin() + low, v.begin() + high, std::back_inserter(vec));
    }
    return vec;
}

#define comments 1ul
#define directives (1ul << 1)

extern std::string g_token_name;
extern uint8_t g_token_index[];

using rune = common::utf8::rune_t;
using rune_t = common::utf8::rune_t;

std::string tokStrFast(token tok) { return g_token_name.substr(g_token_index[tok - 1], g_token_index[tok] - g_token_index[tok - 1]); }
uint hash(std::string s) { return (uint(s[0]) << 4 ^ uint(s[1]) + uint(s.size()) & uint(keywordMap.size() - 1)); }
std::unordered_map<uint, token> keywordMap;

void init() {
    {
        auto tok = Token_Break;
        for (; tok <= Token_Var; tok++) {
            auto h = hash(TokenString(tok));
            if (keywordMap[h] != 0) {
                panic("imperfect hash");
            }
            keywordMap[h] = tok;
        }
    }
}

rune lower(rune ch) { return ('a' - 'A') | ch; }
bool isLetter(rune ch) { return 'a' <= lower(ch) && lower(ch) <= 'z' || ch == '_'; }
bool isDecimal(rune ch) { return '0' <= ch && ch <= '9'; }
bool isHex(rune ch) { return '0' <= ch && ch <= '9' || 'a' <= lower(ch) && lower(ch) <= 'f'; }
std::string baseName(int64_t base) {
    switch (base) {
        case 2: {
            return "binary";
        } break;
        case 8: {
            return "octal";
        } break;
        case 10: {
            return "decimal";
        } break;
        case 16: {
            return "hexadecimal";
        } break;
    }
    panic("invalid base");
}

int64_t invalidSep(string x) {
    rune x1 = ' ';
    rune d = '.';
    auto i = 0;
    if (x.size() >= 2 && x[0] == '0') {
        x1 = lower(rune((unsigned char)x[1]));
        if (x1 == 'x' || x1 == 'o' || x1 == 'b') {
            d = '0';
            i = 2;
        }
    }
    for (; i < x.size(); i++) {
        auto p = d;
        d = rune(x[i]);
        if (d == '_') {
            if (p != '0') {
                return i;
            }
        } else if (isDecimal(d) || x1 == 'x' && isHex(d)) {
            d = '0';
        } else

        {
            if (p == '_') {
                return i - 1;
            }
            d = '.';
        }
    }
    if (d == '_') {
        return x.size() - 1;
    }
    return -1;
}

struct scanner : public source {
    uint _mode;
    bool _nlsemi;
    uint _line;
    uint _col;
    bool _blank;
    token _tok;
    string _lit;
    bool _bad;
    LitKind _kind;
    Operator _op;
    int64_t _prec;
    template<typename... T>
    static void errorf(T... args) {
        std::cout << fmt::format(args) << std::endl;
    }
    template<typename... T>
    static void errorAtf(int idx, T... args) {
        std::cout << "index:" << idx;
        std::cout << fmt::format(args) << std::endl;
    }
    void init(std::string src, uint mode);
    void setLit(LitKind kind, bool ok);
    void next();
    void ident();
    bool atIdentChar(bool first);
    int64_t digits(int64_t base, int* invalid);
    void number(bool seenPoint);
    void rune();
    void stdString();
    void rawString();
    void comment(string text);
    void skipLine();
    void lineComment();
    bool skipComment();
    void fullComment();
    bool escape(rune_t quote);
};
void scanner::init(std::string src, uint mode) {
    source::init(src, nullptr);
    (*this)._mode = mode;
    (*this)._nlsemi = false;
}
void scanner::setLit(LitKind kind, bool ok) {
    (*this)._nlsemi = true;
    (*this)._tok = Token_Literal;
    (*this)._lit = string((*this).segment());
    (*this)._bad = !ok;
    (*this)._kind = kind;
}
void scanner::next() {
    auto nlsemi = (*this)._nlsemi;
    (*this)._nlsemi = false;
redo:
    (*this).stop();
    auto [startLine, startCol] = (*this).pos();
    for (; (*this)._ch == ' ' || (*this)._ch == '\t' || (*this)._ch == '\n' && !nlsemi || (*this)._ch == '\r';) {
        (*this).nextch();
    }
    std::tie((*this)._line, (*this)._col) = (*this).pos();
    (*this)._blank = (*this)._line > startLine || startCol == colbase;
    (*this).start();
    if (isLetter((*this)._ch) || (*this)._ch >= common::utf8::rune_self && (*this).atIdentChar(true)) {
        (*this).nextch();
        (*this).ident();
        return;
    }
    switch ((int)(*this)._ch) {
        case -1: {
            if (nlsemi) {
                (*this)._lit = "EOF";
                (*this)._tok = Token_Semi;
                break;
            }
            (*this)._tok = Token_EOF;
        } break;
        case '\n': {
            (*this).nextch();
            (*this)._lit = "newline";
            (*this)._tok = Token_Semi;
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            (*this).number(false);
        } break;
        case '"': {
            (*this).stdString();
        } break;
        case '`': {
            (*this).rawString();
        } break;
        case '\'': {
            (*this).rune();
        } break;
        case '(': {
            (*this).nextch();
            (*this)._tok = Token_Lparen;
        } break;
        case '[': {
            (*this).nextch();
            (*this)._tok = Token_Lbrack;
        } break;
        case '{': {
            (*this).nextch();
            (*this)._tok = Token_Lbrace;
        } break;
        case ',': {
            (*this).nextch();
            (*this)._tok = Token_Comma;
        } break;
        case ';': {
            (*this).nextch();
            (*this)._lit = "semicolon";
            (*this)._tok = Token_Semi;
        } break;
        case ')': {
            (*this).nextch();
            (*this)._nlsemi = true;
            (*this)._tok = Token_Rparen;
        } break;
        case ']': {
            (*this).nextch();
            (*this)._nlsemi = true;
            (*this)._tok = Token_Rbrack;
        } break;
        case '}': {
            (*this).nextch();
            (*this)._nlsemi = true;
            (*this)._tok = Token_Rbrace;
        } break;
        case ':': {
            (*this).nextch();
            if ((*this)._ch == '=') {
                (*this).nextch();
                (*this)._tok = Token_Define;
                break;
            }
            (*this)._tok = Token_Colon;
        } break;
        case '.': {
            (*this).nextch();
            if (isDecimal((*this)._ch)) {
                (*this).number(true);
                break;
            }
            if ((*this)._ch == '.') {
                (*this).nextch();
                if ((*this)._ch == '.') {
                    (*this).nextch();
                    (*this)._tok = Token_DotDotDot;
                    break;
                }
                (*this).rewind();
                (*this).nextch();
            }
            (*this)._tok = Token_Dot;
        } break;
        case '+': {
            (*this).nextch();
            (*this)._op = Operator_Add;
            (*this)._prec = precAdd;
            if ((*this)._ch != '+') {
                goto assignop;
            }
            (*this).nextch();
            (*this)._nlsemi = true;
            (*this)._tok = Token_IncOp;
        } break;
        case '-': {
            (*this).nextch();
            (*this)._op = Operator_Sub;
            (*this)._prec = precAdd;
            if ((*this)._ch != '-') {
                goto assignop;
            }
            (*this).nextch();
            (*this)._nlsemi = true;
            (*this)._tok = Token_IncOp;
        } break;
        case '*': {
            (*this).nextch();
            (*this)._op = Operator_Mul;
            (*this)._prec = precMul;
            if ((*this)._ch == '=') {
                (*this).nextch();
                (*this)._tok = Token_AssignOp;
                break;
            }
            (*this)._tok = Token_Star;
        } break;
        case '/': {
            (*this).nextch();
            if ((*this)._ch == '/') {
                (*this).nextch();
                (*this).lineComment();
                goto redo;
            }
            if ((*this)._ch == '*') {
                (*this).nextch();
                (*this).fullComment();
                {
                    auto [line, _] = (*this).pos();
                    if (line > (*this)._line && nlsemi) {
                        (*this)._lit = "newline";
                        (*this)._tok = Token_Semi;
                        break;
                    }
                }
                goto redo;
            }
            (*this)._op = Operator_Div;
            (*this)._prec = precMul;
            goto assignop;
        } break;
        case '%': {
            (*this).nextch();
            (*this)._op = Operator_Rem;
            (*this)._prec = precMul;
            goto assignop;
        } break;
        case '&': {
            (*this).nextch();
            if ((*this)._ch == '&') {
                (*this).nextch();
                (*this)._op = Operator_AndAnd;
                (*this)._prec = precAndAnd;
                (*this)._tok = Token_Operator;
                break;
            }
            (*this)._op = Operator_And;
            (*this)._prec = precMul;
            if ((*this)._ch == '^') {
                (*this).nextch();
                (*this)._op = Operator_AndNot;
            }
            goto assignop;
        } break;
        case '|': {
            (*this).nextch();
            if ((*this)._ch == '|') {
                (*this).nextch();
                (*this)._op = Operator_OrOr;
                (*this)._prec = precOrOr;
                (*this)._tok = Token_Operator;
                break;
            }
            (*this)._op = Operator_Or;
            (*this)._prec = precAdd;
            goto assignop;
        } break;
        case '^': {
            (*this).nextch();
            (*this)._op = Operator_Xor;
            (*this)._prec = precAdd;
            goto assignop;
        } break;
        case '<': {
            (*this).nextch();
            if ((*this)._ch == '=') {
                (*this).nextch();
                (*this)._op = Operator_Leq;
                (*this)._prec = precCmp;
                (*this)._tok = Token_Operator;
                break;
            }
            if ((*this)._ch == '<') {
                (*this).nextch();
                (*this)._op = Operator_Shl;
                (*this)._prec = precMul;
                goto assignop;
            }
            if ((*this)._ch == '-') {
                (*this).nextch();
                (*this)._tok = Token_Arrow;
                break;
            }
            (*this)._op = Operator_Lss;
            (*this)._prec = precCmp;
            (*this)._tok = Token_Operator;
        } break;
        case '>': {
            (*this).nextch();
            if ((*this)._ch == '=') {
                (*this).nextch();
                (*this)._op = Operator_Geq;
                (*this)._prec = precCmp;
                (*this)._tok = Token_Operator;
                break;
            }
            if ((*this)._ch == '>') {
                (*this).nextch();
                (*this)._op = Operator_Shr;
                (*this)._prec = precMul;
                goto assignop;
            }
            (*this)._op = Operator_Gtr;
            (*this)._prec = precCmp;
            (*this)._tok = Token_Operator;
        } break;
        case '=': {
            (*this).nextch();
            if ((*this)._ch == '=') {
                (*this).nextch();
                (*this)._op = Operator_Eql;
                (*this)._prec = precCmp;
                (*this)._tok = Token_Operator;
                break;
            }
            (*this)._tok = Token_Assign;
        } break;
        case '!': {
            (*this).nextch();
            if ((*this)._ch == '=') {
                (*this).nextch();
                (*this)._op = Operator_Neq;
                (*this)._prec = precCmp;
                (*this)._tok = Token_Operator;
                break;
            }
            (*this)._op = Operator_Not;
            (*this)._prec = 0;
            (*this)._tok = Token_Operator;
        } break;
        case '~': {
            (*this).nextch();
            (*this)._op = Operator_Tilde;
            (*this)._prec = 0;
            (*this)._tok = Token_Operator;
        } break;
        default: {
            (*this).errorf("invalid character %#U", (*this)._ch);
            (*this).nextch();
            goto redo;
        } break;
    }
    return;
assignop:
    if ((*this)._ch == '=') {
        (*this).nextch();
        (*this)._tok = Token_AssignOp;
        return;
    }
    (*this)._tok = Token_Operator;
}
void scanner::ident() {
    for (; isLetter((*this)._ch) || isDecimal((*this)._ch);) {
        (*this).nextch();
    }
    if ((*this)._ch >= common::utf8::rune_self) {
        for (; (*this).atIdentChar(false);) {
            (*this).nextch();
        }
    }
    auto lit = (*this).segment();
    if (lit.size() >= 2) {
        {
            auto tok = keywordMap[hash(lit)];
            if (tok != 0 && tokStrFast(tok) == string(lit)) {
                (*this)._nlsemi = contains(1 << Token_Break | 1 << Token_Continue | 1 << Token_Fallthrough | 1 << Token_Return, tok);
                (*this)._tok = tok;
                return;
            }
        }
    }
    (*this)._nlsemi = true;
    (*this)._lit = string(lit);
    (*this)._tok = Token_Name;
}
bool scanner::atIdentChar(bool first) {
    if (isLetter((*this)._ch)) {

    } else if ((*this)._ch.is_digit()) {
        if (first) {
            (*this).errorf("identifier cannot begin with digit %#U", (*this)._ch);
        }
    } else if ((*this)._ch >= common::utf8::rune_self) {
        (*this).errorf("invalid character %#U in identifier", (*this)._ch);
    } else

    {
        return false;
    }
    return true;
}
int64_t scanner::digits(int64_t base, int* invalid) {
    int64_t digsep;
    if (base <= 10) {
        auto max = rune_t((unsigned char)('0' + base));
        for (; isDecimal((*this)._ch) || (*this)._ch == '_';) {
            auto ds = 1;
            if ((*this)._ch == '_') {
                ds = 2;
            } else {
                if ((*this)._ch >= max && invalid * < 0) {
                    auto [_, col] = (*this).pos();
                    invalid *= int(col - (*this)._col);
                }
            }
            digsep |= ds;
            (*this).nextch();
        }
    } else {
        for (; isHex((*this)._ch) || (*this)._ch == '_'; ) {
            auto ds = 1;
            if ((*this)._ch == '_') {
                ds = 2;
            }
            digsep |= ds;
            (*this).nextch();
        }
    }
    return digsep;
}
void scanner::number(bool seenPoint) {
    auto ok = true;
    auto kind = IntLit;
    auto base = 10;
    auto prefix = rune_t(0);
    auto digsep = 0;
    auto invalid = -1;
    if (!seenPoint) {
        if ((*this)._ch == '0') {
            (*this).nextch();
            switch (lower((*this)._ch)) {
                case 'x': {
                    (*this).nextch();
                    base = 16;
                    prefix = 'x';
                } break;
                case 'o': {
                    (*this).nextch();
                    base = 8;
                    prefix = 'o';
                } break;
                case 'b': {
                    (*this).nextch();
                    base = 2;
                    prefix = 'b';
                } break;
                default: {
                    base = 8;
                    prefix = '0';
                    digsep = 1;
                } break;
            }
        }
        digsep |= (*this).digits(base, &invalid);
        if ((*this)._ch == '.') {
            if (prefix == 'o' || prefix == 'b') {
                (*this).errorf("invalid radix point in %(*this) literal", baseName(base));
                ok = false;
            }
            (*this).nextch();
            seenPoint = true;
        }
    }
    if (seenPoint) {
        kind = FloatLit;
        digsep |= (*this).digits(base, &invalid);
    }
    if (digsep & 1 == 0 && ok) {
        (*this).errorf("%(*this) literal has no digits", baseName(base));
        ok = false;
    }
    {
        auto e = lower((*this)._ch);
        if (e == 'e' || e == 'p') {
            if (ok) {
                if (e == 'e' && prefix != 0 && prefix != '0') {
                    (*this).errorf("%q exponent requires decimal mantissa", (*this)._ch);
                    ok = false;
                } else if (e == 'p' && prefix != 'x') {
                    (*this).errorf("%q exponent requires hexadecimal mantissa", (*this)._ch);
                    ok = false;
                }
            }
            (*this).nextch();
            kind = FloatLit;
            if ((*this)._ch == '+' || (*this)._ch == '-') {
                (*this).nextch();
            }
            digsep = (*this).digits(10, nil) | digsep & 2;
            if (digsep & 1 == 0 && ok) {
                (*this).errorf("exponent has no digits");
                ok = false;
            }
        } else {
            if (prefix == 'x' && kind == FloatLit && ok) {
                (*this).errorf("hexadecimal mantissa requires a 'p' exponent");
                ok = false;
            }
        }
    }
    if ((*this)._ch == 'i') {
        kind = ImagLit;
        (*this).nextch();
    }
    (*this).setLit(kind, ok);
    if (kind == IntLit && invalid >= 0 && ok) {
        (*this).errorAtf(invalid, "invalid digit %q in %(*this) literal", (*this)._lit[invalid], baseName(base));
        ok = false;
    }
    if (digsep & 2 != 0 && ok) {
        {
            auto i = invalidSep((*this)._lit);
            if (i >= 0) {
                (*this).errorAtf(i, "'_' must separate successive digits");
                ok = false;
            }
        }
    }
    (*this)._bad = !ok;
}
void scanner::rune() {
    auto ok = true;
    (*this).nextch();
    auto n = 0;
    for (;; n++) {
        if ((*this)._ch == '\'') {
            if (ok) {
                if (n == 0) {
                    (*this).errorf("empty rune literal or unescaped '");
                    ok = false;
                } else {
                    if (n != 1) {
                        (*this).errorAtf(0, "more than one character in rune literal");
                        ok = false;
                    }
                }
            }
            (*this).nextch();
            break;
        }
        if ((*this)._ch == '\\') {
            (*this).nextch();
            if (!(*this).escape('\'')) {
                ok = false;
            }
            continue;
        }
        if ((*this)._ch == '\n') {
            if (ok) {
                (*this).errorf("newline in rune literal");
                ok = false;
            }
            break;
        }
        if ((*this)._ch < 0) {
            if (ok) {
                (*this).errorAtf(0, "rune literal not terminated");
                ok = false;
            }
            break;
        }
        (*this).nextch();
    }
    (*this).setLit(RuneLit, ok);
}
void scanner::stdString() {
    auto ok = true;
    (*this).nextch();
    for (;;) {
        if ((*this)._ch == '"') {
            (*this).nextch();
            break;
        }
        if ((*this)._ch == '\\') {
            (*this).nextch();
            if (!(*this).escape('"')) {
                ok = false;
            }
            continue;
        }
        if ((*this)._ch == '\n') {
            (*this).errorf("newline in string");
            ok = false;
            break;
        }
        if ((*this)._ch < 0) {
            (*this).errorAtf(0, "string not terminated");
            ok = false;
            break;
        }
        (*this).nextch();
    }
    (*this).setLit(StringLit, ok);
}
void scanner::rawString() {
    auto ok = true;
    (*this).nextch();
    for (;;) {
        if ((*this)._ch == '`') {
            (*this).nextch();
            break;
        }
        if ((*this)._ch < 0) {
            (*this).errorAtf(0, "string not terminated");
            ok = false;
            break;
        }
        (*this).nextch();
    }
    (*this).setLit(StringLit, ok);
}
void scanner::comment(string text) { (*this).errorAtf(0, "%(*this)", text); }
void scanner::skipLine() {
    for (; (*this)._ch >= 0 && (*this)._ch != '\n';) {
        (*this).nextch();
    }
}
void scanner::lineComment() {
    if ((*this)._mode & comments != 0) {
        (*this).skipLine();
        (*this).comment(string((*this).segment()));
        return;
    }
    if ((*this)._mode & directives == 0 || ((*this)._ch != 'g' && (*this)._ch != 'l')) {
        (*this).stop();
        (*this).skipLine();
        return;
    }
    (*this).skipLine();
    (*this).comment(string((*this).segment()));
}
bool scanner::skipComment() {
    for (; (*this)._ch >= 0;) {
        for (; (*this)._ch == '*';) {
            (*this).nextch();
            if ((*this)._ch == '/') {
                (*this).nextch();
                return true;
            }
        }
        (*this).nextch();
    }
    (*this).errorAtf(0, "comment not terminated");
    return false;
}
void scanner::fullComment() {
    if ((*this)._mode & comments != 0) {
        if ((*this).skipComment()) {
            (*this).comment(string((*this).segment()));
        }
        return;
    }
    if ((*this)._mode & directives == 0 || (*this)._ch != 'l') {
        (*this).stop();
        (*this).skipComment();
        return;
    }
    if ((*this).skipComment()) {
        (*this).comment(string((*this).segment()));
    }
}
bool scanner::escape(rune_t quote) {
    int n;
    uint32_t base;
    uint32_t max;
    switch ((int)(*this)._ch) {
        case quote:
        case 'a':
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
        case 'v':
        case '\\': {
            (*this).nextch();
            return true;
        } break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            n = 3;
            base = 8;
            max = 255;
        } break;
        case 'x': {
            (*this).nextch();
            n = 2;
            base = 16;
            max = 255;
        } break;
        case 'u': {
            (*this).nextch();
            n = 4;
            base = 16;
            max = common::utf8::rune_max;
        } break;
        case 'U': {
            (*this).nextch();
            n = 8;
            base = 16;
            max = common::utf8::rune_max;
        } break;
        default: {
            if ((*this)._ch < 0) {
                return true;
            }
            (*this).errorf("unknown escape");
            return false;
        } break;
    }
    uint32_t x;
    {
        auto i = n;
        for (; i > 0; i--) {
            if ((*this)._ch < 0) {
                return true;
            }
            auto d = base;
            if (isDecimal((*this)._ch)) {
                d = uint32_t((*this)._ch) - '0';
            } else {
                if ('a' <= lower((*this)._ch) && lower((*this)._ch) <= 'f') {
                    d = uint32_t(lower((*this)._ch)) - 'a' + 10;
                }
            }
            if (d >= base) {
                (*this).errorf("invalid character %q in %(*this) escape", (*this)._ch, baseName(int(base)));
                return false;
            }
            x = x * base + d;
            (*this).nextch();
        }
    }
    if (x > max && base == 8) {
        (*this).errorf("octal escape value %d > 255", x);
        return false;
    }
    if (x > max || 0xD800 <= x && x < 0xE000) {
        (*this).errorf("escape is invalid Unicode code point %#U", x);
        return false;
    }
    return true;
}
} // namespace syntax
