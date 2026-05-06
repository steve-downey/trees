// src/smd/ranges/range_functor.hpp                                   -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_SMD_RANGES_RANGE_FUNCTOR
#define INCLUDED_SMD_RANGES_RANGE_FUNCTOR

#include <smd/ranges/range_list.hpp>
#include <smd/typeclass/functor.hpp>

#include <ranges>
#include <utility>

namespace smd {

/** Functor typeclass instance for list_range<VIEW>.
 * fmap applies @p function lazily to each element via std::views::transform
 * and wraps the result in a new list_range.
 * @tparam VIEW underlying view type of the list_range being mapped
 */
template <class VIEW>
struct ListRangeFunctorImpl {
    template <class FUNCTION>
    auto fmap(this auto &&, FUNCTION &&function,
              const smd::ranges::list_range<VIEW> &values) {
        return smd::ranges::all(
            values | std::views::transform(std::forward<FUNCTION>(function)));
    }
};

/** Functor map that exposes fmap for list_range<VIEW>. */
template <class VIEW>
struct ListRangeFunctorMap : Functor<ListRangeFunctorImpl<VIEW>> {
    using ListRangeFunctorImpl<VIEW>::fmap;
};

/** Registers ListRangeFunctorMap as the Functor instance for list_range<VIEW>. */
template <class VIEW>
inline constexpr auto functor_typeclass<smd::ranges::list_range<VIEW>> =
    ListRangeFunctorMap<VIEW>{};

} // namespace smd

#endif
