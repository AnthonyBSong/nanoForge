#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>
#include <cctype>
#include <stdexcept>
#include <unordered_set>

enum class TokenType {
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    LABEL,
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    // Optional: error message if type == ERROR
};


class Lexer {
public:
    Lexer(std::istream& source)
        : source(source), currentLine(1), currentColumn(1) {}

    Token nextToken();

private:
    std::istream& source;
    int currentLine;
    int currentColumn;

    char peekChar();
    char getChar();
    void skipNonAlphanumeric();
    bool isDigitString(const std::string& s);
    bool isInstruction(const std::string& s);

    // Set of valid instruction mnemonics
    const std::unordered_set<std::string> instructions = {
        "lui", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu",
        "lw", "sw", "addi", "ori", "andi", "add", "sub", "sll", "srl",
        "sra", "or", "and"
    };
};

#endif