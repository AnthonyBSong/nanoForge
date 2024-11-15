enum class TokenType {
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    LABEL,
    END_OF_LINE,
    ERROR
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};