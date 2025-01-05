#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

//--------------------------------------------------------------
// Token-related definitions
//--------------------------------------------------------------
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
    // If you want to track where you found this token in the file:
    int line;
    int column;

    Token(TokenType t, std::string l, int ln = 0, int col = 0)
        : type(t), lexeme(std::move(l)), line(ln), column(col) {}

    bool operator==(const Token& other) const {
        return (this->type == other.type && this->lexeme == other.lexeme);
    }

    // Example compareTokenType if you want to compare punctuation, etc.
    bool compareTokenType(const Token& other) const {
        if (this->type == TokenType::PUNCTUATION 
            || other.type == TokenType::PUNCTUATION) {
            // If either is punctuation, make sure both are punctuation
            // and the lexeme matches.
            return (this->type == TokenType::PUNCTUATION 
                    && other.type == TokenType::PUNCTUATION
                    && this->lexeme == other.lexeme);
        }
        else {
            return (this->type == other.type);
        }
    }
};

//--------------------------------------------------------------
// Data structure for bit fields (unchanged)
//--------------------------------------------------------------
struct BitField {
    int bitCount;      // e.g. 5
    std::string field; // e.g. "00101" or "register1"
};

//--------------------------------------------------------------
// Helper to convert something like "register : any" -> Token
// Example inputs:
//   "register : any"       => TokenType::REGISTER, lexeme="any"
//   "punctuation : ,"      => TokenType::PUNCTUATION, lexeme=","
//   "immediate : some_num" => TokenType::IMMEDIATE, lexeme="some_num"
//   (etc.)
//--------------------------------------------------------------
Token parseParamStringToToken(const std::string& paramString)
{
    // Trim
    auto start = paramString.find_first_not_of(" \t");
    auto end   = paramString.find_last_not_of(" \t");
    std::string trimmed = (start == std::string::npos)
                        ? ""
                        : paramString.substr(start, end - start + 1);

    // Typically "register : any", so split on ':'
    size_t colonPos = trimmed.find(':');
    if (colonPos == std::string::npos) {
        // If no colon found, treat the whole thing as an error or fallback
        return Token(TokenType::ERROR, trimmed);
    }

    // Left of colon => something like "register", "punctuation", etc.
    std::string typePart = trimmed.substr(0, colonPos);
    // Right of colon => lexeme
    std::string lexemePart = trimmed.substr(colonPos + 1);

    // Trim both
    {
        auto s1 = typePart.find_first_not_of(" \t");
        auto e1 = typePart.find_last_not_of(" \t");
        if (s1 != std::string::npos && e1 != std::string::npos) {
            typePart = typePart.substr(s1, e1 - s1 + 1);
        }

        auto s2 = lexemePart.find_first_not_of(" \t");
        auto e2 = lexemePart.find_last_not_of(" \t");
        if (s2 != std::string::npos && e2 != std::string::npos) {
            lexemePart = lexemePart.substr(s2, e2 - s2 + 1);
        }
    }

    // Now map the typePart to a TokenType
    TokenType ttype = TokenType::ERROR;
    if (typePart == "instruction")  ttype = TokenType::INSTRUCTION;
    else if (typePart == "register")    ttype = TokenType::REGISTER;
    else if (typePart == "immediate")   ttype = TokenType::IMMEDIATE;
    else if (typePart == "label")       ttype = TokenType::LABEL;
    else if (typePart == "punctuation") ttype = TokenType::PUNCTUATION;
    // else, we keep it as ERROR or do something else

    return Token(ttype, lexemePart);
}

