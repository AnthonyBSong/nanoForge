#include <unordered_set>
#include <deque>
#include <fstream>
#include <iostream>
#include <string>
#include "../include/Token.hpp"
#include "../include/Lexer.hpp"

int main() {
    // Define instructions
    std::unordered_set<std::string> instructions = {
        "lui", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu",
        "bgeu", "lw", "sw", "addi", "ori", "andi", "add", "sub",
        "sll", "srl", "sra", "or", "and"
    };

    // Define punctuation
    std::unordered_set<std::string> punctuation = {
        "(", ")", ":"
    };

    // Example source code
    std::string sourceCode = 
        "label:\n"
        "addi x1, x2, (0b1010)\n"  
        "lw x3, (x4)\n"
        "jal x5, end_label\n"
        "(addi x6, x7, 0x1f)\n"
        "end_label:\n"
        "sw x8, (x9)\n";

    // Write to temp file
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
        // Pass instructions + punctuation sets
        Lexer lexer(source, tokenDeque, instructions, punctuation);

        // Print all tokens
        std::cout << "=== Printing All Tokens (Lexer Output) ===\n";
        lexer.printTokens();

        std::cout << "\n=== Consuming Tokens (Deque) ===\n";
        while (!tokenDeque.empty()) {
            Token token = std::move(tokenDeque.front());
            tokenDeque.pop_front();

            std::cout << "Consumed Token: \"" << token.lexeme << "\", Type: ";
            switch (token.type) {
                case TokenType::INSTRUCTION:   std::cout << "INSTRUCTION";   break;
                case TokenType::REGISTER:      std::cout << "REGISTER";      break;
                case TokenType::IMMEDIATE:     std::cout << "IMMEDIATE";     break;
                case TokenType::LABEL:         std::cout << "LABEL";         break;
                case TokenType::PUNCTUATION:   std::cout << "PUNCTUATION";   break;
                case TokenType::EoL:           std::cout << "EoL";           break;
                case TokenType::EoF:           std::cout << "EoF";           break;
                case TokenType::ERROR:         std::cout << "ERROR";         break;
            }
            std::cout << "\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Lexer Error: " << e.what() << "\n";
    }

    // Clean up
    source.close();
    if (std::remove("temp_source.asm") != 0) {
        std::cerr << "Warning: Failed to delete temporary source file.\n";
    }
    return 0;
}
