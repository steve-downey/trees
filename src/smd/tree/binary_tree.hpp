// src/smd/tree/binary_tree.hpp                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE
#define INCLUDED_SMD_TREE_BINARY_TREE

#include <cassert>
#include <memory>
#include <utility>

namespace smd::tree {

template <class T>
class BinaryTree {
  T d_value;
  std::shared_ptr<BinaryTree> d_left;
  std::shared_ptr<BinaryTree> d_right;

 public:
  using value_type = T;

  static auto leaf(T value) -> BinaryTree
  {
    return BinaryTree(std::move(value), {}, {});
  }

  static auto node(T value, BinaryTree left, BinaryTree right) -> BinaryTree
  {
    return BinaryTree(std::move(value),
                      std::make_shared<BinaryTree>(std::move(left)),
                      std::make_shared<BinaryTree>(std::move(right)));
  }

  static auto branch(T value, BinaryTree left, BinaryTree right) -> BinaryTree
  {
    return node(std::move(value), std::move(left), std::move(right));
  }

  static auto from_children_ptrs(T value,
                                 std::shared_ptr<BinaryTree> left,
                                 std::shared_ptr<BinaryTree> right)
    -> BinaryTree
  {
    return BinaryTree(std::move(value), std::move(left), std::move(right));
  }

  static auto make_ptr(BinaryTree tree) -> std::shared_ptr<BinaryTree>
  {
    return std::make_shared<BinaryTree>(std::move(tree));
  }

  auto value() const -> const T& { return d_value; }

  auto has_left() const -> bool { return static_cast<bool>(d_left); }
  auto has_right() const -> bool { return static_cast<bool>(d_right); }

  auto left() const -> const BinaryTree&
  {
    assert(d_left);
    return *d_left;
  }

  auto right() const -> const BinaryTree&
  {
    assert(d_right);
    return *d_right;
  }

  auto left_ptr() const -> const std::shared_ptr<BinaryTree>& { return d_left; }
  auto right_ptr() const -> const std::shared_ptr<BinaryTree>& { return d_right; }

 private:
  BinaryTree(T value,
             std::shared_ptr<BinaryTree> left,
             std::shared_ptr<BinaryTree> right)
      : d_value(std::move(value))
      , d_left(std::move(left))
      , d_right(std::move(right))
  {
  }
};

}  // close namespace smd::tree

#endif
