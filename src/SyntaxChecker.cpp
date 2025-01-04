#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "AST.h"
#include "TreeNode.hpp"
#include "SyntaxTree.hpp"
#include "Reader.cpp"
#include "Token.hpp" // Include the Token header

// SyntaxChecker class
class SyntaxChecker {
private:
    const std::unordered_map<std::string, std::vector<std::string>>& paramMap;

public:
    // Constructor with paramMap
    SyntaxChecker(const std::unordered_map<std::string, std::vector<std::string>>& paramMap) : paramMap(paramMap) {}

    bool checkInstruction(const std::string& instr) {
        // Check if instruction exists in the paramMap
        if (paramMap.find(instr) != paramMap.end()) {
            return true;
        }
        return false;
    }

    // Check if the tree is syntactically correct
    bool checkSyntax(const AbstractTree<Token>& tree) const {
        // Check root
        AbstractTreeNode<Token>* root = tree.getRoot();
        if (!root) {
            std::cerr << "Warning! The tree is empty (no root node).\n";
            return false;
        }
        return checkNode(root);
    }

private:
    // Function to validate nodes
    bool checkNode(AbstractTreeNode<Token>* node) const {
        // Base Case: If node is nullptr, return true
        if (!node) {
            return true;
        }

        // Get value of current node
        Token value = node->getValue();

        // Check Node
        if (!isValidNode(value)) {
            std::cerr << "Syntax Error: Invalid node value '" << value.lexeme << "'.\n";
            return false;
        }

        // Check node children
        const auto& children = node->getChildren();
        if (!checkChildren(value, children)) {
            return false;
        }

        // Iterate through all children
        for (auto& child : children) {
            if (!checkNode(child.get())) {
                return false;
            }
        }
        return true;
    }

    // Helper function to check Node validity
    bool isValidNode(const Token& value) const {
        return paramMap.find(value.lexeme) != paramMap.end();
    }

    // Check number of children nodes
    bool checkChildren(const Token& value, const std::vector<std::unique_ptr<AbstractTreeNode<Token>>>& children) const {
        const auto& expectedParams = paramMap.at(value.lexeme); // Get expected parameters
        if (children.size() != expectedParams.size()) {
            std::cerr << "Syntax Error: Node '" << value.lexeme << "' expects "
                      << expectedParams.size() << " parameters, but has "
                      << children.size() << ".\n";
            return false;
        }
        return true;
    }
};

// Main
int main() {
    // Maps we want to fill
    std::unordered_map<std::string, std::vector<std::string>> paramMap;
    std::unordered_map<std::string, std::vector<BitField>> binaryMap;

    // Parse the instruction file to populate paramMap and binaryMap
    if (!parseInstructionFile("instructions.txt", paramMap, binaryMap)) {
        std::cerr << "Failed to parse instructions file.\n";
        return 1;
    }

    // Create SyntaxChecker with paramMap
    SyntaxChecker check(paramMap);

    // Example: create a tree
    SyntaxTree<Token> tree;
    
    //Sample checks
    auto root = new TreeNode<Token>({TokenType::INSTRUCTION, "load", 1, 1});
    auto child1 = new TreeNode<Token>({TokenType::REGISTER, "register", 1, 2});
    auto child2 = new TreeNode<Token>({TokenType::IMMEDIATE, "immediate", 1, 3});

    root->addChild(std::unique_ptr<AbstractTreeNode<Token>>(child1));
    root->addChild(std::unique_ptr<AbstractTreeNode<Token>>(child2));
    tree.setRoot(root);

    // Create SyntaxChecker and check
    if (check.checkSyntax(tree)) {
        std::cout << "The tree is valid.\n";
    } else {
        std::cerr << "This tree is invalid due to syntax errors.\n";
    }

    return 0;
}