//--------------------------------------------------------------
// parseParamLine: produces instruction name plus a vector<Token>
//   Example line:
//     "load : register immediate [punctuation : (] register [punctuation : )]"
//
//   But more typically your new input lines are like:
//     "load : [register : any] [punctuation : ,] [immediate : some] ..."
//--------------------------------------------------------------
bool parseParamLine(
    const std::string &line, 
    std::string &outInstrName,
    std::vector<Token> &outTokens)
{
    // Clear output containers
    outInstrName.clear();
    outTokens.clear();

    // 1) Find colon to separate "INSTR_NAME" from the rest
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return false; // invalid format
    }

    // 2) Extract instruction name
    outInstrName = line.substr(0, colonPos);
    // Trim instruction name
    {
        auto start = outInstrName.find_first_not_of(" \t");
        auto end   = outInstrName.find_last_not_of(" \t");
        if (start == std::string::npos || end == std::string::npos) {
            return false; 
        }
        outInstrName = outInstrName.substr(start, end - start + 1);
    }

    // 3) The remainder is the parameter portion
    std::string remainder = line.substr(colonPos + 1);
    // Trim
    {
        auto start = remainder.find_first_not_of(" \t");
        auto end   = remainder.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            remainder = remainder.substr(start, end - start + 1);
        }
    }

    // 4) We'll parse bracket tokens "[ ... ]" as single tokens; 
    //    everything else is whitespace-separated, which might be plain words
    //    or might be "foo : bar" style strings.
    std::regex bracketRegex(R"(\[([^]]*)\])"); 
    auto begin = std::sregex_iterator(remainder.begin(), remainder.end(), bracketRegex);
    auto endIt = std::sregex_iterator();

    std::vector<std::pair<size_t, std::string>> bracketTokens; 
    for (std::sregex_iterator i = begin; i != endIt; ++i) {
        std::smatch match = *i;
        bracketTokens.push_back({ match.position(), match.str() });
    }

    size_t currentPos = 0;
    for (auto &bk : bracketTokens) {
        size_t bracketStart = bk.first;
        const std::string &fullBracketStr = bk.second; // e.g. "[register : any]"

        // The substring from currentPos to bracketStart = non-bracket text
        if (bracketStart > currentPos) {
            std::string nonBracket = remainder.substr(currentPos, bracketStart - currentPos);

            // Split on whitespace, parse each chunk if it looks like a param
            std::stringstream ss(nonBracket);
            std::string tokenChunk;
            while (ss >> tokenChunk) {
                // If the chunk might have a colon, parse as param
                // otherwise treat as error or something
                // For example, if it's "register" with no colon, 
                // you might interpret it as "register : ???"
                // or just store a default token.
                // For simplicity, let's do something minimal:
                size_t cpos = tokenChunk.find(':');
                if (cpos == std::string::npos) {
                    // No colon => treat it as register or immediate? Up to you.
                    // We'll just say error for now.
                    outTokens.push_back(Token(TokenType::ERROR, tokenChunk));
                } else {
                    // parse as "xxxx : yyyy"
                    outTokens.push_back(parseParamStringToToken(tokenChunk));
                }
            }
        }

        // Now parse the bracketed text: remove [ ]
        if (fullBracketStr.size() >= 2) {
            std::string inside = fullBracketStr.substr(1, fullBracketStr.size() - 2);
            // e.g. "register : any"
            Token t = parseParamStringToToken(inside);
            outTokens.push_back(t);
        }

        currentPos = bracketStart + fullBracketStr.size();
    }

    // 5) Parse any trailing text after the last bracket
    if (currentPos < remainder.size()) {
        std::string tail = remainder.substr(currentPos);
        std::stringstream ss(tail);
        std::string tokenChunk;
        while (ss >> tokenChunk) {
            size_t cpos = tokenChunk.find(':');
            if (cpos == std::string::npos) {
                outTokens.push_back(Token(TokenType::ERROR, tokenChunk));
            } else {
                outTokens.push_back(parseParamStringToToken(tokenChunk));
            }
        }
    }

    return true;
}

