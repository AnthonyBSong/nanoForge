#include <unordered_set>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include "../include/Token.h"
#include "../include/Lexer.h"

int main() {
    // Define the set of instructions
    std::unordered_set<std::string> instructions = {
        "lui", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu",
        "bgeu", "lw", "sw", "addi", "ori", "andi", "add", "sub",
        "sll", "srl", "sra", "or", "and"
    };

    // Example source code as a string (could be read from a file)
    std::string sourceCode = "addi x1, x2, 10\njal x3, label";

    // Write the source code to a temporary file for demonstration
    std::ofstream tempFile("temp_source.asm");
    if (!tempFile.is_open()) {
        std::cerr << "Failed to create temporary source file.\n";
        return 1;
    }
    tempFile << sourceCode;
    tempFile.close();

    // Open the file for reading
    std::ifstream source("temp_source.asm");
    if (!source.is_open()) {
        std::cerr << "Failed to open the source file.\n";
        return 1;
    }

    // Create an empty deque<Token>
    std::deque<Token> tokenDeque;

    try {
        // Create a Lexer instance, passing the source file, the empty deque, and the instructions set
        Lexer lexer(source, tokenDeque, instructions);

        // Print all tokens
        lexer.printTokens();

        // Example of consuming tokens
        std::cout << "\nConsuming tokens:\n";
        while (!tokenDeque.empty()) {
            Token token = std::move(tokenDeque.front());
            tokenDeque.pop_front();
            // Process the token as needed (for demonstration, just print it)
            std::cout << "Processed Token: \"" << token.lexeme << "\" of type ";

            switch (token.type) {
                case TokenType::INSTRUCTION:
                    std::cout << "INSTRUCTION";
                    break;
                case TokenType::REGISTER:
                    std::cout << "REGISTER";
                    break;
                case TokenType::IMMEDIATE:
                    std::cout << "IMMEDIATE";
                    break;
                case TokenType::LABEL:
                    std::cout << "LABEL";
                    break;
                case TokenType::END_OF_LINE:
                    std::cout << "END_OF_LINE";
                    break;
                case TokenType::ERROR:
                    std::cout << "ERROR";
                    break;
                default:
                    std::cout << "UNKNOWN";
                    break;
            }
            std::cout << "\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Lexer Error: " << e.what() << "\n";
    }

    // Clean up the temporary file
    source.close();
    if (std::remove("temp_source.asm") != 0) {
        std::cerr << "Warning: Failed to delete temporary source file.\n";
    }

    return 0;
}
