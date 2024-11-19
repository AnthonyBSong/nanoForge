#ifndef LEXER_H
#define LEXER_H

#include <deque>
#include <fstream>
#include <string>
#include <unordered_set>

// Define TokenType enum
enum class TokenType {
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    LABEL,
    END_OF_LINE,
    ERROR
};

// Define Token struct
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    // Constructor
    Token(TokenType t, std::string lex, int ln, int col);
};

// Lexer class declaration
class Lexer {
public:
    // Constructor takes a reference to an empty deque and an unordered_set of instructions
    Lexer(std::ifstream& source, std::deque<Token>& parsedFileRef, const std::unordered_set<std::string>& instructionsSet);

    // Token consumption functions
    bool hasMoreTokens() const;
    const Token& peekNextToken() const;
    Token getNextToken();

    // Function to print all tokens (optional, for debugging)
    void printTokens() const;

private:
    int currentLine;
    int currentColumn;
    std::deque<Token>& parsedFile;                           // Reference to deque<Token>
    const std::unordered_set<std::string>& instructions;     // Reference to instruction set

    // Tokenization function
    Token tokenize(const char* str, size_t length, int line, int column);
};

#endif // LEXER_H