//--------------------------------------------------------------
// parseBinaryLine (unchanged, except you can keep it if needed)
//--------------------------------------------------------------
bool parseBinaryLine(const std::string &line, std::vector<BitField> &outBitFields)
{
    outBitFields.clear();

    std::regex bracketRegex(R"(\[([^]]*)\])");
    auto begin = std::sregex_iterator(line.begin(), line.end(), bracketRegex);
    auto endIt = std::sregex_iterator();

    for (std::sregex_iterator i = begin; i != endIt; ++i) {
        std::smatch match = *i;
        std::string inside = match[1].str(); // e.g. "5 : 00101"
        
        size_t colonPos = inside.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

        // Extract bitCount
        std::string numStr = inside.substr(0, colonPos);
        {
            auto s = numStr.find_first_not_of(" \t");
            auto e = numStr.find_last_not_of(" \t");
            if (s != std::string::npos && e != std::string::npos) {
                numStr = numStr.substr(s, e - s + 1);
            }
        }
        int bitCount = 0;
        try {
            bitCount = std::stoi(numStr);
        } catch(...) {
            continue;
        }

        // Extract field
        std::string fieldStr = inside.substr(colonPos + 1);
        {
            auto s = fieldStr.find_first_not_of(" \t");
            auto e = fieldStr.find_last_not_of(" \t");
            if (s != std::string::npos && e != std::string::npos) {
                fieldStr = fieldStr.substr(s, e - s + 1);
            }
        }

        BitField bf;
        bf.bitCount = bitCount;
        bf.field    = fieldStr;
        outBitFields.push_back(bf);
    }

    return true;
}

//--------------------------------------------------------------
// parseInstructionFile:
//   Reads lines in pairs: 
//     1) param line -> generates tokens
//     2) binary line -> generates bit fields
//--------------------------------------------------------------
bool parseInstructionFile(
    const std::string &filename,
    std::unordered_map<std::string, std::vector<Token>> &paramMap,
    std::unordered_map<std::string, std::vector<BitField>> &binaryMap)
{
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    std::string line;
    int lineCount = 0;

    std::string instrName;
    std::vector<Token> tokens;
    std::vector<BitField> bits;

    while (true) {
        // 1) Read param line
        if (!std::getline(infile, line)) {
            break; // no more lines
        }
        lineCount++;

        bool okParam = parseParamLine(line, instrName, tokens);
        if (!okParam) {
            std::cerr << "Warning: parseParamLine() failed on line " << lineCount << "\n";
            return false;
        }

        // 2) Read binary line
        if (!std::getline(infile, line)) {
            std::cerr << "Warning: instruction " << instrName 
                      << " has no binary-mapping line.\n";
            // Decide if you want to continue or break
            break;
        }
        lineCount++;

        bool okBits = parseBinaryLine(line, bits);
        if (!okBits) {
            std::cerr << "Warning: parseBinaryLine() failed on line " << lineCount << "\n";
            return false;
        }

        // 3) Store into maps
        paramMap[instrName]  = tokens;
        binaryMap[instrName] = bits;
    }

    return true;
}

//--------------------------------------------------------------
// Demo main
//--------------------------------------------------------------
int main() {
    // Now paramMap is: instructionName -> vector<Token>
    std::unordered_map<std::string, std::vector<Token>> paramMap;
    std::unordered_map<std::string, std::vector<BitField>> binaryMap;

    if (!parseInstructionFile("instructions.txt", paramMap, binaryMap)) {
        std::cerr << "Failed to parse instructions file.\n";
        return 1;
    }

    // Demonstration: print out paramMap
    std::cout << "=== Param Map ===\n";
    for (auto &kv : paramMap) {
        const auto &instr  = kv.first;
        const auto &tokens = kv.second;
        std::cout << instr << " : \n";
        for (auto &tok : tokens) {
            std::cout << "   TokenType=" << static_cast<int>(tok.type)
                      << ", lexeme='" << tok.lexeme << "'\n";
        }
        std::cout << "\n";
    }

    // Print out binaryMap
    std::cout << "\n=== Binary Map ===\n";
    for (auto &kv : binaryMap) {
        std::cout << kv.first << " : \n";
        for (auto &bf : kv.second) {
            std::cout << "   - " << bf.bitCount << " : " << bf.field << "\n";
        }
    }

    return 0;
}
