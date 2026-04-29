#ifndef INCLUDE_SMD_TREE_BINARY_TREE_TRAVERSABLE_HPP
#define INCLUDE_SMD_TREE_BINARY_TREE_TRAVERSABLE_HPP

#include <smd/tree/binary_tree_applicative.hpp>
#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct BinaryTreeTraversableImpl {
  template <class F>
  auto traverse(this auto&& self, F&& function, const smd::tree::BinaryTree<T>& tree)
    -> decltype(smd::applicative_typeclass<remove_cvref_t<std::invoke_result_t<F, const T&> >>.invoke(
      [](auto&& value) {
        using U = remove_cvref_t<decltype(value)>;
        return smd::tree::BinaryTree<U>::leaf(std::forward<decltype(value)>(value));
      },
      std::invoke(std::forward<F>(function), tree.value())))
  {
    auto value_context = std::invoke(std::forward<F>(function), tree.value());
    using Context = remove_cvref_t<decltype(value_context)>;
    const auto& applicative = smd::applicative_typeclass<Context>;
    using U = smd::applicative_value_t<Context>;
    using TreeContext = decltype(applicative.invoke(
      [](auto&& value) {
        using V = remove_cvref_t<decltype(value)>;
        return smd::tree::BinaryTree<V>::leaf(std::forward<decltype(value)>(value));
      },
      value_context));

    if (!tree.has_left() && !tree.has_right()) {
      return applicative.invoke(
        [](auto&& value) {
          using V = remove_cvref_t<decltype(value)>;
          return smd::tree::BinaryTree<V>::leaf(std::forward<decltype(value)>(value));
        },
        value_context);
    }

    std::optional<TreeContext> left_tree_context;
    if (tree.has_left()) {
      left_tree_context.emplace(self.traverse(function, tree.left()));
    }

    std::optional<TreeContext> right_tree_context;
    if (tree.has_right()) {
      right_tree_context.emplace(self.traverse(function, tree.right()));
    }

    auto to_child_ptr = [&](const auto& child_tree_context) {
      return applicative.invoke(
        [](auto&& subtree) {
          using SubTree = remove_cvref_t<decltype(subtree)>;
          return std::make_shared<SubTree>(
            std::forward<decltype(subtree)>(subtree));
        },
        child_tree_context);
    };

    auto empty_child_like = [&](const auto& child_tree_context) {
      return applicative.invoke(
        [](const auto&) {
          return std::shared_ptr<smd::tree::BinaryTree<U> >{};
        },
        child_tree_context);
    };

    auto left_context = [&]() {
      if (left_tree_context.has_value()) {
        return to_child_ptr(*left_tree_context);
      }

      return empty_child_like(*right_tree_context);
    }();

    auto right_context = [&]() {
      if (right_tree_context.has_value()) {
        return to_child_ptr(*right_tree_context);
      }

      return empty_child_like(*left_tree_context);
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
