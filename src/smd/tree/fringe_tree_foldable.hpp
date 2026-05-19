// src/smd/tree/fringe_tree_foldable.hpp                              -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_FOLDABLE
#define INCLUDED_SMD_TREE_FRINGE_TREE_FOLDABLE

#include <smd/tree/fringe_tree.hpp>
#include <smd/typeclass/foldable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Foldable typeclass instance for FringeTree<T>.
 * fold_map maps @p function over leaf values and combines results with the
 * Monoid identity/combine. Empty trees yield the monoid identity; branches
 * combine left and right recursively.
 * @tparam T leaf element type
 */
template <class T>
struct FringeTreeFoldableImpl {
    template <class F>
    auto fold_map(this auto &&self, F &&function,
                  const smd::tree::FringeTree<T> &tree)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;

        if (tree.is_empty()) {
            return smd::typeclass::monoid_v<Result>.identity();
        }

        if (tree.is_leaf()) {
            return std::invoke(function, tree.value());
        }

        auto left = self.fold_map(function, tree.left());
        auto right = self.fold_map(function, tree.right());
        return smd::typeclass::monoid_v<Result>.combine(std::move(left),
                                                        std::move(right));
    }
};

/** Foldable map that exposes fold_map for FringeTree<T>. */
template <class T>
struct FringeTreeFoldableMap : Foldable<FringeTreeFoldableImpl<T>> {
    using FringeTreeFoldableImpl<T>::fold_map;
};

/** Registers FringeTreeFoldableMap as the Foldable instance for FringeTree<T>.
 */
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FringeTree<T>> =
    FringeTreeFoldableMap<T>{};

} // namespace smd

#endif
