#include "token_string.hh"
#include <iostream>

void test_token_string() {
    std::cout << syntax::TokenString(Token_EOF) << std::endl;
    std::cout << syntax::TokenString(Token_Name) << std::endl;
    std::cout << syntax::TokenString(Token_Literal) << std::endl;
}

int main() {
    test_token_string();
    return 0;
}