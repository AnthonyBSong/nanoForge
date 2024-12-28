#pragma once

#include <deque>
#include <fstream>
#include <string>
#include <unordered_set>
#include "Token.hpp"

class Lexer {
public:
    // Constructor
    Lexer(std::ifstream& source,
          std::deque<Token>& parsedFileRef,
          const std::unordered_set<std::string>& instructionsSet,
          const std::unordered_set<std::string>& punctuationSet);

    // Token consumption functions
    bool hasMoreTokens() const;
    const Token& peekNextToken() const;
    Token getNextToken();

    // Function to print all tokens (optional, for debugging)
    void printTokens() const;

private:
    int currentLine;
    int currentColumn;
    std::deque<Token>& parsedFile;                           
    const std::unordered_set<std::string>& instructions;     
    const std::unordered_set<std::string>& punctuations; 

    // Tokenization function
    Token tokenize(const char* str, size_t length, int line, int column);
};
