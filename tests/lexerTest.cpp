#include "../include/Lexer.hpp"  // Adjust the path if needed
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
    //std::deque<Token> parsedTokens;
    Lexer lexer(source);

    // Assuming Lexer has a method `nextToken()` to fetch tokens
    auto token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::INSTRUCTION);
    EXPECT_EQ(token.lexeme, "add");

    // Additional checks for other tokens
}

TEST_F(LexerTest, RegisterTokenization) {
    for (int i = 0; i<= 50; ++i) {
        std::string input = "x" + std::to_string(i);
        autosource = createInputStream(input);
        Lexer lexer(source, parsedTokens, instructions);

        auto token = lexer.getNextToken();

        if (i <= 31) {
            //Regsiters x0 to x31 should be valid
            EXPECT_EQ(token.type, TokenType::REGISTER);
            EXPECT_EQ(token.lexeme, "x" + std::tostring(i));
            
        } else {
            //Registers x32 to x50 are invalid, and should result in null
            EXPECT_EQ(token.type, nullptr); // not necessarily nullptr
            EXPECT_EQ(token.lexeme, "x" + std::tostring(i));
        }
    }
}

// Struct to hold the test data for immediate values
struct ImmediateTestCase {
    std::string input;
    std::string expectedLexeme;
};

// Test case: Parameterized test for different types of immediate values (binary, hexadecimal, decimal)
class ImmediateTokenizationTest : public LexerTest, public ::testing::WithParamInterface<ImmediateTestCase> {
};

// Instantiating the test suite with multiple test cases (binary, hexadecimal, and decimal)
INSTANTIATE_TEST_SUITE_P(
    ImmediateTokenizationTests,
    ImmediateTokenizationTest,
    ::testing::Values(
        ImmediateTestCase{"0b101010", "0b101010"},  // Binary immediate
        ImmediateTestCase{"0x1A3F", "0x1A3F"},    // Hexadecimal immediate
        ImmediateTestCase{"12345", "12345"}       // Decimal immediate
    )
);

// Combined test for all types of immediate tokenization
TEST_P(ImmediateTokenizationTest, ImmediateTokenization) {
    const ImmediateTestCase& testCase = GetParam();
    auto source = createInputStream(testCase.input);
    std::deque<Token> parsedTokens;
    Lexer lexer(source, parsedTokens, instructions);

    auto token = lexer.getNextToken();
    EXPECT_EQ(token.type, TokenType::IMMEDIATE);
    EXPECT_EQ(token.lexeme, testCase.expectedLexeme);
}

// Test case: Label tokenization
TEST_F(LexerTest, LabelTokenization) {
    std::string input = "Label:";
    auto source = createInputStream(input);
    //std::deque<Token> parsedTokens;
    Lexer lexer(source);

    // Assuming Lexer has a method `nextToken()` to fetch tokens
    auto token = lexer.nextToken();
    EXPECT_EQ(token.type, TokenType::LABEL);
    EXPECT_EQ(token.lexeme, "Label:");

    // Additional checks for other tokens
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
