// src/smd/tree/binary_tree_traversable.hpp                           -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_BINARY_TREE_TRAVERSABLE
#define INCLUDED_SMD_TREE_BINARY_TREE_TRAVERSABLE

#include <smd/tree/binary_tree.hpp>
#include <smd/tree/binary_tree_applicative.hpp>
#include <smd/typeclass/traversable.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace smd {

/** Traversable typeclass instance for BinaryTree<T>.
 * traverse maps each node value into an applicative context and rebuilds a
 * BinaryTree inside that context, preserving the original tree's shape.
 * @tparam T element type of the tree being traversed
 */
template <class T>
struct BinaryTreeTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&self, const APPLICATIVE &applicative,
                  F &&function, const smd::tree::BinaryTree<T> &tree) {
        auto value_context =
            std::invoke(std::forward<F>(function), tree.value());
        using Context = remove_cvref_t<decltype(value_context)>;
        using U = smd::applicative_value_t<Context>;
        using TreeContext = decltype(applicative.invoke(
            [](auto &&value) {
                using V = remove_cvref_t<decltype(value)>;
                return smd::tree::BinaryTree<V>::leaf(
                    std::forward<decltype(value)>(value));
            },
            value_context));

        if (!tree.has_left() && !tree.has_right()) {
            return applicative.invoke(
                [](auto &&value) {
                    using V = remove_cvref_t<decltype(value)>;
                    return smd::tree::BinaryTree<V>::leaf(
                        std::forward<decltype(value)>(value));
                },
                value_context);
        }

        std::optional<TreeContext> left_tree_context;
        if (tree.has_left()) {
            left_tree_context.emplace(
                self.traverse(applicative, function, tree.left()));
        }

        std::optional<TreeContext> right_tree_context;
        if (tree.has_right()) {
            right_tree_context.emplace(
                self.traverse(applicative, function, tree.right()));
        }

        auto to_child_ptr = [&](const auto &child_tree_context) {
            return applicative.invoke(
                [](auto &&subtree) {
                    using SubTree = remove_cvref_t<decltype(subtree)>;
                    return std::make_shared<SubTree>(
                        std::forward<decltype(subtree)>(subtree));
                },
                child_tree_context);
        };

        auto empty_child_like = [&](const auto &child_tree_context) {
            return applicative.invoke(
                [](const auto &) {
                    return std::shared_ptr<smd::tree::BinaryTree<U>>{};
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
            [](auto &&value, auto &&left, auto &&right) {
                using U = remove_cvref_t<decltype(value)>;
                return smd::tree::BinaryTree<U>::from_children_ptrs(
                    std::forward<decltype(value)>(value),
                    std::forward<decltype(left)>(left),
                    std::forward<decltype(right)>(right));
            },
            value_context, left_context, right_context);
    }
};

/** Traversable map that exposes traverse for BinaryTree<T>. */
template <class T>
struct BinaryTreeTraversableMap : Traversable<BinaryTreeTraversableImpl<T>> {
    using BinaryTreeTraversableImpl<T>::traverse;
};

/** Registers BinaryTreeTraversableMap as the Traversable instance for
 * BinaryTree<T>. */
template <class T>
inline constexpr auto traversable_typeclass<smd::tree::BinaryTree<T>> =
    BinaryTreeTraversableMap<T>{};

} // namespace smd

#endif
