#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

// should probably create a paramType struct with first value being the type:
// and the second being the value:. So like it could be type: punctuation
// value: (.

//---------------------------------
// Data structure for bit fields
//---------------------------------
struct BitField {
    int bitCount;      // e.g. 5
    std::string field; // e.g. "00101" or "register1"
};

//---------------------------------
// parseParamLine:
//   Parses something like:
//     "load : register immediate [punctuation : (] register [punctuation : )]"
//   Returns the instruction name, plus a list of parameter tokens.
//
//   We expect a format like:
//     INSTR_NAME : param param param ...
//   Where some params may be bracketed with "[punctuation : (]" etc.
//
//   Example paramMap entry:
//     paramMap["load"] = {
//       "register", "immediate", "punctuation : (", "register", "punctuation : )"
//     };
//---------------------------------
bool parseParamLine(
    const std::string &line, 
    std::string &outInstrName,
    std::vector<std::string> &outParams)
{
    // Clear output containers
    outInstrName.clear();
    outParams.clear();

    // 1) Find colon to separate "INSTR_NAME" from the rest
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return false; // invalid format
    }

    // 2) Extract instruction name
    outInstrName = line.substr(0, colonPos);
    // Trim
    {
        auto start = outInstrName.find_first_not_of(" \t");
        auto end   = outInstrName.find_last_not_of(" \t");
        if (start == std::string::npos || end == std::string::npos) {
            return false; 
        }
        outInstrName = outInstrName.substr(start, end - start + 1);
    }

    // 3) The remainder is the parameter + bracket stuff
    std::string remainder = line.substr(colonPos + 1);
    // Trim
    {
        auto start = remainder.find_first_not_of(" \t");
        auto end   = remainder.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            remainder = remainder.substr(start, end - start + 1);
        }
    }

    // 4) We can parse out bracket tokens "[punctuation : (]" as a single token
    //    and also parse whitespace-separated tokens in between.
    //    We'll do a simple approach: use a regex to find bracket segments,
    //    then replace them with a placeholder, then split.
    //
    //    Or we can do a two-step approach: 
    //    - find bracketed segments with a regex
    //    - everything outside is whitespace tokens.
    //    For the bracket content, we keep it as is, e.g. "punctuation : (".
    
    // a) We'll gather bracket tokens in order
    // b) We'll gather non-bracket tokens in between
    // c) Combine them into outParams in the correct sequence

    std::vector<std::pair<size_t, std::string>> bracketTokens; // (position, bracketContent)

    // Regex to capture [ ... ]
    std::regex bracketRegex(R"(\[([^]]*)\])"); 
    auto begin = std::sregex_iterator(remainder.begin(), remainder.end(), bracketRegex);
    auto endIt = std::sregex_iterator();

    // We'll store the match positions and content
    for (std::sregex_iterator i = begin; i != endIt; ++i) {
        std::smatch match = *i;
        // match.str() includes the brackets [ ... ], match[1] is inside
        std::string fullBracket = match.str();   // e.g. "[punctuation : (]"
        size_t bracketStart     = match.position(); // index in remainder
        bracketTokens.push_back({ bracketStart, fullBracket });
    }

    // We'll parse from left to right, collecting whitespace tokens
    // up until we hit a bracket. Then we'll push the bracket token as-is.
    size_t currentPos = 0;
    for (size_t idx = 0; idx < bracketTokens.size(); ++idx) {
        auto &bk = bracketTokens[idx];
        size_t bracketStart = bk.first;
        std::string bracketStr = bk.second; // e.g. "[punctuation : (]"

        // The substring from currentPos to bracketStart is non-bracket text
        if (bracketStart > currentPos) {
            std::string nonBracket = remainder.substr(currentPos, bracketStart - currentPos);
            // Split that by whitespace
            std::stringstream ss(nonBracket);
            std::string token;
            while (ss >> token) {
                outParams.push_back(token);
            }
        }
        // Now push the bracket token literally, but we might want to strip off
        // the outer brackets and keep the inside exactly. 
        // e.g. if bracketStr = "[punctuation : (]", inside is "punctuation : ("
        {
            // Chop off the leading '[' and trailing ']'
            // bracketStr.size() >= 2, presumably
            std::string insideBrackets = bracketStr.substr(1, bracketStr.size() - 2); 
            // e.g. "punctuation : ("
            // We will store that as a single token
            outParams.push_back(insideBrackets);
        }

        // Move currentPos to the end of the bracket
        currentPos = bracketStart + bracketStr.size();
    }

    // Finally, parse any trailing text after the last bracket
    if (currentPos < remainder.size()) {
        std::string tail = remainder.substr(currentPos);
        std::stringstream ss(tail);
        std::string token;
        while (ss >> token) {
            outParams.push_back(token);
        }
    }

    // Done parsing param line
    return true;
}

