// src/smd/tree/fringe_tree_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FRINGE_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_FRINGE_TREE_APPLICATIVE

#include <smd/tree/fringe_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <type_traits>
#include <utility>

namespace smd {

/** Applicative typeclass instance for FringeTree<T> with shape-aware semantics.
 *
 * pure(v) produces a single leaf. apply recurses pairwise: a leaf function
 * distributes over the argument's shape, a leaf argument distributes over the
 * function's shape, and two branches recurse on matching sides. Empty operands
 * yield an empty result. These are monad-derived (not zip) applicative
 * semantics; the structure mirrors the sequence monad over the fringe.
 * @tparam T element type of the function tree
 */
template <class T>
struct FringeTreeApplicativeImpl {
    /** Lift a plain value into a single-leaf tree. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using U = remove_cvref_t<VALUE>;
        return smd::tree::FringeTree<U>::leaf(std::forward<VALUE>(value));
    }

    /**
     * @brief Apply a tree of functions to a tree of arguments, shape-aware.
     * @param functions tree whose leaves contain callables
     * @param arguments tree whose leaves contain arguments
     * @return tree of results; empty if either operand is empty
     */
    template <class F, class A>
    auto apply(this auto &&self, const smd::tree::FringeTree<F> &functions,
               const smd::tree::FringeTree<A> &arguments)
        -> smd::tree::FringeTree<std::invoke_result_t<const F &, const A &>> {
        using R = std::invoke_result_t<const F &, const A &>;

        if (functions.is_empty() || arguments.is_empty()) {
            return smd::tree::FringeTree<R>::empty();
        }

        if (functions.is_leaf()) {
            auto function = functions.value();
            if (arguments.is_leaf()) {
                return smd::tree::FringeTree<R>::leaf(
                    function(arguments.value()));
            }
            return smd::tree::FringeTree<R>::branch(
                self.apply(functions, arguments.left()),
                self.apply(functions, arguments.right()));
        }

        if (arguments.is_leaf()) {
            return smd::tree::FringeTree<R>::branch(
                self.apply(functions.left(), arguments),
                self.apply(functions.right(), arguments));
        }

        return smd::tree::FringeTree<R>::branch(
            self.apply(functions.left(), arguments.left()),
            self.apply(functions.right(), arguments.right()));
    }
};

/** Applicative map exposing pure and apply for FringeTree<T>. */
template <class T>
struct FringeTreeApplicativeMap : Applicative<FringeTreeApplicativeImpl<T>> {
    using FringeTreeApplicativeImpl<T>::apply;
    using FringeTreeApplicativeImpl<T>::pure;
};

/** Registers FringeTreeApplicativeMap as the Applicative instance for
 * FringeTree<T>. */
template <class T>
inline constexpr auto applicative_typeclass<smd::tree::FringeTree<T>> =
    FringeTreeApplicativeMap<T>{};

} // namespace smd

#endif
