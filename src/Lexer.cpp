#include <deque>
#include <fstream>
#include <regex>
#include <string>
#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <cctype>

#include "../include/Lexer.hpp"

// Implementation of the Lexer constructor
// Implementation of the Lexer constructor
Lexer::Lexer(std::ifstream& source, std::deque<Token>& parsedFileRef, const std::unordered_set<std::string>& instructionsSet, const std::unordered_set<std::string>& punctuationSet)
    : currentLine(1), currentColumn(1), parsedFile(parsedFileRef), instructions(instructionsSet), punctuations(punctuationSet)
{
    if (!source.is_open()) {
        throw std::runtime_error("Source file not found!");
    }

    std::string line;
    // Updated regex to also capture any trailing colon on labels:
    std::regex re("([a-zA-Z0-9_]+:|[a-zA-Z0-9_]+|\\(|\\))", std::regex_constants::optimize);

    while (std::getline(source, line)) {
        auto lineStart = line.data(); // Pointer to the start of the line
        std::sregex_iterator it(line.begin(), line.end(), re);
        std::sregex_iterator end;

        for (; it != end; ++it) {
            const std::smatch& match = *it;

            // Get the position and length of the token
            int tokenPosition = match.position(0);
            int tokenLength = match.length(0);

            // Update currentColumn to the position of the token
            currentColumn = tokenPosition + 1; // Columns start at 1

            // Get a pointer to the token within the line
            const char* tokenStr = lineStart + tokenPosition;

            // Tokenize and push the token into parsedFile
            Token tok = tokenize(tokenStr, tokenLength, currentLine, currentColumn);
            parsedFile.push_back(std::move(tok));
        }

        // After processing all tokens in the line, add an END_OF_LINE token
        parsedFile.emplace_back(TokenType::_EOL, "\n", currentLine, currentColumn);

        // Move to the next line
        currentLine++;
        currentColumn = 1; // Reset column at the start of a new line
    }

    // Add an End of File token at the end
    parsedFile.emplace_back(TokenType::_EOF, "EOF", currentLine, currentColumn);
}

// Implementation of the tokenize function
Token Lexer::tokenize(const char* str, size_t length, int line, int column) {
    TokenType type = TokenType::_ERROR; // Default type

    // Avoid unnecessary copying by using string_view if available (C++17)
#if __cplusplus >= 201703L
    std::string_view tokenStr(str, length);
#else
    const char* tokenStr = str;
    size_t tokenLength = length;
#endif

    std::string tokenLexeme(str, length);

    // Detect parenthesis // <-- NEW CODE
    if (punctuations.find(tokenLexeme) != punctuations.end()) {
        type = TokenType::_PUNC;
    }
    // Check if the token is an instruction
    else if (instructions.find(tokenLexeme) != instructions.end()) {
        type = TokenType::_INSTR;
    }
    // Check if the token is a register
    else if (length >= 2 && str[0] == 'x') {
        int reg_num = 0;
        bool valid = true;
        for (size_t i = 1; i < length; ++i) {
            char c = str[i];
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                valid = false;
                break;
            }
            reg_num = reg_num * 10 + (c - '0');
            if (reg_num > 31) { // Early exit if reg_num exceeds 31
                valid = false;
                break;
            }
        }
        if (valid) {
            type = TokenType::_REG;
        }
    }
    // Check if the token is an immediate value
    else if (length > 0) {
        const char* s = str;
        // Check for binary immediate (0b...)
        if (length > 2 && s[0] == '0' && s[1] == 'b') {
            bool valid = true;
            for (size_t i = 2; i < length; ++i) {
                char c = s[i];
                if (c != '0' && c != '1') {
                    valid = false;
                    break;
                }
            }
            if (valid) type = TokenType::_IMM;
        }
        // Check for hexadecimal immediate (0x...)
        else if (length > 2 && s[0] == '0' && s[1] == 'x') {
            bool valid = true;
            for (size_t i = 2; i < length; ++i) {
                char c = s[i];
                if (!std::isxdigit(static_cast<unsigned char>(c))) {
                    valid = false;
                    break;
                }
            }
            if (valid) type = TokenType::_IMM;
        }
        else if (type == TokenType::_ERROR && length > 0 && str[length - 1] == ':') {
            // Labels shouldn't contain underscores (as per your specification)
            bool hasUnderscore = false;
            for (size_t i = 0; i < length - 1; ++i) {
                if (str[i] == '_') {
                    hasUnderscore = true;
                    break;
                }
            }
            if (!hasUnderscore) {
                type = TokenType::_LABEL;
            }
        }
        // Check for decimal immediate
        else {
            bool valid = true;
            for (size_t i = 0; i < length; ++i) {
                char c = s[i];
                if (!std::isdigit(static_cast<unsigned char>(c))) {
                    valid = false;
                    break;
                }
            }
            if (valid) type = TokenType::_IMM;
        }
    }

    // Check if the token is a label (must appear after initial immediate check)
    if (type == TokenType::_ERROR && length > 0 && str[length - 1] == ':') {
        // Labels shouldn't contain underscores (as per your specification)
        bool hasUnderscore = false;
        for (size_t i = 0; i < length - 1; ++i) {
            if (str[i] == '_') {
                hasUnderscore = true;
                break;
            }
        }
        if (!hasUnderscore) {
            type = TokenType::_LABEL;
        }
    }

    return Token(type, std::move(tokenLexeme), line, column);
}

// Implementation of token consumption functions

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

// Implementation of printTokens. May need to be changed later
void Lexer::printTokens() const {
    for (const auto& tok : parsedFile) {
        std::string typeStr;
        switch (tok.type) {
            case TokenType::_INSTR: typeStr = "INSTRUCTION"; break;
            case TokenType::_REG:    typeStr = "REGISTER";    break;
            case TokenType::_IMM:   typeStr = "IMMEDIATE";   break;
            case TokenType::_LABEL:       typeStr = "LABEL";       break;
            case TokenType::_PUNC:      typeStr = "PUNCTUATION";      break;
            case TokenType::_EOL:         typeStr = "EoL";         break;
            case TokenType::_EOF:         typeStr = "EoF";         break;
            case TokenType::_ERROR:       typeStr = "ERROR";       break;
            default:                     typeStr = "UNKNOWN";     break;
        }
        std::cout << "Token: \"" << tok.lexeme << "\", Type: " << typeStr
                  << ", Line: " << tok.line << ", Column: " << tok.column << "\n";
    }
}
