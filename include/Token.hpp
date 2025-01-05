#pragma once
#include <string>

struct Punctuation {
    std::string type;
};

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

    bool operator==(const Token& other) const {
        return this->type == other.type && this->lexeme == other.lexeme;
    }

    bool compareTokenType(const Token& other) const {
        if (this->type == TokenType::PUNCTUATION 
            || other.type == TokenType::PUNCTUATION) {
                return (this->type == TokenType::PUNCTUATION && other.type == TokenType::PUNCTUATION)
                    && (this->lexeme == other.lexeme);
        }
        else return this->type == TokenType::PUNCTUATION && other.type == TokenType::PUNCTUATION;
    }
};
