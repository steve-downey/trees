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

/** Foldable typeclass instance for BinaryTree<T>.
 * fold_map applies @p function to every node value (in-order: left, root,
 * right) and combines the results using the Monoid for the return type.
 * @tparam T element type of the tree being folded
 */
template <class T>
struct BinaryTreeFoldableImpl {
    template <class F>
    auto fold_map(this auto &&self, F &&function,
                  const smd::tree::BinaryTree<T> &tree)
        -> remove_cvref_t<decltype(std::invoke(function, tree.value()))> {
        auto value_result = std::invoke(function, tree.value());
        using Result = remove_cvref_t<decltype(value_result)>;

        Result acc = tree.has_left() ? smd::typeclass::monoid_v<Result>.combine(
                                           self.fold_map(function, tree.left()),
                                           std::move(value_result))
                                     : std::move(value_result);

        if (tree.has_right()) {
            acc = smd::typeclass::monoid_v<Result>.combine(
                std::move(acc), self.fold_map(function, tree.right()));
        }

        return acc;
    }
};

/** Foldable map that exposes the fold_map operation for BinaryTree<T>. */
template <class T>
struct BinaryTreeFoldableMap : Foldable<BinaryTreeFoldableImpl<T>> {
    using BinaryTreeFoldableImpl<T>::fold_map;
};

/** Registers BinaryTreeFoldableMap as the Foldable instance for BinaryTree<T>. */
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::BinaryTree<T>> =
    BinaryTreeFoldableMap<T>{};

} // namespace smd

#endif
