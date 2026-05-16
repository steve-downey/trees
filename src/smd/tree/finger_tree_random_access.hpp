// src/smd/tree/finger_tree_random_access.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS
#define INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS

// Persistent random-access sequence backed by a finger tree.
//
// The Tree template parameter accepts any fully-specialised finger tree type
// whose measure is an element count (size_t, UnitMeasure):
//
//   FingerTreeRandomAccess<int>                  // FT5-backed (default, correct at all sizes)
//   FingerTreeRandomAccess<int, FingerTree2<int>> // FT2-backed (kMaxDepth hazard at N > ~2000)
//
// This mirrors std::stack<T, Container>: the element type T plus an optional
// fully-specialised backing tree.  Reason for the full-specialisation approach
// (not template-template): different finger tree variants have different
// template signatures; taking the complete instantiation is robust to drift.

#include <smd/tree/finger_tree5.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

/** @brief Persistent random-access sequence backed by a finger tree.
 *
 * @tparam T    Element type.
 * @tparam Tree Backing finger tree type (must use a unit-count measure).
 *              Default: FingerTree5<T> — correct at all sizes.
 *
 * Complexity:
 * - at / update / insert / erase: O(log n)
 * - push_front / push_back:       O(1) amortized
 * - size / empty:                 O(1)
 * - to_vector:                    O(n)
 */
template <typename T,
          typename Tree = FingerTree5<T, std::size_t, UnitMeasure5<T, std::size_t>>>
class FingerTreeRandomAccess {
    Tree d_tree;

  public:
    /** Constructs an empty sequence. */
    FingerTreeRandomAccess() : d_tree(Tree::empty()) {}

    /** Constructs from an existing backing tree. */
    explicit FingerTreeRandomAccess(Tree tree) : d_tree(std::move(tree)) {}

    /** Builds a sequence from a vector in order; O(n). */
    static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(Tree::from_sequence(std::move(values)));
    }

    /** Returns the number of elements; O(1). */
    auto size() const -> std::size_t { return d_tree.measure(); }

    /** Returns true if the sequence is empty; O(1). */
    auto empty() const -> bool { return d_tree.is_empty(); }

    /** Returns the element at @p index, or nullopt if out of range; O(log n). */
    auto at(std::size_t index) const -> std::optional<T> {
        if (index >= size())
            return std::nullopt;
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value())
            return std::nullopt;
        return sp->d_pivot;
    }

    /** Returns a new sequence with @p value appended at the back; O(1) amortized. */
    auto push_back(T value) const -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(d_tree.snoc(std::move(value)));
    }

    /** Returns a new sequence with @p value prepended at the front; O(1) amortized. */
    auto push_front(T value) const -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(d_tree.cons(std::move(value)));
    }

    /** Returns a new sequence with @p value inserted before @p index; O(log n). */
    auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess {
        auto parts = d_tree.split_at(
            [index](std::size_t prefix) { return prefix > index; });
        return FingerTreeRandomAccess(Tree::concat(
            Tree::concat(parts.d_left, Tree::leaf(std::move(value))),
            parts.d_right));
    }

    /** Returns a new sequence with the element at @p index removed; O(log n).
     *  Returns @c *this unchanged if @p index is out of range.
     */
    auto erase(std::size_t index) const -> FingerTreeRandomAccess {
        if (index >= size())
            return *this;
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value())
            return *this;
        return FingerTreeRandomAccess(Tree::concat(sp->d_left, sp->d_right));
    }

    /** Returns a new sequence with position @p index replaced by @p value; O(log n).
     *  Returns @c *this unchanged if @p index is out of range.
     */
    auto update(std::size_t index, T value) const -> FingerTreeRandomAccess {
        if (index >= size())
            return *this;
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value())
            return *this;
        return FingerTreeRandomAccess(
            Tree::concat(sp->d_left.snoc(std::move(value)), sp->d_right));
    }

    /** Materialises all elements into a vector in sequence order; O(n). */
    auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

} // namespace smd::tree

namespace smd {

/** Foldable typeclass implementation for FingerTreeRandomAccess<T, Tree>. */
template <class T, class Tree>
struct FingerTreeRandomAccessFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeRandomAccess<T, Tree> &sequence)
        -> remove_cvref_t<std::invoke_result_t<F, const T &>> {
        using Result = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        return std::ranges::fold_left(
            sequence.to_vector(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &value) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, value));
            });
    }
};

template <class T, class Tree>
struct FingerTreeRandomAccessFoldableMap
    : Foldable<FingerTreeRandomAccessFoldableImpl<T, Tree>> {
    using FingerTreeRandomAccessFoldableImpl<T, Tree>::fold_map;
};

template <class T, class Tree>
inline constexpr auto
    foldable_typeclass<smd::tree::FingerTreeRandomAccess<T, Tree>> =
        FingerTreeRandomAccessFoldableMap<T, Tree>{};

/** Traversable typeclass implementation for FingerTreeRandomAccess<T, Tree>.
 *
 * The result of traversal is always FingerTreeRandomAccess<U> (using the
 * default FT5-backed type for U), regardless of the source Tree parameter.
 * This is the production-correct choice; callers who need a specific backing
 * tree for traversal results can write their own adapter.
 */
template <class T, class Tree>
struct FingerTreeRandomAccessTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
                  const smd::tree::FingerTreeRandomAccess<T, Tree> &sequence) {
        using Context = remove_cvref_t<std::invoke_result_t<F, const T &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated = applicative.pure(std::vector<U>{});

        for (const auto &value : sequence.to_vector()) {
            auto lifted = std::invoke(function, value);
            accumulated = applicative.invoke(
                [](std::vector<U> values, U element) {
                    values.push_back(std::move(element));
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<U> values) {
                return smd::tree::FingerTreeRandomAccess<U>::from_sequence(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

template <class T, class Tree>
struct FingerTreeRandomAccessTraversableMap
    : Traversable<FingerTreeRandomAccessTraversableImpl<T, Tree>> {
    using FingerTreeRandomAccessTraversableImpl<T, Tree>::traverse;
};

template <class T, class Tree>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTreeRandomAccess<T, Tree>> =
        FingerTreeRandomAccessTraversableMap<T, Tree>{};

} // namespace smd

#endif
