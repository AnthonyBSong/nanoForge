#include <deque>
#include <fstream>
#include <regex>
#include <string>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <cctype>

#include "../include/Lexer.hpp"

Lexer::Lexer(std::ifstream& source,
             std::deque<Token>& parsedFileRef,
             const std::unordered_set<std::string>& instructionsSet,
             const std::unordered_set<std::string>& punctuationSet)
    : currentLine(1),
      currentColumn(1),
      parsedFile(parsedFileRef),
      instructions(instructionsSet),
      punctuations(punctuationSet)
{
    if (!source.is_open()) {
        throw std::runtime_error("Source file not found!");
    }

    std::string line;
    // Regex to capture tokens, including possible trailing colons (labels),
    // plus parentheses. Adjust as needed for your use case.
    std::regex re("([a-zA-Z0-9_]+:|[a-zA-Z0-9_]+|\\(|\\))", std::regex_constants::optimize);

    while (std::getline(source, line)) {
        auto lineStart = line.data(); // Pointer to the start of the line
        std::sregex_iterator it(line.begin(), line.end(), re);
        std::sregex_iterator end;

        for (; it != end; ++it) {
            const std::smatch& match = *it;

            // Position & length of the token in this line
            int tokenPosition = match.position(0);
            int tokenLength   = match.length(0);

            // Update currentColumn; columns start at 1
            currentColumn = tokenPosition + 1;

            // Get a pointer to the token substring within the line
            const char* tokenStr = lineStart + tokenPosition;

            // Tokenize and push the token
            Token tok = tokenize(tokenStr, tokenLength, currentLine, currentColumn);
            parsedFile.push_back(std::move(tok));
        }

        // After each line, we add an EoL token
        parsedFile.emplace_back(TokenType::EoL, "\n", currentLine, currentColumn);

        // Move to the next line
        currentLine++;
        currentColumn = 1;
    }

    // Finally, add an EoF token
    parsedFile.emplace_back(TokenType::EoF, "EOF", currentLine, currentColumn);
}

Token Lexer::tokenize(const char* str, size_t length, int line, int column) {
    TokenType type = TokenType::ERROR; // Default

    // We could use std::string_view in C++17+ for no-copy
    std::string tokenLexeme(str, length);

    // 1) Check punctuation first
    if (punctuations.find(tokenLexeme) != punctuations.end()) {
        // "(" or ")" or ":" etc.
        type = TokenType::PUNCTUATION;
    }
    // 2) Check if the token is an instruction
    else if (instructions.find(tokenLexeme) != instructions.end()) {
        type = TokenType::INSTRUCTION;
    }
    // 3) Check register (x0..x31)
    else if (length >= 2 && str[0] == 'x') {
        bool valid = true;
        int regNum = 0;
        for (size_t i = 1; i < length; ++i) {
            if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
                valid = false;
                break;
            }
            regNum = regNum * 10 + (str[i] - '0');
            if (regNum > 31) {
                valid = false;
                break;
            }
        }
        if (valid) {
            type = TokenType::REGISTER;
        }
    }
    // 4) Check immediate (binary, hex, decimal)
    else if (length > 0) {
        // Shortcut references
        const char* s = str;

        // a) binary immediate: 0bxxxx
        if (length > 2 && s[0] == '0' && s[1] == 'b') {
            bool valid = true;
            for (size_t i = 2; i < length; ++i) {
                if (s[i] != '0' && s[i] != '1') {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                type = TokenType::IMMEDIATE;
            }
        }
        // b) hex immediate: 0x....
        else if (length > 2 && s[0] == '0' && s[1] == 'x') {
            bool valid = true;
            for (size_t i = 2; i < length; ++i) {
                if (!std::isxdigit(static_cast<unsigned char>(s[i]))) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                type = TokenType::IMMEDIATE;
            }
        }
        // c) decimal immediate: all digits
        else {
            bool allDigits = true;
            for (size_t i = 0; i < length; ++i) {
                if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
                    allDigits = false;
                    break;
                }
            }
            if (allDigits) {
                type = TokenType::IMMEDIATE;
            }
        }
    }

    // 5) Check label: must end in ':' and not contain underscores (per your note)
    //    We'll do it *after* the immediate checks, so it doesn't conflict with numeric tokens that happen to have a trailing colon (unusual, but just in case).
    if (type == TokenType::ERROR && length > 0 && str[length - 1] == ':') {
        bool hasUnderscore = false;
        for (size_t i = 0; i < length - 1; ++i) {
            if (str[i] == '_') {
                hasUnderscore = true;
                break;
            }
        }
        if (!hasUnderscore) {
            type = TokenType::LABEL;
        }
    }

    return Token(type, std::move(tokenLexeme), line, column);
}

bool Lexer::hasMoreTokens() const {
    return !parsedFile.empty();
}

const Token& Lexer::peekNextToken() const {
    if (parsedFile.empty()) {
        throw std::out_of_range("No tokens available to peek.");
    }
    return parsedFile.front();
}

Token Lexer::getNextToken() {
    if (parsedFile.empty()) {
        throw std::out_of_range("No tokens available.");
    }
    Token nextToken = std::move(parsedFile.front());
    parsedFile.pop_front();
    return nextToken;
}

void Lexer::printTokens() const {
    for (const auto& tok : parsedFile) {
        std::string typeStr;
        switch (tok.type) {
            case TokenType::INSTRUCTION:   typeStr = "INSTRUCTION";   break;
            case TokenType::REGISTER:      typeStr = "REGISTER";      break;
            case TokenType::IMMEDIATE:     typeStr = "IMMEDIATE";     break;
            case TokenType::LABEL:         typeStr = "LABEL";         break;
            case TokenType::PUNCTUATION:   typeStr = "PUNCTUATION";   break;
            case TokenType::EoL:           typeStr = "EoL";           break;
            case TokenType::EoF:           typeStr = "EoF";           break;
            case TokenType::ERROR:         typeStr = "ERROR";         break;
        }
        std::cout << "Token: \"" << tok.lexeme << "\", Type: " << typeStr
                  << ", Line: " << tok.line << ", Column: " << tok.column << "\n";
    }
}
