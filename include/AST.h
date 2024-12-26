#pragma once

#include <iostream>
#include <vector>
#include <memory> // Needed for std::unique_ptr

// Abstract class for a generic tree node
template <typename T>
class AbstractTreeNode {
public:
    virtual ~AbstractTreeNode() = default;

    // Get the value stored in the node
    virtual T getValue() const = 0;

    // Set the value of the node
    virtual void setValue(const T& value) = 0;

    // Add a child node using a unique_ptr
    virtual void addChild(std::unique_ptr<AbstractTreeNode<T>> child) = 0;

    // Get all child nodes (stored as unique_ptr)
    virtual const std::vector<std::unique_ptr<AbstractTreeNode<T>>>& getChildren() const = 0;

    // Remove a child node by raw pointer reference
    virtual bool removeChild(AbstractTreeNode<T>* child) = 0;
};

// Abstract class for a generic tree
template <typename T>
class AbstractTree {
public:
    virtual ~AbstractTree() = default;

    // Get the root node of the tree
    virtual AbstractTreeNode<T>* getRoot() const = 0;

    // Set the root node of the tree
    virtual void setRoot(AbstractTreeNode<T>* root) = 0;

    // Traverse the tree
    virtual void traverse(std::ostream& os = std::cout) const = 0;

    // Clear the tree
    virtual void clear() = 0;
};
