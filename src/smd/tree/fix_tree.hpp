#ifndef INCLUDE_SMD_TREE_FIX_TREE_HPP
#define INCLUDE_SMD_TREE_FIX_TREE_HPP

#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>

namespace smd::tree {

template <class VALUE_TYPE>
class FixTree {
  public:
    using value_type = VALUE_TYPE;

  private:
    struct Node {
        std::shared_ptr<const FixTree> d_left;
        std::shared_ptr<const FixTree> d_right;
    };

    std::variant<VALUE_TYPE, Node> d_data;

    explicit FixTree(VALUE_TYPE value);
    FixTree(FixTree left, FixTree right);

  public:
    static auto leaf(VALUE_TYPE value) -> FixTree;
    static auto branch(FixTree left, FixTree right) -> FixTree;

    auto is_leaf() const -> bool;
    auto is_branch() const -> bool;
    auto value() const -> const VALUE_TYPE&;
    auto left() const -> const FixTree&;
    auto right() const -> const FixTree&;

    friend auto operator==(const FixTree& lhs, const FixTree& rhs) -> bool
    {
        if (lhs.is_leaf() != rhs.is_leaf()) {
            return false;
        }
        if (lhs.is_leaf()) {
            return lhs.value() == rhs.value();
        }
        return lhs.left() == rhs.left() && lhs.right() == rhs.right();
    }
};

template <class VALUE_TYPE>
FixTree<VALUE_TYPE>::FixTree(VALUE_TYPE value)
: d_data(std::move(value))
{
}

template <class VALUE_TYPE>
FixTree<VALUE_TYPE>::FixTree(FixTree left, FixTree right)
: d_data(Node{std::make_shared<FixTree>(std::move(left)),
              std::make_shared<FixTree>(std::move(right))})
{
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::leaf(VALUE_TYPE value) -> FixTree
{
    return FixTree(std::move(value));
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::branch(FixTree left, FixTree right) -> FixTree
{
    return FixTree(std::move(left), std::move(right));
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::is_leaf() const -> bool
{
    return std::holds_alternative<VALUE_TYPE>(d_data);
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::is_branch() const -> bool
{
    return !is_leaf();
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::value() const -> const VALUE_TYPE&
{
    if (!is_leaf()) {
        throw std::logic_error("FixTree::value called for a branch");
    }
    return std::get<VALUE_TYPE>(d_data);
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::left() const -> const FixTree&
{
    if (!is_branch()) {
        throw std::logic_error("FixTree::left called for a leaf");
    }
    return *std::get<Node>(d_data).d_left;
}

template <class VALUE_TYPE>
auto FixTree<VALUE_TYPE>::right() const -> const FixTree&
{
    if (!is_branch()) {
        throw std::logic_error("FixTree::right called for a leaf");
    }
    return *std::get<Node>(d_data).d_right;
}

}  // close namespace smd::tree

#endif  // INCLUDE_SMD_TREE_FIX_TREE_HPP