//---------------------------------
// parseBinaryLine:
//   Parses something like:
//     "[5 : 00101] [3 : register1] [6 : immediateVal]"
//   We store the results in a vector of BitField.
//
//   e.g. binaryMap["load"] -> { {5, "00101"}, {3, "register1"}, {6, "immediateVal"} }
//---------------------------------
bool parseBinaryLine(const std::string &line, std::vector<BitField> &outBitFields)
{
    outBitFields.clear();

    // We'll look for bracketed segments: "[N : field]"
    std::regex bracketRegex(R"(\[([^]]*)\])"); // captures text within [ ... ]
    auto begin = std::sregex_iterator(line.begin(), line.end(), bracketRegex);
    auto endIt = std::sregex_iterator();

    for (std::sregex_iterator i = begin; i != endIt; ++i) {
        std::smatch match = *i;
        std::string inside = match[1].str(); // e.g. "5 : 00101"
        
        // We expect "<N> : <field>"
        size_t colonPos = inside.find(':');
        if (colonPos == std::string::npos) {
            // Invalid format => skip or handle error
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
            // Handle error if needed
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

//---------------------------------
// parseInstructionFile:
//   Reads the file line-by-line in *pairs*:
//     - First line: param line
//     - Second line: binary line
//   Populates paramMap and binaryMap accordingly.
//
//   If the file has an odd number of lines, the last line won't have a pair
//   => handle that as you see fit (e.g., ignore or error).
//---------------------------------
bool parseInstructionFile(
    const std::string &filename,
    std::unordered_map<std::string, std::vector<std::string>> &paramMap,
    std::unordered_map<std::string, std::vector<BitField>>     &binaryMap)
{
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << filename << "\n";
        return false;
    }

    std::string line;
    int lineCount = 0;
    std::string instrName;
    std::vector<std::string> params;
    std::vector<BitField> bits;

    while (true) {
        // 1) Read param line
        if (!std::getline(infile, line)) {
            break; // no more lines
        }
        lineCount++;

        // parse param line
        bool okParam = parseParamLine(line, instrName, params);
        if (!okParam) {
            std::cerr << "Warning: parseParamLine() failed on line " << lineCount << "\n";
            // Decide whether to continue or return false
            return false;
        }

        // 2) Read binary line
        if (!std::getline(infile, line)) {
            // No second line => incomplete pair
            std::cerr << "Warning: instruction " << instrName 
                      << " has no binary-mapping line.\n";
            // Decide if you want to break or treat as error
            break;
        }
        lineCount++;

        // parse binary line
        bool okBits = parseBinaryLine(line, bits);
        if (!okBits) {
            std::cerr << "Warning: parseBinaryLine() failed on line " << lineCount << "\n";
            return false;
        }

        // 3) Store into maps
        paramMap[instrName]  = params;
        binaryMap[instrName] = bits;
    }

    return true;
}

int main() {
    // Maps we want to fill
    std::unordered_map<std::string, std::vector<std::string>> paramMap;
    std::unordered_map<std::string, std::vector<BitField>>     binaryMap;

    // Example usage
    if (!parseInstructionFile("instructions.txt", paramMap, binaryMap)) {
        std::cerr << "Failed to parse instructions file.\n";
        return 1;
    }

    // Demo: print what we parsed
    std::cout << "=== Param Map ===\n";
    for (auto &kv : paramMap) {
        std::cout << kv.first << " : ";
        for (auto &p : kv.second) {
            std::cout << "[" << p << "] ";
        }
        std::cout << "\n";
    }

    std::cout << "\n=== Binary Map ===\n";
    for (auto &kv : binaryMap) {
        std::cout << kv.first << " : \n";
        for (auto &bf : kv.second) {
            std::cout << "   - " << bf.bitCount << " : " << bf.field << "\n";
        }
    }

    return 0;
}
