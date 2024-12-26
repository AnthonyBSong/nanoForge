#include "../include/AST.h"

template <typename T>
class TreeNode : public AbstractTreeNode<T> {
public:
    TreeNode() : _value{} {}
    TreeNode(T value) : _value(value) {}
    T getValue() const override { return this._value; }
    void setValue(const T& value) { this._value = value; }
    void addChild(AbstractTreeNode<T>* child) override { _children.push_back(child); }
    const std::vector<AbstractTreeNode<T>*>& getChildren() const override { return _children; }
    bool removeChild(AbstractTreeNode<T>* child) override {
        auto it = std::find(_children.begin(), _children.end(), child);
        if (it != _children.end()) {
            _children.erase(it);
            return true;
        }
        return false;
    }
private:
    T _value;
    std::vector<TreeNode<T>> _children;
};
