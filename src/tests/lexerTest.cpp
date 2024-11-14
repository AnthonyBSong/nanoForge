#include <iostream>
#include <sstream>
#include <compiler/lexer/Lexer.h>

int main() {
    std::istringstream source("add x1 x2 x3\nlabel:\nlw x5 100 x6");
    Lexer lexer(source);
    Token token;
    while ((token = lexer.getNextToken()).type != TokenType::END_OF_FILE) {
        std::cout << "Token: " << token.lexeme
                  << ", Type: " << static_cast<int>(token.type)
                  << ", Line: " << token.line
                  << ", Column: " << token.column << std::endl;
    }
    return 0;
}