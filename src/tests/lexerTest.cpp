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
    std::string input = "int main() { return 0; }";
    auto source = createInputStream(input);
    Lexer lexer(source);

    // Assuming Lexer has a method `nextToken()` to fetch tokens
    auto token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Keyword);
    EXPECT_EQ(token.value, "int");

    // Additional checks for other tokens
}

// Test case: Handling of special characters
TEST_F(LexerTest, SpecialCharacters) {
    std::string input = "@#$%^&*";
    auto source = createInputStream(input);
    Lexer lexer(source);

    // Assuming Lexer returns `Invalid` or similar for unknown tokens
    auto token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::Invalid);
}

// Test case: Handling numbers and identifiers
TEST_F(LexerTest, NumbersAndIdentifiers) {
    std::string input = "var1 = 12345";
    auto source = createInputStream(input);
    Lexer lexer(source);

    auto token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::Identifier);
    EXPECT_EQ(token1.value, "var1");

    auto token2 = lexer.nextToken();
    EXPECT_EQ(token2.type, TokenType::Operator);  // Assuming '=' is an operator

    auto token3 = lexer.nextToken();
    EXPECT_EQ(token3.type, TokenType::Number);
    EXPECT_EQ(token3.value, "12345");
}

// Test case: End of file (EOF) handling
TEST_F(LexerTest, EOFHandling) {
    std::string input = "int a;";
    auto source = createInputStream(input);
    Lexer lexer(source);

    while (lexer.hasNext()) {
        lexer.nextToken();
    }

    // After last token, should not have more tokens
    EXPECT_FALSE(lexer.hasNext());
}

// Test case: Whitespace handling
TEST_F(LexerTest, WhitespaceHandling) {
    std::string input = "   \n   int a = 5;  ";
    auto source = createInputStream(input);
    Lexer lexer(source);

    auto token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::Keyword);
    EXPECT_EQ(token1.value, "int");
}

// Add more cases as needed to handle edge cases, large inputs, etc.

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
