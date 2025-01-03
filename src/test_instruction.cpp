#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

//===================
// Copy of your structs & functions
//===================

// Data structure for parameters (optional in your final code)
struct Param {
    std::string type;
    std::string value;
};

// Data structure for bit fields
struct BitField {
    int bitCount;
    std::string field;
};

// parseParamLine
bool parseParamLine(const std::string &line, std::string &outInstrName, std::vector<std::string> &outParams)
{
    outInstrName.clear();
    outParams.clear();

    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return false; // invalid format
    }

    // Instruction name
    outInstrName = line.substr(0, colonPos);
    {
        auto start = outInstrName.find_first_not_of(" \t");
        auto end   = outInstrName.find_last_not_of(" \t");
        if (start == std::string::npos || end == std::string::npos) {
            return false;
        }
        outInstrName = outInstrName.substr(start, end - start + 1);
    }

    // The remainder
    std::string remainder = line.substr(colonPos + 1);
    {
        auto start = remainder.find_first_not_of(" \t");
        auto end   = remainder.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            remainder = remainder.substr(start, end - start + 1);
        }
    }

    // Regex to find bracketed tokens
    std::regex bracketRegex(R"(\[([^]]*)\])");
    std::vector<std::pair<size_t, std::string>> bracketTokens;
    auto begin = std::sregex_iterator(remainder.begin(), remainder.end(), bracketRegex);
    auto endIt = std::sregex_iterator();
    for (auto i = begin; i != endIt; ++i) {
        std::smatch match = *i;
        std::string fullBracket = match.str();
        size_t bracketStart     = match.position();
        bracketTokens.push_back({ bracketStart, fullBracket });
    }

    // Collect tokens
    size_t currentPos = 0;
    for (size_t idx = 0; idx < bracketTokens.size(); ++idx) {
        auto &bk = bracketTokens[idx];
        size_t bracketStart = bk.first;
        std::string bracketStr = bk.second;

        // Non-bracket portion
        if (bracketStart > currentPos) {
            std::string nonBracket = remainder.substr(currentPos, bracketStart - currentPos);
            std::stringstream ss(nonBracket);
            std::string token;
            while (ss >> token) {
                outParams.push_back(token);
            }
        }
        // The bracket itself
        {
            std::string inside = bracketStr.substr(1, bracketStr.size() - 2);
            outParams.push_back(inside);
        }

        currentPos = bracketStart + bracketStr.size();
    }

    // Trailing text after the last bracket
    if (currentPos < remainder.size()) {
        std::string tail = remainder.substr(currentPos);
        std::stringstream ss(tail);
        std::string token;
        while (ss >> token) {
            outParams.push_back(token);
        }
    }

    return true;
}

// parseBinaryLine
bool parseBinaryLine(const std::string &line, std::vector<BitField> &outBitFields)
{
    outBitFields.clear();
    std::regex bracketRegex(R"(\[([^]]*)\])");
    auto begin = std::sregex_iterator(line.begin(), line.end(), bracketRegex);
    auto endIt = std::sregex_iterator();

    for (auto i = begin; i != endIt; ++i) {
        std::smatch match = *i;
        std::string inside = match[1].str();

        size_t colonPos = inside.find(':');
        if (colonPos == std::string::npos) {
            continue;
        }

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

// parseInstructionFile
bool parseInstructionFile(
    const std::string &filename,
    std::unordered_map<std::string, std::vector<std::string>> &paramMap,
    std::unordered_map<std::string, std::vector<BitField>> &binaryMap)
{
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    std::string line;
    int lineCount = 0;
    while (true) {
        // Attempt to read param line
        if (!std::getline(infile, line)) {
            break;
        }
        lineCount++;

        std::string instrName;
        std::vector<std::string> params;
        bool okParam = parseParamLine(line, instrName, params);
        if (!okParam) {
            std::cerr << "Warning: parseParamLine() failed on line " << lineCount << "\n";
            return false;
        }

        // Attempt to read binary line
        if (!std::getline(infile, line)) {
            // No matching binary line
            std::cerr << "Warning: instruction " << instrName 
                      << " has no binary-mapping line.\n";
            break;
        }
        lineCount++;

        std::vector<BitField> bits;
        bool okBits = parseBinaryLine(line, bits);
        if (!okBits) {
            std::cerr << "Warning: parseBinaryLine() failed on line " << lineCount << "\n";
            return false;
        }

        // Store results
        paramMap[instrName]  = params;
        binaryMap[instrName] = bits;
    }

    return true;
}

//===================
// MAIN - Test
//===================
int main()
{
    // 1) Create a small instructions file
    {
        std::ofstream ofs("test_instructions.txt");
        if (!ofs.is_open()) {
            std::cerr << "Failed to create test_instructions.txt\n";
            return 1;
        }
        
        // Each instruction has two lines:
        // (1) Param line
        // (2) Binary line
        ofs << "addi : register register [punctuation : (] immediate [punctuation : )]\n";
        ofs << "[5 : 01100] [3 : register1] [3 : register2] [8 : immediateVal]\n";

        ofs << "lw : register [punctuation : (] register [punctuation : )]\n";
        ofs << "[5 : 10000] [3 : register1] [3 : register2]\n";

        ofs << "jal : register immediate\n";
        ofs << "[5 : 10110] [3 : register1] [8 : immediateVal]\n";

        ofs << "sw : register [punctuation : (] register [punctuation : )]\n";
        ofs << "[5 : 10101] [3 : register1] [3 : register2]\n";
        
        ofs.close();
    }

    // 2) Parse that file into paramMap and binaryMap
    std::unordered_map<std::string, std::vector<std::string>> paramMap;
    std::unordered_map<std::string, std::vector<BitField>> binaryMap;

    if (!parseInstructionFile("test_instructions.txt", paramMap, binaryMap)) {
        std::cerr << "parseInstructionFile() failed.\n";
        return 1;
    }

    // 3) Print out results
    std::cout << "\n==== Param Map ====\n";
    for (auto &kv : paramMap) {
        const std::string &instrName = kv.first;
        const auto &params = kv.second;
        std::cout << instrName << " : ";
        for (auto &p : params) {
            std::cout << "[" << p << "] ";
        }
        std::cout << "\n";
    }

    std::cout << "\n==== Binary Map ====\n";
    for (auto &kv : binaryMap) {
        const std::string &instrName = kv.first;
        const auto &bits = kv.second;
        std::cout << instrName << " :\n";
        for (auto &bf : bits) {
            std::cout << "  - " << bf.bitCount << " : " << bf.field << "\n";
        }
    }

    // 4) Show the "test assembly code" snippet
    std::cout << "\n=== Example Assembly Snippet to match (not fully parsed here) ===\n";
    std::string testAsm = 
        "label:\n"
        "addi x1, x2, (0b1010)\n"
        "lw x3, (x4)\n"
        "jal x5, end_label\n"
        "(addi x6, x7, 0x1f)\n"
        "end_label:\n"
        "sw x8, (x9)\n";

    std::cout << testAsm << "\n\n";

    std::cout << "Note: The above code snippet is not automatically parsed by this test.\n"
                 "We've only demonstrated that 'addi', 'lw', 'jal', 'sw' instructions\n"
                 "are defined in test_instructions.txt in a way consistent with these lines.\n"
                 "A real assembler would match tokens in 'addi x1, x2, (0b1010)' against\n"
                 "the pattern [register, register, punctuation : (, immediate, punctuation : )], etc.\n";

    return 0;
}
