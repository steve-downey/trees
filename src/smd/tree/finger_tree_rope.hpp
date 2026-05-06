// src/smd/tree/finger_tree_rope.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_ROPE
#define INCLUDED_SMD_TREE_FINGER_TREE_ROPE

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/traversable.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

/** Measure policy that maps a string chunk to its byte length. */
struct RopeChunkMeasure {
    auto operator()(const std::string &value) const -> std::size_t {
        return value.size();
    }
};

/** @brief Persistent text buffer implemented as a rope over string chunks.
 *
 * The underlying finger tree uses RopeChunkMeasure so the accumulated measure
 * at any node is the total byte count of all chunks to the left.  This enables
 * O(log n) positional split and, consequently, O(log n) insert/erase/replace.
 *
 * Chunk boundaries are not exposed in the public API; callers work with byte
 * positions and string_view values.
 *
 * Complexity:
 * - from_text:    O(n / chunk_size)
 * - size_bytes:   O(1)
 * - to_string:    O(n)
 * - insert/erase/replace: O(log n)
 * - chunks:       O(n)
 */
class FingerTreeRope {
    using Tree = FingerTree<std::string, std::size_t, RopeChunkMeasure>;

    Tree d_tree;

    /** Splits the rope at byte position @p pos; may split a chunk in two. */
    auto split_chars(std::size_t pos) const
        -> std::pair<FingerTreeRope, FingerTreeRope> {
        if (pos == 0) {
            return {FingerTreeRope{}, *this};
        }

        if (pos >= size_bytes()) {
            return {*this, FingerTreeRope{}};
        }

        auto split =
            d_tree.split([pos](std::size_t prefix) { return prefix > pos; });

        if (!split.has_value()) {
            return {*this, FingerTreeRope{}};
        }

        auto left_prefix_bytes = split->d_left.measure();
        auto local = pos - left_prefix_bytes;

        const auto &pivot = split->d_pivot;
        assert(local < pivot.size());

        auto left = split->d_left;
        if (local > 0) {
            left = left.snoc(pivot.substr(0, local));
        }

        auto right = split->d_right;
        if (local < pivot.size()) {
            right = right.cons(pivot.substr(local));
        }

        return {FingerTreeRope{std::move(left)},
                FingerTreeRope{std::move(right)}};
    }

  public:
    /** Constructs an empty rope. */
    FingerTreeRope() : d_tree(Tree::empty()) {}

    /** Builds a rope from a vector of pre-formed string chunks; O(n chunks). */
    static auto from_chunks(std::vector<std::string> chunks) -> FingerTreeRope {
        return FingerTreeRope{Tree::from_sequence(std::move(chunks))};
    }

    /** @brief Builds a rope from a text string, splitting into chunks of at
     *         most @p chunk_size bytes.
     */
    static auto from_text(std::string_view text, std::size_t chunk_size = 16)
        -> FingerTreeRope {
        std::vector<std::string> chunks;
        chunks.reserve((text.size() / chunk_size) + 1);

        for (std::size_t i = 0; i < text.size(); i += chunk_size) {
            const auto n = std::min(chunk_size, text.size() - i);
            chunks.emplace_back(text.substr(i, n));
        }

        return from_chunks(std::move(chunks));
    }

    /** Returns the total length of the text in bytes; O(1). */
    auto size_bytes() const -> std::size_t { return d_tree.measure(); }

    /** Concatenates all chunks into a single string; O(n). */
    auto to_string() const -> std::string {
        std::string out;
        out.reserve(size_bytes());
        std::ranges::for_each(
            d_tree.flatten(),
            [&out](const std::string &chunk) { out += chunk; });
        return out;
    }

    /** Returns a new rope with @p text inserted at byte position @p pos; O(log n). */
    auto insert(std::size_t pos, std::string_view text) const
        -> FingerTreeRope {
        auto [left, right] = split_chars(pos);
        auto middle = from_text(text);
        return FingerTreeRope{Tree::concat(
            Tree::concat(left.d_tree, middle.d_tree), right.d_tree)};
    }

    /** Returns a new rope with @p count bytes removed starting at @p pos; O(log n). */
    auto erase(std::size_t pos, std::size_t count) const -> FingerTreeRope {
        auto [left, rest] = split_chars(pos);
        auto [drop, right] = rest.split_chars(count);
        static_cast<void>(drop);
        return FingerTreeRope{Tree::concat(left.d_tree, right.d_tree)};
    }

    /** Returns a new rope with @p count bytes at @p pos replaced by @p text; O(log n). */
    auto replace(std::size_t pos, std::size_t count,
                 std::string_view text) const -> FingerTreeRope {
        return erase(pos, count).insert(pos, text);
    }

    /** Returns a snapshot of the internal chunk vector in sequence order; O(n). */
    auto chunks() const -> std::vector<std::string> { return d_tree.flatten(); }

  private:
    explicit FingerTreeRope(Tree tree) : d_tree(std::move(tree)) {}
};

} // namespace smd::tree

namespace smd {

/** Foldable typeclass implementation for FingerTreeRope; folds over string chunks. */
struct FingerTreeRopeFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeRope &rope)
        -> remove_cvref_t<std::invoke_result_t<F, const std::string &>> {
        using Result =
            remove_cvref_t<std::invoke_result_t<F, const std::string &>>;
        return std::ranges::fold_left(
            rope.chunks(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &chunk) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, chunk));
            });
    }
};

/** Foldable typeclass map entry for FingerTreeRope. */
struct FingerTreeRopeFoldableMap : Foldable<FingerTreeRopeFoldableImpl> {
    using FingerTreeRopeFoldableImpl::fold_map;
};

/** Registers FingerTreeRope as a Foldable. */
template <>
inline constexpr auto foldable_typeclass<smd::tree::FingerTreeRope> =
    FingerTreeRopeFoldableMap{};

/** Traversable typeclass implementation for FingerTreeRope; maps over chunks. */
struct FingerTreeRopeTraversableImpl {
    using element_type = std::string;

    template <class APPLICATIVE, class F>
    auto traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
                  const smd::tree::FingerTreeRope &rope) {
        using Context =
            remove_cvref_t<std::invoke_result_t<F, const std::string &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated = applicative.pure(std::vector<U>{});

        for (const auto &chunk : rope.chunks()) {
            auto lifted = std::invoke(function, chunk);
            accumulated = applicative.invoke(
                [](std::vector<U> values, U element) {
                    values.push_back(std::move(element));
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<U> values) {
                return smd::tree::FingerTreeRope::from_chunks(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

/** Traversable typeclass map entry for FingerTreeRope. */
struct FingerTreeRopeTraversableMap
    : Traversable<FingerTreeRopeTraversableImpl> {
    using FingerTreeRopeTraversableImpl::traverse;
};

/** Registers FingerTreeRope as a Traversable. */
template <>
inline constexpr auto traversable_typeclass<smd::tree::FingerTreeRope> =
    FingerTreeRopeTraversableMap{};

} // namespace smd

#endif
