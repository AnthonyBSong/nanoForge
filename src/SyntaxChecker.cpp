#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "AST.h"
#include "TreeNode.hpp"
#include "SyntaxTree.hpp" 

//SyntaxChecker class
class SyntaxChecker {
    public:
    //Constructor
    SyntaxChecker() = default;

    //Check for tree being syntactically correct
    bool checkSyntax(const AbstractTree<std::string>& tree) const {
        //Check root
        AbstractTreeNode<std::string>* root = tree.getRoot();
        if (!root) {
            std::cerr << "Warning! The tree is empty (no root node).\n";
            return false;
        }
        return checkNode(root);
    }

    private:
    //Function to validate nodes
    bool checkNode(AbstractTreeNode<std::string>* node) const {
        //Base Case: If node is nullptr, return true
        if (!node) {
            return true;
        }

        //Get value of current node
        std::string value = node->getValue();

        //Check Node
        if (!isValidNode(value)) {
            std::cerr << "Syntax Error: Invalid node value '" << value << "'.\n";
            return false;
        }

        //Check node children
        const auto& children = node->getChildren();
        if (!checkChildren(value,children)) {
            return false;
        }

        //Iterate through all children
        for (auto& child :children) {
            if (!checkNode(child.get())) {
                return false;
            }
        }
        return true;
    }

    //Helper function to check Node validity
    bool isValidNode(const std::string& value) const {
        return;
    }

    //Check number of children nodes
    bool checkChildren(const std::string& value, const std::vector<std::unique_ptr<AbstractTreeNode<std::string>>>& children) const {
        return;
    }
};

//Main
int main() {
    //Example: create a tree
    SyntaxTree<std::string> tree;
    
    auto root = new TreeNode<std::string>("");
    auto child1 = new TreeNode<std::string>("");
    auto child2 = new TreeNode<std::string>("");

    root->addChild(std::unique_ptr<AbstractTreeNode<std::string>>(child1));
    root->addChild(std::unique_ptr<AbstractTreeNode<std::string>>(child2));
    tree.setRoot(root);

    //Create SyntaxChecker and check
    SyntaxChecker check;
    if (check.checkSyntax(tree)) {
        std::cout<<"The tree is valid.\n";
    }
    else {
        std::cerr <<"This tree is invalid due to syntax errors.\n";
    }

    return 0;
}