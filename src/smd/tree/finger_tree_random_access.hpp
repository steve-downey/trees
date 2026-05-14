// src/smd/tree/finger_tree_random_access.hpp                         -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS
#define INCLUDED_SMD_TREE_FINGER_TREE_RANDOM_ACCESS

#include <smd/tree/finger_tree2.hpp>
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
 * @tparam T Element type.
 *
 * Uses UnitMeasure (element count) as the finger tree measure, enabling O(log
 * n) index navigation without materialisation.
 *
 * Complexity:
 * - at / update / insert / erase: O(log n)
 * - push_front / push_back:       O(1) amortized
 * - size / empty:                 O(1)
 * - to_vector:                    O(n)
 */
template <typename T>
class FingerTreeRandomAccess {
    FingerTree2<T> d_tree;

  public:
    /** Constructs an empty sequence. */
    FingerTreeRandomAccess() : d_tree(FingerTree2<T>::empty()) {}

    /** Constructs from an existing finger tree. */
    explicit FingerTreeRandomAccess(FingerTree2<T> tree)
        : d_tree(std::move(tree)) {}

    /** Builds a sequence from a vector in order; O(n). */
    static auto from_sequence(std::vector<T> values) -> FingerTreeRandomAccess {
        return FingerTreeRandomAccess(
            FingerTree2<T>::from_sequence(std::move(values)));
    }

    /** Returns the number of elements. */
    auto size() const -> std::size_t { return d_tree.breadth(); }

    /** Returns true if the sequence contains no elements. */
    auto empty() const -> bool { return d_tree.is_empty(); }

    /** Returns the element at @p index, or nullopt if out of range; O(log n). */
    auto at(std::size_t index) const -> std::optional<T> {
        if (index >= size()) {
            return std::nullopt;
        }
        // split() with count predicate: pivot is the element at position index.
        // O(log n).
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value()) {
            return std::nullopt;
        }
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

    /** Returns a new sequence with @p value inserted before position @p index; O(log n). */
    auto insert(std::size_t index, T value) const -> FingerTreeRandomAccess {
        // split_at with count predicate puts [0,index) left, [index,n) right.
        // O(log n).
        auto parts = d_tree.split_at(
            [index](std::size_t prefix) { return prefix > index; });
        return FingerTreeRandomAccess(FingerTree2<T>::concat(
            FingerTree2<T>::concat(parts.d_left,
                                  FingerTree2<T>::leaf(std::move(value))),
            parts.d_right));
    }

    /** Returns a new sequence with the element at @p index removed; O(log n).
     * Returns @c *this unchanged if @p index is out of range.
     */
    auto erase(std::size_t index) const -> FingerTreeRandomAccess {
        if (index >= size()) {
            return *this;
        }
        // split() finds the element at index as pivot; drop it by concat(left,
        // right). O(log n).
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value()) {
            return *this;
        }
        return FingerTreeRandomAccess(
            FingerTree2<T>::concat(sp->d_left, sp->d_right));
    }

    /** Returns a new sequence with position @p index replaced by @p value; O(log n).
     * Returns @c *this unchanged if @p index is out of range.
     */
    auto update(std::size_t index, T value) const -> FingerTreeRandomAccess {
        if (index >= size()) {
            return *this;
        }
        // Single split+replace: find element at index as pivot, swap in the new
        // value. O(log n).
        auto sp = d_tree.split(
            [index](std::size_t prefix) { return prefix > index; });
        if (!sp.has_value()) {
            return *this;
        }
        return FingerTreeRandomAccess(FingerTree2<T>::concat(
            sp->d_left.snoc(std::move(value)), sp->d_right));
    }

    /** Materialises all elements into a vector in sequence order; O(n). */
    auto to_vector() const -> std::vector<T> { return d_tree.flatten(); }
};

} // namespace smd::tree

namespace smd {

/** Foldable typeclass implementation for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeRandomAccess<T> &sequence)
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

/** Foldable typeclass map entry for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessFoldableMap
    : Foldable<FingerTreeRandomAccessFoldableImpl<T>> {
    using FingerTreeRandomAccessFoldableImpl<T>::fold_map;
};

/** Registers FingerTreeRandomAccess as a Foldable. */
template <class T>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
    FingerTreeRandomAccessFoldableMap<T>{};

/** Traversable typeclass implementation for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessTraversableImpl {
    using element_type = T;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
                  const smd::tree::FingerTreeRandomAccess<T> &sequence) {
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

/** Traversable typeclass map entry for FingerTreeRandomAccess. */
template <class T>
struct FingerTreeRandomAccessTraversableMap
    : Traversable<FingerTreeRandomAccessTraversableImpl<T>> {
    using FingerTreeRandomAccessTraversableImpl<T>::traverse;
};

/** Registers FingerTreeRandomAccess as a Traversable. */
template <class T>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTreeRandomAccess<T>> =
        FingerTreeRandomAccessTraversableMap<T>{};

} // namespace smd

#endif
