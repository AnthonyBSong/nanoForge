#pragma once

#include "AST.h"
#include <memory>
#include <algorithm>
#include <vector>

template <typename T>
class TreeNode : public AbstractTreeNode<T> {
public:
    // Constructors
    TreeNode() : _value{} {}
    explicit TreeNode(const T& value) : _value(value) {}

    // Implementations of AbstractTreeNode<T> pure virtual methods
    T getValue() const override {
        return _value;
    }

    void setValue(const T& value) override {
        _value = value;
    }

    void addChild(std::unique_ptr<AbstractTreeNode<T>> child) override {
        _children.emplace_back(std::move(child));
    }

    const std::vector<std::unique_ptr<AbstractTreeNode<T>>>& getChildren() const override {
        return _children;
    }

    bool removeChild(AbstractTreeNode<T>* child) override {
        // Find the child whose pointer matches 'child'
        auto it = std::find_if(_children.begin(), _children.end(),
            [&](const std::unique_ptr<AbstractTreeNode<T>>& ptr) {
                return ptr.get() == child;
            }
        );

        if (it != _children.end()) {
            _children.erase(it);
            return true;
        }
        return false;
    }

private:
    T _value;
    // Store children as unique_ptr to manage ownership
    std::vector<std::unique_ptr<AbstractTreeNode<T>>> _children;
};
