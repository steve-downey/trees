// src/smd/tree/finger_tree_interval_index.hpp                        -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_TREE_FINGER_TREE_INTERVAL_INDEX
#define INCLUDED_SMD_TREE_FINGER_TREE_INTERVAL_INDEX

#include <smd/tree/finger_tree.hpp>
#include <smd/typeclass/applicative.hpp>
#include <smd/typeclass/foldable.hpp>
#include <smd/typeclass/monoid.hpp>
#include <smd/typeclass/traversable.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace smd::tree {

template <typename PAYLOAD_TYPE>
struct Interval {
    std::size_t d_start;
    std::size_t d_end;
    PAYLOAD_TYPE d_payload;
};

template <typename PAYLOAD_TYPE>
struct IntervalMaxEndTag {
    std::size_t d_max_end;

    friend bool operator==(const IntervalMaxEndTag &,
                           const IntervalMaxEndTag &) = default;
};

template <typename PAYLOAD_TYPE>
struct IntervalMeasure {
    auto operator()(const Interval<PAYLOAD_TYPE> &interval) const
        -> IntervalMaxEndTag<PAYLOAD_TYPE> {
        return IntervalMaxEndTag<PAYLOAD_TYPE>{interval.d_end};
    }
};

template <typename PAYLOAD_TYPE>
class FingerTreeIntervalIndex {
    using Entry = Interval<PAYLOAD_TYPE>;
    using Tree = FingerTree<Entry, IntervalMaxEndTag<PAYLOAD_TYPE>,
                            IntervalMeasure<PAYLOAD_TYPE>>;

    Tree d_tree;

  public:
    FingerTreeIntervalIndex() : d_tree(Tree::empty()) {}

    static auto from_intervals(std::vector<Entry> entries)
        -> FingerTreeIntervalIndex {
        return FingerTreeIntervalIndex{Tree::from_sequence(std::move(entries))};
    }

    auto insert(Entry entry) const -> FingerTreeIntervalIndex {
        return FingerTreeIntervalIndex{d_tree.snoc(std::move(entry))};
    }

    auto query_point(std::size_t point) const -> std::vector<PAYLOAD_TYPE> {
        std::vector<PAYLOAD_TYPE> out;

        // Prune: skip all intervals whose subtree max_end <= point.
        auto parts = d_tree.split_at(
            [point](const IntervalMaxEndTag<PAYLOAD_TYPE> &prefix) {
                return prefix.d_max_end > point;
            });

        // Fold the candidate subtree directly — no intermediate vector.
        parts.d_right.for_each([&](const Entry &e) {
            if (e.d_start <= point && point < e.d_end) {
                out.push_back(e.d_payload);
            }
        });

        return out;
    }

    auto query_overlap(std::size_t start, std::size_t end) const
        -> std::vector<PAYLOAD_TYPE> {
        std::vector<PAYLOAD_TYPE> out;

        // Prune: skip all intervals whose subtree max_end <= start.
        auto parts = d_tree.split_at(
            [start](const IntervalMaxEndTag<PAYLOAD_TYPE> &prefix) {
                return prefix.d_max_end > start;
            });

        // Fold the candidate subtree directly — no intermediate vector.
        parts.d_right.for_each([&](const Entry &e) {
            if (e.d_start < end && start < e.d_end) {
                out.push_back(e.d_payload);
            }
        });

        return out;
    }

    auto entries() const -> std::vector<Entry> { return d_tree.flatten(); }

  private:
    explicit FingerTreeIntervalIndex(Tree tree) : d_tree(std::move(tree)) {}
};

} // namespace smd::tree

namespace smd::typeclass {

template <typename PAYLOAD_TYPE>
struct Monoid<smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE>> {
    auto identity() const -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> {
        return {0U};
    }

    auto combine(const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> &lhs,
                 const smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> &rhs) const
        -> smd::tree::IntervalMaxEndTag<PAYLOAD_TYPE> {
        return {std::max(lhs.d_max_end, rhs.d_max_end)};
    }
};

} // namespace smd::typeclass

namespace smd {

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableImpl {
    template <class F>
    auto fold_map(this auto &&, F &&function,
                  const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE> &index)
        -> remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE &>> {
        using Result =
            remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE &>>;
        return std::ranges::fold_left(
            index.entries(), smd::typeclass::monoid_v<Result>.identity(),
            [&](Result acc, const auto &entry) {
                return smd::typeclass::monoid_v<Result>.combine(
                    std::move(acc), std::invoke(function, entry.d_payload));
            });
    }
};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexFoldableMap
    : Foldable<FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>> {
    using FingerTreeIntervalIndexFoldableImpl<PAYLOAD_TYPE>::fold_map;
};

template <class PAYLOAD_TYPE>
inline constexpr auto
    foldable_typeclass<smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
        FingerTreeIntervalIndexFoldableMap<PAYLOAD_TYPE>{};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableImpl {
    using element_type = PAYLOAD_TYPE;

    template <class APPLICATIVE, class F>
    auto
    traverse(this auto &&, const APPLICATIVE &applicative, F &&function,
             const smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE> &index) {
        using Context =
            remove_cvref_t<std::invoke_result_t<F, const PAYLOAD_TYPE &>>;
        using U = smd::applicative_value_t<Context>;

        auto accumulated =
            applicative.pure(std::vector<smd::tree::Interval<U>>{});

        for (const auto &entry : index.entries()) {
            auto lifted = std::invoke(function, entry.d_payload);
            accumulated = applicative.invoke(
                [start = entry.d_start, end = entry.d_end](
                    std::vector<smd::tree::Interval<U>> values, U payload) {
                    values.push_back(
                        smd::tree::Interval<U>{start, end, std::move(payload)});
                    return values;
                },
                std::move(accumulated), std::move(lifted));
        }

        return applicative.invoke(
            [](std::vector<smd::tree::Interval<U>> values) {
                return smd::tree::FingerTreeIntervalIndex<U>::from_intervals(
                    std::move(values));
            },
            std::move(accumulated));
    }
};

template <class PAYLOAD_TYPE>
struct FingerTreeIntervalIndexTraversableMap
    : Traversable<FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>> {
    using FingerTreeIntervalIndexTraversableImpl<PAYLOAD_TYPE>::traverse;
};

template <class PAYLOAD_TYPE>
inline constexpr auto
    traversable_typeclass<smd::tree::FingerTreeIntervalIndex<PAYLOAD_TYPE>> =
        FingerTreeIntervalIndexTraversableMap<PAYLOAD_TYPE>{};

} // namespace smd

#endif
