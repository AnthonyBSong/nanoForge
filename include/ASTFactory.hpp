#pragma once

#include "AST.h"         // Abstract interfaces
#include "TreeNode.hpp"  // Concrete TreeNode
#include "SyntaxTree.hpp"// Concrete SyntaxTree

template <typename T>
class ASTFactory {
public:
    static AbstractTreeNode<T>* createNode(const T& value) {
        // Return a new TreeNode with 'value'
        return new TreeNode<T>(value);
    }

    static AbstractTree<T>* createTree() {
        // Return a new SyntaxTree
        return new SyntaxTree<T>();
    }
};
