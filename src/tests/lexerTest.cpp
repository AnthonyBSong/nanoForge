#include "../compiler/lexer/Lexer.h"  // Adjust the path if needed
#include <gtest/gtest.h>
#include <sstream>
#include <string>

// Utility function to create a sample input stream
std::istringstream createInputStream(const std::string& input) {
    return std::istringstream(input);
}

// Test Suite for Lexer
class LexerTest : public ::testing::Test {
protected:
    // Common setup can go here if needed
};

// Test case: Basic tokenization
TEST_F(LexerTest, BasicTokenization) {
    std::string input = "add";
    auto source = createInputStream(input);
    Lexer lexer(source);

    // Assuming Lexer has a method `nextToken()` to fetch tokens
    auto token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::INSTRUCTION);
    EXPECT_EQ(token.lexeme, "add");

    // Additional checks for other tokens
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
