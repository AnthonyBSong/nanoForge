#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <regex>

// A simple struct for the bracketed bitfields, e.g. "[5 : 00000]"
struct BitField {
    int bitCount;      // e.g., 5
    std::string field; // e.g., "00000" or "register1"
};

// A simple function to split the parameter pattern by whitespace.
std::vector<std::string> tokenizeParamPattern(const std::string &paramsStr) {
    std::vector<std::string> tokens;
    std::stringstream ss(paramsStr);
    std::string word;
    while (ss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

int main() {
    // Maps: instruction -> vector of parameter tokens
    std::unordered_map<std::string, std::vector<std::string>> paramMap;
    // Maps: instruction -> vector of BitField (bitCount, field)
    std::unordered_map<std::string, std::vector<BitField>> binaryMap;

    // Open your specification file
    std::ifstream infile("instructions.txt");
    if (!infile.is_open()) {
        std::cerr << "Failed to open instructions.txt.\n";
        return 1;
    }

    std::string line;
    while (std::getline(infile, line)) {
        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Find the colon that separates instruction name from the rest
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            // No colon -> invalid format? Skip or handle error
            continue;
        }

        // Extract instruction name (trim whitespace)
        std::string instructionName = line.substr(0, colonPos);
        {
            // Trim from both ends
            auto start = instructionName.find_first_not_of(" \t");
            auto end   = instructionName.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                instructionName = instructionName.substr(start, end - start + 1);
            }
        }

        // The remainder (after the colon)
        std::string remainder = line.substr(colonPos + 1);
        // Trim
        {
            auto start = remainder.find_first_not_of(" \t");
            auto end   = remainder.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                remainder = remainder.substr(start, end - start + 1);
            }
        }

        // Now, we want to separate the parameter pattern from any bracket fields.
        // We'll look for the first '[' (if any).
        size_t bracketPos = remainder.find('[');
        std::string paramsPart, bitsPart;
        if (bracketPos == std::string::npos) {
            // No bracket -> everything is the parameter pattern
            paramsPart = remainder;
        } else {
            paramsPart = remainder.substr(0, bracketPos);
            bitsPart   = remainder.substr(bracketPos);
        }

        // Tokenize the paramsPart by whitespace into a vector
        auto parameters = tokenizeParamPattern(paramsPart);
        // Store in the paramMap
        paramMap[instructionName] = parameters;

        // Use a regex to find bracketed segments: "[...]"
        // Each bracketed segment might be like "5 : 00000" or "3 : register1"
        std::vector<BitField> bitFields;
        std::regex bracketRegex(R"(\[([^]]*)\])"); // captures text within [ ... ]
        auto begin = std::sregex_iterator(bitsPart.begin(), bitsPart.end(), bracketRegex);
        auto endIt = std::sregex_iterator();

        for (std::sregex_iterator i = begin; i != endIt; ++i) {
            std::smatch match = *i;
            std::string inside = match[1].str(); // content of the brackets, e.g. "5 : 00000"

            // We now expect something like "<N> : <field>", e.g. "5 : 00000"
            BitField bf;
            bf.bitCount = 0;
            bf.field    = "";

            // 1) Locate the colon
            size_t localColonPos = inside.find(':');
            if (localColonPos != std::string::npos) {
                // The substring before the colon is presumably the number of bits
                std::string numberPart = inside.substr(0, localColonPos);
                // Trim
                {
                    auto s = numberPart.find_first_not_of(" \t");
                    auto e = numberPart.find_last_not_of(" \t");
                    if (s != std::string::npos && e != std::string::npos) {
                        numberPart = numberPart.substr(s, e - s + 1);
                    }
                }

                // Convert numberPart to int
                bf.bitCount = std::stoi(numberPart);

                // The substring after the colon is the field
                std::string fieldPart = inside.substr(localColonPos + 1);
                // Trim
                {
                    auto s = fieldPart.find_first_not_of(" \t");
                    auto e = fieldPart.find_last_not_of(" \t");
                    if (s != std::string::npos && e != std::string::npos) {
                        fieldPart = fieldPart.substr(s, e - s + 1);
                    }
                }
                bf.field = fieldPart; // e.g. "00000" or "register1"
            }

            bitFields.push_back(bf);
        }

        // Store the bit fields in the binaryMap
        binaryMap[instructionName] = bitFields;
    }

    infile.close();

    // --- DEMO: Print out what we have parsed ---
    std::cout << "=== Param Map ===\n";
    for (auto &kv : paramMap) {
        std::cout << kv.first << " :";
        for (auto &p : kv.second) {
            std::cout << " " << p;
        }
        std::cout << "\n";
    }

    std::cout << "\n=== Binary Map ===\n";
    for (auto &kv : binaryMap) {
        std::cout << kv.first << " :\n";
        for (auto &bf : kv.second) {
            std::cout << "   - " << bf.bitCount
                      << " : " << bf.field << "\n";
        }
    }

    return 0;
}
