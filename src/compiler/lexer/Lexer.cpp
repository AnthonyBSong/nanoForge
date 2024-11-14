#include "Lexer.h"

Lexer::Lexer(std::istream& source)
    : source(source), currentLine(1), currentColumn(1) {};


char Lexer::peekChar() {
    return source.peek();
}

char Lexer::getChar() {
    char c = source.get();
    if (c == '\n') {
        currentLine++;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
    return c;
}

void Lexer::skipNonAlphanumeric() {
    while (source.good()) {
        char c = peekChar();
        if (!std::isalnum(c)) {
            getChar(); // Consume the character
        } else {
            break;
        }
    }
}

bool Lexer::isDigitString(const std::string& s) {
    for (char c : s) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return !s.empty();
}

bool Lexer::isInstruction(const std::string& s) {
    return instructions.find(s) != instructions.end();
}

Token Lexer::getNextToken() {
    skipNonAlphanumeric();

    if (!source.good() || source.eof()) {
        return Token{TokenType::END_OF_FILE, "", currentLine, currentColumn};
    }

    // Record the starting position of the token
    int tokenLine = currentLine;
    int tokenColumn = currentColumn;

    std::string lexeme;
    while (source.good()) {
        char c = peekChar();
        if (std::isalnum(c)) {
            lexeme += getChar(); // Consume the character
        } else {
            break;
        }
    }

    if (lexeme.empty()) {
        // Could be EOF or non-alphanumeric character
        if (!source.good() || source.eof()) {
            return Token{TokenType::END_OF_FILE, "", currentLine, currentColumn};
        } else {
            // Consume the unexpected character and return an error
            char unexpectedChar = getChar();
            std::string errorMsg = "Unexpected character: ";
            errorMsg += unexpectedChar;
            return Token{TokenType::ERROR, errorMsg, currentLine, currentColumn};
        }
    }

    // Determine the token type
    TokenType tokenType;

    if (isInstruction(lexeme)) {
        tokenType = TokenType::INSTRUCTION;
    } else if (lexeme[0] == 'x' && lexeme.length() > 1 && isDigitString(lexeme.substr(1))) {
        // Check if it's a register
        int regNumber = std::stoi(lexeme.substr(1));
        if (regNumber >= 1 && regNumber <= 32) {
            tokenType = TokenType::REGISTER;
        } else {
            // Throw an error for invalid register option
            return Token{TokenType::ERROR, "Invalid register number: " + lexeme, tokenLine, tokenColumn};
        }
    } else if (isDigitString(lexeme)) {
        tokenType = TokenType::IMMEDIATE;
    } else {
        tokenType = TokenType::LABEL;
    }

    return Token{tokenType, lexeme, tokenLine, tokenColumn};
}
