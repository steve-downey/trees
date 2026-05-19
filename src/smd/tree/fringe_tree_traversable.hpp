// src/smd/tree/fringe_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_FRINGE_TREE_TRAVERSABLE

#include <smd/tree/fringe_tree.hpp>
#include <smd/tree/fringe_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace smd {

/** Traversable typeclass instance for FringeTree<T>.
 * traverse maps each leaf value into an applicative context and rebuilds a
 * FringeTree inside that context, preserving the original tree's structure.
 * Empty stays empty; leaves become single-element trees; branches combine
 * the traversed subtrees with FringeTree::branch inside the applicative.
 * @tparam T leaf element type of the tree being traversed
 */
template <class T>
struct FringeTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative,
                  F &&function, const smd::tree::FringeTree<T> &tree) {
        using Context = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        using U = smd::applicative_value_t<Context>;

        if (tree.is_empty()) {
            return applicative.pure(smd::tree::FringeTree<U>::empty());
        }

        if (tree.is_leaf()) {
            return applicative.invoke(
                [](auto &&value) {
                    using U = remove_cvref_t<decltype(value)>;
                    return smd::tree::FringeTree<U>::leaf(
                        std::forward<decltype(value)>(value));
                },
                std::invoke(std::forward<F>(function), tree.value()));
        }

        auto left = self.traverse(applicative, function, tree.left());
        auto right = self.traverse(applicative, function, tree.right());

        return applicative.invoke(
            [](auto &&l, auto &&r) {
                return smd::tree::
                    FringeTree<remove_cvref_t<decltype(l.value())>>::branch(
                        std::forward<decltype(l)>(l),
                        std::forward<decltype(r)>(r));
            },
            left, right);
    }
};

/** Traversable map that exposes traverse for FringeTree<T>. */
template <class T>
struct FringeTreeTraversableMap : Traversable<FringeTreeTraversableImpl<T>> {
    using FringeTreeTraversableImpl<T>::traverse;
};

/** Registers FringeTreeTraversableMap as the Traversable instance for
 * FringeTree<T>. */
template <class T>
inline constexpr auto traversable_typeclass<smd::tree::FringeTree<T>> =
    FringeTreeTraversableMap<T>{};

} // namespace smd

#endif
