#pragma once
#include <string>

enum class TokenType {
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    LABEL,
    PUNCTUATION,
    EoL,
    EoF,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenType t, std::string l, int ln, int col)
        : type(t), lexeme(std::move(l)), line(ln), column(col) {}
};
