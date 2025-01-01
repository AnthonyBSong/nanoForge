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
        return validNode(root);
    }

    private:
    //Function to validate nodes
    bool validNode(AbstractTreeNode<std::string>* node) const {
        //Base Case: If node is nullptr, return true
        return;
    }
}