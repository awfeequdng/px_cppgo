#include <iostream>

#include "syntax/token_string.hh"
#include "syntax/operator_string.hh"
#include "syntax/source.hh"

void test_token_string() {
    std::cout << syntax::TokenString(Token_EOF) << std::endl;
    std::cout << syntax::TokenString(Token_Name) << std::endl;
    std::cout << syntax::TokenString(Token_Literal) << std::endl;
}

void test_operator_string() {
    std::cout << syntax::OperatorString(Operator_Def) << std::endl;
    std::cout << syntax::OperatorString(Operator_Tilde) << std::endl;
    std::cout << syntax::OperatorString(Operator_Shr) << std::endl;
}

void test_source() {
    syntax::source source;
    source.init("test.cc", nullptr);
    int cnt = 0;
    while (source._ch != -1) {
        cnt++;
        source.nextch();
        std::cout << "\t: " << source._ch << std::endl;
    }
}

int main() {
    test_token_string();
    test_operator_string();
    test_source();
    return 0;
}