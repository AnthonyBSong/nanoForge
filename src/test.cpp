#include <unordered_set>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include "../include/Token.hpp"
#include "../include/Lexer.hpp"

int main() {
    // Define the set of instructions
    std::unordered_set<std::string> instructions = {
        "lui", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu",
        "bgeu", "lw", "sw", "addi", "ori", "andi", "add", "sub",
        "sll", "srl", "sra", "or", "and"
    };

    // Example source code as a string (could be read from a file).
    // Notice it includes parentheses '(' and ')', as well as labels and various tokens.
    std::string sourceCode = 
        "label:\n"
        "addi x1, x2, (0b1010)\n"  // Parentheses around a binary immediate
        "lw x3, (x4)\n"            // Parentheses around a register
        "jal x5, end_label\n"
        "(addi x6, x7, 0x1f)\n"    // Entire instruction in parentheses
        "end_label:\n"
        "sw x8, (x9)\n";

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

        // Print all tokens using the Lexer’s built-in method
        std::cout << "=== Printing All Tokens (Lexer Output) ===\n";
        lexer.printTokens();

        // Demonstrate consuming tokens from the deque
        std::cout << "\n=== Consuming Tokens (Deque) ===\n";
        while (!tokenDeque.empty()) {
            Token token = std::move(tokenDeque.front());
            tokenDeque.pop_front();

            // Print the consumed token
            std::cout << "Consumed Token: \"" << token.lexeme << "\", Type: ";
            switch (token.type) {
                case TokenType::INSTRUCTION: std::cout << "INSTRUCTION"; break;
                case TokenType::REGISTER:    std::cout << "REGISTER";    break;
                case TokenType::IMMEDIATE:   std::cout << "IMMEDIATE";   break;
                case TokenType::LABEL:       std::cout << "LABEL";       break;
                case TokenType::PARENS:      std::cout << "PARENS";      break;
                case TokenType::PARENE:      std::cout << "PARENE";      break;
                case TokenType::EoL:         std::cout << "EoL";         break;
                case TokenType::EoF:         std::cout << "EoF";         break;
                case TokenType::ERROR:       std::cout << "ERROR";       break;
                default:                     std::cout << "UNKNOWN";     break;
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
