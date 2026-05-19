// src/smd/tree/binary_tree_applicative.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_APPLICATIVE
#define INCLUDED_SMD_TREE_BINARY_TREE_APPLICATIVE

#include <smd/tree/binary_tree.hpp>
#include <smd/typeclass/applicative.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace smd {

/** Applicative typeclass instance for BinaryTree<T> with shape-aware semantics.
 *
 * pure(v) produces a single leaf. apply recurses pairwise over matching tree
 * structure: a leaf function distributes over the argument's shape; a leaf
 * argument distributes over the function's shape; when both have children,
 * only positions where both trees have a child are combined (pairwise).
 * These are monad-derived (not zip) applicative semantics.
 * @tparam T element type of the function tree (F is the function type stored)
 */
template <class T>
struct BinaryTreeApplicativeImpl {
    /** Lift a plain value into a single-leaf tree. */
    template <class VALUE>
    auto pure(this auto &&, VALUE &&value) {
        using U = remove_cvref_t<VALUE>;
        return smd::tree::BinaryTree<U>::leaf(std::forward<VALUE>(value));
    }

    /**
     * @brief Apply a tree of functions to a tree of arguments, shape-aware.
     * @param functions tree whose nodes contain callables
     * @param arguments tree whose nodes contain arguments
     * @return tree of results; shape determined by pairwise recursion rules
     */
    template <class F, class A>
    auto apply(this auto &&self, const smd::tree::BinaryTree<F> &functions,
               const smd::tree::BinaryTree<A> &arguments)
        -> smd::tree::BinaryTree<std::invoke_result_t<const F &, const A &>> {
        using R = std::invoke_result_t<const F &, const A &>;

        std::shared_ptr<smd::tree::BinaryTree<R>> left{};
        std::shared_ptr<smd::tree::BinaryTree<R>> right{};

        const auto function_is_leaf =
            !functions.has_left() && !functions.has_right();
        const auto argument_is_leaf =
            !arguments.has_left() && !arguments.has_right();

        if (function_is_leaf) {
            // pure(f) should distribute f over the argument shape.
            if (arguments.has_left()) {
                left = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions, arguments.left()));
            }
            if (arguments.has_right()) {
                right = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions, arguments.right()));
            }
        } else if (argument_is_leaf) {
            // A non-leaf function tree can be applied pointwise to a single
            // argument.
            if (functions.has_left()) {
                left = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.left(), arguments));
            }
            if (functions.has_right()) {
                right = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.right(), arguments));
            }
        } else {
            // If both have shape, recurse pairwise where both children exist.
            if (functions.has_left() && arguments.has_left()) {
                left = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.left(), arguments.left()));
            }

            if (functions.has_right() && arguments.has_right()) {
                right = smd::tree::BinaryTree<R>::make_ptr(
                    self.apply(functions.right(), arguments.right()));
            }
        }

        return smd::tree::BinaryTree<R>::from_children_ptrs(
            functions.value()(arguments.value()), std::move(left),
            std::move(right));
    }
};

/** Applicative map exposing pure and apply for BinaryTree<T>. */
template <class T>
struct BinaryTreeApplicativeMap : Applicative<BinaryTreeApplicativeImpl<T>> {
    using BinaryTreeApplicativeImpl<T>::apply;
    using BinaryTreeApplicativeImpl<T>::pure;
};

/** Registers BinaryTreeApplicativeMap as the Applicative instance for
 * BinaryTree<T>. */
template <class T>
inline constexpr auto applicative_typeclass<smd::tree::BinaryTree<T>> =
    BinaryTreeApplicativeMap<T>{};

} // namespace smd

#endif
