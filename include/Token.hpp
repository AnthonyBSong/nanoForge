#pragma once
#include <string>

enum class TokenType {
    _INSTR,
    _REG,
    _IMM,
    _LABEL,
    _PUNC,
    _EOL,
    _EOF,
    _ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenType t, const std::string& lex, int ln, int col)
        : type(t), lexeme(lex), line(ln), column(col) {}
};