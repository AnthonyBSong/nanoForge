#pragma once

#include "AST.h"
#include <memory>
#include <queue>
#include <iostream>

template <typename T>
class SyntaxTree : public AbstractTree<T> {
public:
    // Constructors
    SyntaxTree() : _root(nullptr) {}
    explicit SyntaxTree(AbstractTreeNode<T>* root) : _root(root) {}

    // Implementations of AbstractTree<T> pure virtual methods
    AbstractTreeNode<T>* getRoot() const override {
        return _root;
    }

    void setRoot(AbstractTreeNode<T>* root) override {
        _root = root;
    }

    // Preorder traversal (default for 'traverse')
    void traverse(std::ostream& os = std::cout) const override {
        preorderTraversal(_root, os);
        os << std::endl;
    }

    // Recursively delete all nodes
    void clear() override {
        clearNode(_root);
        _root = nullptr;
    }

private:
    // ------------------------
    // Pre-Order Traversal
    // ------------------------
    void preorderTraversal(AbstractTreeNode<T>* node, std::ostream& os) const {
        if (!node) return;
        // Visit the current node
        os << node->getValue() << " ";
        // Recursively traverse children
        for (auto& child : node->getChildren()) {
            preorderTraversal(child.get(), os);
        }
    }

    // ------------------------
    // In-Order Traversal
    // ------------------------
    void inorderTraversal(AbstractTreeNode<T>* node, std::ostream& os) const {
        if (!node) return;
        const auto& children = node->getChildren();

        // Process the first child as "left subtree"
        if (!children.empty()) {
            inorderTraversal(children[0].get(), os);
        }

        // Visit the current node
        os << node->getValue() << " ";

        // Process the rest of the children as "right subtree"
        for (size_t i = 1; i < children.size(); ++i) {
            inorderTraversal(children[i].get(), os);
        }
    }

    // ------------------------
    // Post-Order Traversal
    // ------------------------
    void postorderTraversal(AbstractTreeNode<T>* node, std::ostream& os) const {
        if (!node) return;
        // Recursively traverse children first
        for (auto& child : node->getChildren()) {
            postorderTraversal(child.get(), os);
        }
        // Visit the current node
        os << node->getValue() << " ";
    }

    // ------------------------
    // Level-Order Traversal
    // ------------------------
    void levelOrderTraversal(AbstractTreeNode<T>* node, std::ostream& os) const {
        if (!node) return;
        std::queue<AbstractTreeNode<T>*> bfsQueue;
        bfsQueue.push(node);

        while (!bfsQueue.empty()) {
            auto* current = bfsQueue.front();
            bfsQueue.pop();
            // Visit the current node
            os << current->getValue() << " ";
            for (auto& child : current->getChildren()) {
                bfsQueue.push(child.get());
            }
        }
    }

    // ------------------------
    // Utility to free all nodes in a subtree
    // ------------------------
    void clearNode(AbstractTreeNode<T>* node) {
        if (!node) return;

        // Recursively clear children first
        for (auto& child : node->getChildren()) {
            clearNode(child.get());
        }
        // Then delete this node
        delete node;
    }

    AbstractTreeNode<T>* _root;
};
