#include "include/Lexer.h"

#include <string_view>
#include <vector>
#include <deque>
#include <regex>
#include <fstream>

// Define TokenType and Token as per your specifications
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

// Precomputed instruction codes (packed 4-char codes)
constexpr uint32_t instruction_codes[] = {
    0x6C756900, // "lui"
    0x6A616C00, // "jal"
    0x6A616C72, // "jalr"
    0x62657100, // "beq"
    0x626E6500, // "bne"
    0x626C7400, // "blt"
    0x62676500, // "bge"
    0x626C7475, // "bltu"
    0x62676575, // "bgeu"
    0x6C770000, // "lw"
    0x73770000, // "sw"
    0x61646469, // "addi"
    0x6F726900, // "ori"
    0x616E6469, // "andi"
    0x61646400, // "add"
    0x73756200, // "sub"
    0x736C6C00, // "sll"
    0x73726C00, // "srl"
    0x73726100, // "sra"
    0x6F720000, // "or"
    0x616E6400  // "and"
};

constexpr int num_instructions = sizeof(instruction_codes) / sizeof(instruction_codes[0]);

// Function to pack up to 4 characters into a 32-bit integer
inline uint32_t pack4(const std::string_view& str_view) {
    uint32_t code = 0;
    // Process up to 4 characters
    for (size_t i = 0; i < 4 && i < str_view.size(); ++i) {
        uint8_t c = (uint8_t)str_view[i];
        code |= (uint32_t)c << (8 * (3 - i));
    }
    return code;
}

// Function to check if the token is an instruction
inline bool is_instruction(uint32_t code) {
    // Use bitwise operations to compare codes
    for (int i = 0; i < num_instructions; ++i) {
        if (code == instruction_codes[i]) {
            return true;
        }
    }
    return false;
}

// Function to check if the token is a register
inline bool is_register(const char* str, size_t length) {
    // Check if the string starts with 'x' and has at least one digit
    if (length < 2 || str[0] != 'x') return false;

    uint32_t reg_num = 0;
    for (size_t i = 1; i < length; ++i) {
        uint8_t c = (uint8_t)str[i];
        uint8_t digit = c - '0';
        if (digit > 9) return false;
        reg_num = reg_num * 10 + digit;
    }
    return reg_num <= 31;
}

// Function to check if the token is an immediate value
inline bool is_immediate(const char* str, size_t length) {
    if (length == 0) return false;

    uint8_t first_char = (uint8_t)str[0];
    uint8_t second_char = length >= 2 ? (uint8_t)str[1] : 0;

    size_t i = 0;

    // Check for binary immediate (0b...)
    if (first_char == '0' && second_char == 'b') {
        if (length <= 2) return false;  // Must have digits after '0b'
        i = 2;
        while (i < length) {
            uint8_t c = (uint8_t)str[i++];
            if (c != '0' && c != '1') return false;
        }
        return true;
    }
    // Check for hexadecimal immediate (0x...)
    else if (first_char == '0' && second_char == 'x') {
        if (length <= 2) return false;  // Must have digits after '0x'
        i = 2;
        while (i < length) {
            uint8_t c = (uint8_t)str[i++];
            c |= 32;  // Convert to lowercase
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
        }
        return true;
    }
    // Check for decimal immediate
    else {
        while (i < length) {
            uint8_t c = (uint8_t)str[i++];
            if (c < '0' || c > '9') return false;
        }
        return i > 0;  // At least one digit
    }
}

// Function to check if the token is a label
inline bool is_label(const char* str, size_t length) {
    if (length == 0 || str[length - 1] != ':') return false;

    // Labels shouldn't contain spaces represented by '_'
    for (size_t i = 0; i < length - 1; ++i) {
        if (str[i] == '_') return false;
    }
    return true;
}

// Main tokenization function
Token tokenize(const std::string_view& token_str_view, int line, int column) {
    const char* str = token_str_view.data();
    size_t length = token_str_view.size();

    uint32_t code = pack4(token_str_view);

    if (is_instruction(code)) {
        return {TokenType::INSTRUCTION, std::string(token_str_view), line, column};
    } else if (is_register(str, length)) {
        return {TokenType::REGISTER, std::string(token_str_view), line, column};
    } else if (is_immediate(str, length)) {
        return {TokenType::IMMEDIATE, std::string(token_str_view), line, column};
    } else if (is_label(str, length)) {
        return {TokenType::LABEL, std::string(token_str_view), line, column};
    } else {
        return {TokenType::ERROR, std::string(token_str_view), line, column};
    }
}

// 0 indexing for line/column of the source .s or .asm file
Lexer::Lexer(std::ifstream& source) : currentLine(0), currentColumn(0) {
    if (!source.is_open()) throw std::runtime_error("Source file not found!");

    std::string line;
    std::regex re("[a-zA-Z0-9_]+");
    while (std::getline(source, line)) {
            auto it = std::sregex_iterator(line.begin(), line.end(), re);
            auto end = std::sregex_iterator();

            for (; it != end; ++it) {
                const auto& match = *it;

                // Get the position and length of the token
                int tokenPosition = match.position(0);
                int tokenLength = match.length(0);

                // Update currentColumn to the position of the token
                currentColumn = tokenPosition + 1;  // Columns start at 1

                // Create a string_view to the token within the line
                std::string tokenStr(line.data() + tokenPosition, tokenLength);

                // Tokenize and push the token into parsedFile
                Token tok = tokenize(tokenStr, currentLine, currentColumn);
                parsedFile.push_back(tok);
            }

            //push END_OF_LINE at end of the line
            parsedFile.push_back(Token(TokenType::END_OF_LINE, "\n", currentLine, currentColumn + 1));


            // Move to the next line
            currentLine++;
            currentColumn = 1;  // Reset column at the start of a new line
    }
};
