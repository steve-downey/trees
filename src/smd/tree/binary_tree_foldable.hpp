// src/smd/tree/binary_tree_foldable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_BINARY_TREE_FOLDABLE

#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

template <class T>
struct BinaryTreeFoldableImpl {
  template <class F>
  auto fold_map(this auto&& self,
                F&& function,
                const smd::tree::BinaryTree<T>& tree)
    -> remove_cvref_t<decltype(std::invoke(function, tree.value()))>
  {
    auto value_result = std::invoke(function, tree.value());
    using Result = remove_cvref_t<decltype(value_result)>;

    Result acc = tree.has_left()
      ? smd::typeclass::monoid_v<Result>.combine(
          self.fold_map(function, tree.left()),
          std::move(value_result))
      : std::move(value_result);

    if (tree.has_right()) {
      acc = smd::typeclass::monoid_v<Result>.combine(
        std::move(acc),
        self.fold_map(function, tree.right()));
    }

    return acc;
  }
};

template <class T>
struct BinaryTreeFoldableMap : Foldable<BinaryTreeFoldableImpl<T> > {
  using BinaryTreeFoldableImpl<T>::fold_map;
};

template <class T>
inline constexpr auto foldable_typeclass<smd::tree::BinaryTree<T> > =
  BinaryTreeFoldableMap<T>{};

}  // close namespace smd

#endif
