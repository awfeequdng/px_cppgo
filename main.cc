#include "token_string.hh"
#include "operator_string.hh"
#include <iostream>

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



int main() {
    test_token_string();
    test_operator_string();
    return 0;
}