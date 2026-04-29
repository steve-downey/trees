#ifndef INCLUDE_SMD_TREE_BINARY_TREE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_BINARY_TREE_TRAVERSABLE_HPP

#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct BinaryTreeTraversableImpl {
  template <class F>
  auto traverse(this auto&& self, F&& function, const smd::tree::BinaryTree<T>& tree)
  {
    auto value_context = std::invoke(std::forward<F>(function), tree.value());
    using Context = remove_cvref_t<decltype(value_context)>;
    const auto& applicative = smd::applicative_typeclass<Context>;

    auto left_context = [&]() {
      if (tree.has_left()) {
        auto left_tree_context = self.traverse(function, tree.left());
        return applicative.invoke(
          [](auto&& subtree) {
            using SubTree = remove_cvref_t<decltype(subtree)>;
            return std::make_shared<SubTree>(
              std::forward<decltype(subtree)>(subtree));
          },
          left_tree_context);
      }
      return applicative.invoke(
        [](auto&& value) {
          using U = remove_cvref_t<decltype(value)>;
          return std::shared_ptr<smd::tree::BinaryTree<U> >{};
        },
        value_context);
    }();

    auto right_context = [&]() {
      if (tree.has_right()) {
        auto right_tree_context = self.traverse(function, tree.right());
        return applicative.invoke(
          [](auto&& subtree) {
            using SubTree = remove_cvref_t<decltype(subtree)>;
            return std::make_shared<SubTree>(
              std::forward<decltype(subtree)>(subtree));
          },
          right_tree_context);
      }
      return applicative.invoke(
        [](auto&& value) {
          using U = remove_cvref_t<decltype(value)>;
          return std::shared_ptr<smd::tree::BinaryTree<U> >{};
        },
        value_context);
    }();

    return applicative.invoke(
      [](auto&& value, auto&& left, auto&& right) {
        using U = remove_cvref_t<decltype(value)>;
        return smd::tree::BinaryTree<U>::from_children_ptrs(
          std::forward<decltype(value)>(value),
          std::forward<decltype(left)>(left),
          std::forward<decltype(right)>(right));
      },
      value_context,
      left_context,
      right_context);
  }
};

template <class T>
struct BinaryTreeTraversableMap : Traversable<BinaryTreeTraversableImpl<T> > {
  using BinaryTreeTraversableImpl<T>::traverse;
};

template <class T>
inline constexpr auto traversable_typeclass<smd::tree::BinaryTree<T> > =
  BinaryTreeTraversableMap<T>{};

}  // close namespace smd

#endif
