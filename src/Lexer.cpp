#include "include/Lexer.h"

// 0 indexing for line/column of the source .s or .asm file
Lexer::Lexer(std::ifstream& source) : currentLine(0), currentColumn(0) {
    if (!source.is_open()) ;

    std::string line;
    while (std::getline(source, line)) {
        std::cout << line;
    }
};
