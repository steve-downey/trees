// src/smd/ranges/range_foldable.hpp                                  -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_FOLDABLE
#define INCLUDED_SMD_RANGES_RANGE_FOLDABLE

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/foldable.hpp>

#include <algorithm>
#include <functional>
#include <ranges>
#include <utility>

namespace smd {

template <class VIEW>
struct ListRangeFoldableImpl {
    template <class FUNCTION>
    auto fold_map(this auto &&self, FUNCTION &&function,
                  const smd::ranges::list_range<VIEW> &values) {
        using Result = remove_cvref_t<std::invoke_result_t<
            FUNCTION,
            const typename smd::ranges::list_range<VIEW>::value_type &>>;

        return self.fold_left(values, smd::monoid_identity<Result>(),
                              [&function](Result acc, const auto &value) {
                                  return smd::monoid_combine(
                                      std::move(acc),
                                      std::invoke(function, value));
                              });
    }

    auto length(this auto &&, const smd::ranges::list_range<VIEW> &values)
        -> std::size_t {
        return static_cast<std::size_t>(std::ranges::distance(values));
    }

    template <class STATE, class FUNCTION>
    auto fold_left(this auto &&, const smd::ranges::list_range<VIEW> &values,
                   STATE initial_state, FUNCTION &&function) {
        return std::ranges::fold_left(values, std::move(initial_state),
                                      std::forward<FUNCTION>(function));
    }

    template <class STATE, class FUNCTION>
    auto fold_right(this auto &&, const smd::ranges::list_range<VIEW> &values,
                    STATE initial_state, FUNCTION &&function) {
        return std::ranges::fold_right(smd::ranges::detail::materialize(values),
                                       std::move(initial_state),
                                       std::forward<FUNCTION>(function));
    }

    template <class PREDICATE>
    auto any_of(this auto &&, const smd::ranges::list_range<VIEW> &values,
                PREDICATE &&predicate) -> bool {
        return std::ranges::any_of(values, std::forward<PREDICATE>(predicate));
    }

    template <class PREDICATE>
    auto all_of(this auto &&, const smd::ranges::list_range<VIEW> &values,
                PREDICATE &&predicate) -> bool {
        return std::ranges::all_of(values, std::forward<PREDICATE>(predicate));
    }

    auto empty(this auto &&, const smd::ranges::list_range<VIEW> &values)
        -> bool {
        return std::ranges::empty(values);
    }

    auto to_vector(this auto &&, const smd::ranges::list_range<VIEW> &values) {
        return smd::ranges::detail::materialize(values);
    }

    template <class PREDICATE>
    auto find_first(this auto &&, const smd::ranges::list_range<VIEW> &values,
                    PREDICATE &&predicate) {
        auto it =
            std::ranges::find_if(values, std::forward<PREDICATE>(predicate));
        if (it == std::ranges::end(values)) {
            return std::optional<
                typename smd::ranges::list_range<VIEW>::value_type>{};
        }
        return std::optional<
            typename smd::ranges::list_range<VIEW>::value_type>{*it};
    }
};

template <class VIEW>
struct ListRangeFoldableMap : Foldable<ListRangeFoldableImpl<VIEW>> {
    using ListRangeFoldableImpl<VIEW>::all_of;
    using ListRangeFoldableImpl<VIEW>::any_of;
    using ListRangeFoldableImpl<VIEW>::empty;
    using ListRangeFoldableImpl<VIEW>::find_first;
    using ListRangeFoldableImpl<VIEW>::fold_map;
    using ListRangeFoldableImpl<VIEW>::fold_left;
    using ListRangeFoldableImpl<VIEW>::fold_right;
    using ListRangeFoldableImpl<VIEW>::length;
    using ListRangeFoldableImpl<VIEW>::to_vector;
};

template <class VIEW>
inline constexpr auto foldable_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeFoldableMap<VIEW>{};

} // namespace smd

#endif
