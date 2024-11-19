#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <fstream>
#include <stdexcept>
#include <regex>
#include <unordered_set>
#include <iostream>
#include <queue>
#include "../include/Token.h"


class Lexer {
public:
    Lexer(std::ifstream& source);

    Token nextToken();

private:
    std::deque<Token> parsedFile;
    int currentLine;
    int currentColumn;

    // Set of valid instruction mnemonics
    const std::unordered_set<std::string> instructions = {
        "lui", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu",
        "lw", "sw", "addi", "ori", "andi", "add", "sub", "sll", "srl",
        "sra", "or", "and"
    };
};

#endif